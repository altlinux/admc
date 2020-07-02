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

#include "config.h"

#include "main_window.h"
#include "containers_widget.h"
#include "contents_widget.h"
#include "members_widget.h"
#include "details_widget.h"
#include "attributes_widget.h"
#include "status.h"
#include "settings.h"
#include "entry_context_menu.h"
#include "confirmation_dialog.h"
#include "login_dialog.h"

#include <QApplication>
#include <QString>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QSplitter>
#include <QStatusBar>
#include <QTreeView>
#include <QVBoxLayout>
#include <QTextEdit>

void on_action_login() {
}

void on_action_exit(QMainWindow *main_window) {
    const QString text = QString("Are you sure you want to exit?");
    const bool confirmed = confirmation_dialog(text, main_window);

    if (confirmed) {
        QApplication::quit();
    }   
}

MainWindow::MainWindow()
: QMainWindow()
{
    // TODO: setting width to 1600+ fullscreens the window, no idea why
    resize(1200, 700);
    setWindowTitle("MainWindow");

    // Menubar
    QAction *login_action = nullptr;
    {
        QMenuBar *menubar = menuBar();
        QMenu *menubar_file = menubar->addMenu("File");
        login_action = menubar_file->addAction("Login");
        menubar_file->addAction("Exit", [this]() {
            on_action_exit(this);
        });

        QMenu *menubar_view = menubar->addMenu("View");
        menubar_view->addAction(SETTINGS()->toggle_advanced_view);
        menubar_view->addAction(SETTINGS()->toggle_show_dn_column);
        menubar_view->addAction(SETTINGS()->toggle_show_status_log);

        QMenu *menubar_preferences = menubar->addMenu("Preferences");
        menubar_preferences->addAction(SETTINGS()->details_on_containers_click);
        menubar_preferences->addAction(SETTINGS()->details_on_contents_click);
        menubar_preferences->addAction(SETTINGS()->confirm_actions);
        menubar_preferences->addAction(SETTINGS()->auto_login);
    }

    // Widgets
    auto central_widget = new QWidget(this);
    setCentralWidget(central_widget);

    auto entry_context_menu = new EntryContextMenu(this);
    
    auto containers_widget = new ContainersWidget(entry_context_menu, this);
    auto contents_widget = new ContentsWidget(containers_widget, entry_context_menu, this);
    auto details_widget = new DetailsWidget(entry_context_menu, containers_widget, contents_widget, this);

    auto status_log = new QTextEdit(this);
    status_log->setReadOnly(true);

    new LoginDialog(login_action, this);

    new Status(statusBar(), status_log, this);

    // NOTE: do this after all widgets are constructed so that all of
    // them load initial settings correctly
    SETTINGS()->emit_toggle_signals();

    // Layout
    {
        auto horiz_splitter = new QSplitter(Qt::Horizontal);
        horiz_splitter->addWidget(containers_widget);
        horiz_splitter->addWidget(contents_widget);
        horiz_splitter->addWidget(details_widget);
        horiz_splitter->setStretchFactor(0, 1);
        horiz_splitter->setStretchFactor(1, 2);
        horiz_splitter->setStretchFactor(2, 2);

        auto vert_splitter = new QSplitter(Qt::Vertical);
        vert_splitter->addWidget(status_log);
        vert_splitter->addWidget(horiz_splitter);
        vert_splitter->setStretchFactor(0, 1);
        vert_splitter->setStretchFactor(1, 3);

        auto central_layout = new QVBoxLayout();
        central_layout->addWidget(vert_splitter);
        central_layout->setContentsMargins(0, 0, 0, 0);
        central_layout->setSpacing(0);

        central_widget->setLayout(central_layout);
    }

    // Enable central widget on login
    central_widget->setEnabled(false);
    QObject::connect(
        AD(), &AdInterface::ad_interface_login_complete,
        [central_widget] () {
            central_widget->setEnabled(true);
        });

    const bool auto_login = SETTINGS()->auto_login->isChecked();
    if (auto_login) {
        const QString host = SETTINGS()->get_string(SettingString_Host);

        if (!host.isEmpty()) {
            const QString uri = "ldap://" + host;

            AD()->ad_interface_login(uri, "DC=domain,DC=alt");
        }
    }    
}
