/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "main_window.h"
#include "menubar.h"
#include "containers_widget.h"
#include "contents_widget.h"
#include "details_dialog.h"
#include "status.h"
#include "settings.h"
#include "confirmation_dialog.h"
#include "policies_widget.h"
#include "ad_interface.h"
#include "ad_config.h"
#include "object_model.h"
#include "filter_dialog.h"
#include "object_menu.h"
#include "panes.h"
#include "utils.h"

#include <QApplication>
#include <QString>
#include <QSplitter>
#include <QStatusBar>
#include <QTextEdit>
#include <QAction>
#include <QTreeWidget>

MainWindow::MainWindow()
: QMainWindow()
{
    SETTINGS()->restore_geometry(this, VariantSetting_MainWindowGeometry);

    menubar = new MenuBar();
    setMenuBar(menubar);

    QStatusBar *status_bar = STATUS()->status_bar;
    setStatusBar(status_bar);

    // Setup fake offline versions of widgets for display purposes
    auto offline_containers = new QTreeWidget();
    offline_containers->setHeaderLabels({
        tr("Name"),
    });

    auto offline_contents = new QTreeWidget();
    offline_contents->setHeaderLabels({
        tr("Name"),
        tr("Class"),
        tr("Description"),
    });

    auto splitter = new QSplitter();
    splitter->addWidget(offline_containers);
    splitter->addWidget(offline_contents);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);

    setCentralWidget(splitter);

    connect(
        AD(), &AdInterface::connected,
        this, &MainWindow::on_connected);

    // NOTE: order of operations here is very finnicky. Have
    // to connect() BEFORE show(), otherwise for the
    // duration of the connection process, main window will
    // be black. Have to end_error_log() (show error popup)
    // AFTER show(), otherwise error popup modality won't
    // work.

    STATUS()->start_error_log();

    AD()->connect();

    show();

    STATUS()->end_error_log(this);
}

void MainWindow::on_connected() {
    QTextEdit *status_log = STATUS()->status_log;
    status_log->clear();
    STATUS()->status_bar->showMessage(tr("Ready"));

    auto panes = new Panes();

    auto details_widget_docked_container = DetailsDialog::get_docked_container();
    
    // TODO: really should show a blank widget when docked details is toggled. Right now, it does nothing until some object's details is opened which is confusing.

    auto panes_details_splitter = new QSplitter(Qt::Horizontal);
    panes_details_splitter->addWidget(panes);
    panes_details_splitter->addWidget(details_widget_docked_container);
    panes_details_splitter->setStretchFactor(0, 2);
    panes_details_splitter->setStretchFactor(1, 1);

    auto vert_splitter = new QSplitter(Qt::Vertical);
    vert_splitter->addWidget(status_log);
    vert_splitter->addWidget(panes_details_splitter);
    vert_splitter->setStretchFactor(0, 1);
    vert_splitter->setStretchFactor(1, 3);

    setCentralWidget(vert_splitter);

    auto connect_toggle_widget =
    [](QWidget *widget, const BoolSetting setting) {
        const BoolSettingSignal *signal = SETTINGS()->get_bool_signal(setting);

        auto on_changed =
        [=]() {
            const bool visible = SETTINGS()->get_bool(setting);
            widget->setVisible(visible);
        };

        connect(
            signal, &BoolSettingSignal::changed,
            on_changed);
        on_changed();
    };

    // connect_toggle_widget(containers_widget, BoolSetting_ShowContainers);
    connect_toggle_widget(status_log, BoolSetting_ShowStatusLog);
    connect(
        menubar->filter_action, &QAction::triggered,
        panes->filter_dialog, &QDialog::open);

    connect(
        menubar->up_one_level_action, &QAction::triggered,
        this, &MainWindow::navigate_up);
    connect(
        menubar->back_action, &QAction::triggered,
        this, &MainWindow::navigate_back);
    connect(
        menubar->forward_action, &QAction::triggered,
        this, &MainWindow::navigate_forward);

    update_navigation_actions();

    connect(
        menubar->action_menu, &QMenu::aboutToShow,
        [=]() {
            menubar->action_menu->clear();
            panes->load_menu(menubar->action_menu);
        });
}

void MainWindow::closeEvent(QCloseEvent *event) {
    SETTINGS()->save_geometry(this, VariantSetting_MainWindowGeometry);

    QApplication::quit();
}

// TODO: need to disable navigation options depending on if they are possible to do or not! then won't need all this validity checking
void MainWindow::on_containers_current_changed(const QModelIndex &source_index) {
    // NOTE: this can occur when navigation changes selection, which will trigger a signal which will call this slot. If current target is unchanged, we don't want to erase future target history.
    if (source_index == targets_current) {
        return;
    }

    // Erase future target history because current ovewrites it
    targets_future.clear();
    if (targets_current.isValid()) {
        targets_past.append(targets_current);
    }

    targets_current = source_index;
    update_navigation_actions();

    contents_widget->set_target(targets_current);
}

void MainWindow::navigate_up() {
    targets_future.clear();
    targets_past.append(targets_current);

    targets_current = targets_current.parent();
    update_navigation_actions();

    contents_widget->set_target(targets_current);
    containers_widget->change_current(targets_current);
}

void MainWindow::navigate_back() {
    targets_future.prepend(targets_current);

    targets_current = targets_past.takeLast();
    update_navigation_actions();
    
    contents_widget->set_target(targets_current);
    containers_widget->change_current(targets_current);
}

void MainWindow::navigate_forward() {
    targets_past.append(targets_current);

    targets_current = targets_future.takeFirst();
    update_navigation_actions();

    contents_widget->set_target(targets_current);
    containers_widget->change_current(targets_current);
}

// NOTE: as long as this is called where appropriate (on every target change), it is not necessary to do any condition checks in navigation f-ns since the actions that call them will be disabled if they can't be done
void MainWindow::update_navigation_actions() {
    menubar->up_one_level_action->setEnabled(targets_current.isValid());
    menubar->back_action->setEnabled(!targets_past.isEmpty());
    menubar->forward_action->setEnabled(!targets_future.isEmpty());
}
