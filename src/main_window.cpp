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
#include "members_model.h"
#include "members_widget.h"
#include "details_widget.h"
#include "ad_model.h"
#include "attributes_model.h"
#include "status.h"
#include "entry_widget.h"
#include "settings.h"
#include "entry_context_menu.h"
#include "confirmation_dialog.h"

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

MainWindow::MainWindow(const bool auto_login)
: QMainWindow()
{
    // TODO: setting width to 1600+ fullscreens the window, no idea why
    resize(1500, 1000);
    setWindowTitle("MainWindow");

    // Menubar
    {
        QMenuBar *menubar = menuBar();
        QMenu *menubar_file = menubar->addMenu("File");
        menubar_file->addAction("Login", [this]() {
            on_action_login();
        });
        menubar_file->addAction("Exit", [this]() {
            on_action_exit();
        });

        QMenu *menubar_view = menubar->addMenu("View");
        menubar_view->addAction(SETTINGS()->toggle_advanced_view);
        menubar_view->addAction(SETTINGS()->toggle_show_dn_column);
        menubar_view->addAction(SETTINGS()->toggle_show_status_log);

        QMenu *menubar_preferences = menubar->addMenu("Preferences");
        menubar_preferences->addAction(SETTINGS()->details_on_containers_click);
        menubar_preferences->addAction(SETTINGS()->details_on_contents_click);
        menubar_preferences->addAction(SETTINGS()->confirm_actions);
    }

    // Widgets
    auto central_widget = new QWidget(this);
    setCentralWidget(central_widget);

    auto entry_context_menu = new EntryContextMenu(this);
   
    auto ad_model = new AdModel(this);
    auto containers_widget = new ContainersWidget(ad_model, entry_context_menu, this);
    auto contents_widget = new ContentsWidget(ad_model, entry_context_menu, this);

    auto members_model = new MembersModel(this);
    auto members_widget = new MembersWidget(members_model, entry_context_menu, this);
    
    auto details_widget = new DetailsWidget(members_widget, this);

    auto status_log = new QTextEdit(this);
    status_log->setReadOnly(true);

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

    // Connect signals
    {
        connect(
            containers_widget, &ContainersWidget::selected_container_changed,
            contents_widget, &ContentsWidget::on_selected_container_changed);
        connect(
            containers_widget, &EntryWidget::clicked_dn,
            details_widget, &DetailsWidget::on_containers_clicked_dn);
        connect(
            contents_widget, &EntryWidget::clicked_dn,
            details_widget, &DetailsWidget::on_contents_clicked_dn);
        connect(
            entry_context_menu, &EntryContextMenu::details,
            details_widget, &DetailsWidget::on_context_menu_details);
    }

    if (auto_login) {
        on_action_login();
    }    
}

void MainWindow::on_action_login() {
    AD()->ad_interface_login(SEARCH_BASE, HEAD_DN);
}

void MainWindow::on_action_exit() {
    const QString text = QString("Are you sure you want to exit?");
    const bool confirmed = confirmation_dialog(text, this);

    if (confirmed) {
        QApplication::quit();
    }   
}
