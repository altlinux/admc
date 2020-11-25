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

#include <QApplication>
#include <QString>
#include <QSplitter>
#include <QStatusBar>
#include <QTextEdit>
#include <QMessageBox>
#include <QTimer>
#include <QAction>
#include <QTreeWidget>

MainWindow::MainWindow()
: QMainWindow()
{
    SETTINGS()->restore_geometry(this, VariantSetting_MainWindowGeometry);

    menubar = new MenuBar();
    setMenuBar(menubar);

    connect(
        menubar->connect_action, &QAction::triggered,
        [this]() {
            attempt_to_connect();
        });

    connect(
        menubar->quit_action, &QAction::triggered,
        []() {
            QApplication::quit();
        });

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

    attempt_to_connect();
}

void MainWindow::attempt_to_connect() {
    const ConnectResult result = AD()->connect();

    if (result == ConnectResult_Success) {
        finish_init();
    } else {
        // Open dialog explaining connection error
        // TODO: not sure about phrasing of errors
        const QMessageBox::Icon icon = QMessageBox::Warning;
        const QString title = tr("Failed to connect");
        const QString text =
        [result]() {
            switch (result) {
                case ConnectResult_Success: return QString();
                case ConnectResult_FailedToFindHosts: return tr("Not connected to a domain network");
                case ConnectResult_FailedToConnect: return tr("Authentication failed");
            }
            return QString();
        }();
        const QMessageBox::StandardButtons buttons = QMessageBox::Ok;
        auto dialog = new QMessageBox(icon, title, text, buttons, this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);

        dialog->open();
    }
}

// Complete initialization
void MainWindow::finish_init() {
    statusBar()->showMessage(tr("Ready"));
    
    menubar->enter_online_mode();

    QTextEdit *status_log = STATUS()->status_log;

    auto containers_widget = new ContainersWidget(this);

    auto contents_widget = new ContentsWidget(containers_widget, menubar->filter_contents_action);
    
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
        connect(
            signal, &BoolSettingSignal::changed,
            [=]() {
                const bool visible = SETTINGS()->get_bool(setting);
                widget->setVisible(visible); 
            });
    };

    connect_toggle_widget(containers_widget, BoolSetting_ShowContainers);
    connect_toggle_widget(status_log, BoolSetting_ShowStatusLog);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    SETTINGS()->save_geometry(this, VariantSetting_MainWindowGeometry);

    QApplication::quit();
}
