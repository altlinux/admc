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
#include "console.h"
#include "utils.h"

#include <QApplication>
#include <QString>
#include <QSplitter>
#include <QStatusBar>
#include <QTextEdit>
#include <QAction>
#include <QDesktopWidget>

MainWindow::MainWindow()
: QMainWindow()
{
    if (SETTINGS()->contains_variant(VariantSetting_MainWindowGeometry)) {
        SETTINGS()->restore_geometry(this, VariantSetting_MainWindowGeometry);
    } else {
        // Make window 70% of desktop size for first startup
        resize(QDesktopWidget().availableGeometry(this).size() * 0.7);
    }

    // Setup menubar, status bar and a dummy central widget
    // for display in offline state
    menubar = new MenuBar();
    setMenuBar(menubar);

    QStatusBar *status_bar = STATUS()->status_bar;
    setStatusBar(status_bar);

    auto dummy_widget = new QWidget();
    setCentralWidget(dummy_widget);

    connect(
        AD(), &AdInterface::connected,
        this, &MainWindow::on_connected);

    STATUS()->start_error_log();

    AD()->connect();

    STATUS()->end_error_log(this);
}

// NOTE: need to finish creating widgets after connection
// because some widgets require text strings that need to be
// obtained from the server
void MainWindow::on_connected() {
    QTextEdit *status_log = STATUS()->status_log;
    status_log->clear();
    STATUS()->status_bar->showMessage(tr("Ready"));

    auto console = new Console(menubar);

    auto details_widget_docked_container = DetailsDialog::get_docked_container();
    
    // TODO: really should show a blank widget when docked details is toggled. Right now, it does nothing until some object's details is opened which is confusing.

    auto console_details_splitter = new QSplitter(Qt::Horizontal);
    console_details_splitter->addWidget(console);
    console_details_splitter->addWidget(details_widget_docked_container);
    console_details_splitter->setStretchFactor(0, 2);
    console_details_splitter->setStretchFactor(1, 1);

    auto vert_splitter = new QSplitter(Qt::Vertical);
    vert_splitter->addWidget(status_log);
    vert_splitter->addWidget(console_details_splitter);
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

    connect_toggle_widget(status_log, BoolSetting_ShowStatusLog);
    connect(
        menubar->filter_action, &QAction::triggered,
        console->filter_dialog, &QDialog::open);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    SETTINGS()->save_geometry(this, VariantSetting_MainWindowGeometry);

    QApplication::quit();
}
