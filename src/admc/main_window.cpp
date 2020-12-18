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
#include "object_model.h"
#include "filter_dialog.h"
#include "find_dialog.h"

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

    auto filter_dialog = new FilterDialog(this);

    auto find_dialog = new FindDialog(this);

    auto object_model = new ObjectModel(this);

    auto containers_widget = new ContainersWidget(object_model, this);

    auto contents_widget = new ContentsWidget(object_model);

    auto details_widget_docked_container = DetailsDialog::get_docked_container();
    auto policies_widget = new PoliciesWidget();

    // Layout
    const auto containers_policies_splitter = new QSplitter(Qt::Vertical);
    containers_policies_splitter->addWidget(containers_widget);
    containers_policies_splitter->addWidget(policies_widget);
    containers_policies_splitter->setStretchFactor(0, 2);
    containers_policies_splitter->setStretchFactor(1, 1);

    auto horiz_splitter = new QSplitter(Qt::Horizontal);
    horiz_splitter->addWidget(containers_policies_splitter);
    horiz_splitter->addWidget(contents_widget);
    horiz_splitter->addWidget(details_widget_docked_container);
    horiz_splitter->setStretchFactor(0, 1);
    horiz_splitter->setStretchFactor(1, 2);
    horiz_splitter->setStretchFactor(2, 2);

    auto vert_splitter = new QSplitter(Qt::Vertical);
    vert_splitter->addWidget(status_log);
    vert_splitter->addWidget(horiz_splitter);
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

    connect_toggle_widget(containers_widget, BoolSetting_ShowContainers);
    connect_toggle_widget(status_log, BoolSetting_ShowStatusLog);

    connect(
        filter_dialog, &FilterDialog::filter_changed,
        object_model, &ObjectModel::on_filter_changed);

    connect(
        menubar->filter_action, &QAction::triggered,
        filter_dialog, &QDialog::open);
    connect(
        menubar->find_action, &QAction::triggered,
        find_dialog, &QDialog::open);

    connect(
        containers_widget, &ContainersWidget::selected_changed,
        contents_widget, &ContentsWidget::on_containers_selected_changed);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    SETTINGS()->save_geometry(this, VariantSetting_MainWindowGeometry);

    QApplication::quit();
}
