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
#include "containers_widget.h"
#include "contents_widget.h"
#include "details_widget.h"
#include "status.h"
#include "settings.h"
#include "object_context_menu.h"
#include "confirmation_dialog.h"
#include "login_dialog.h"

#include <QApplication>
#include <QString>
#include <QMenu>
#include <QMenuBar>
#include <QSplitter>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QSettings>

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
    setWindowTitle("MainWindow");

    // Restore last geometry
    const QByteArray geometry = Settings::instance()->get_value(SettingsValue_MainWindowGeometry).toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }

    // Menubar
    QAction *login_action = nullptr;
    {
        QAction *advanced_view = Settings::instance()->checkable(SettingsCheckable_AdvancedView);
        QAction *show_dn_column = Settings::instance()->checkable(SettingsCheckable_DnColumn);
        QAction *show_status_log = Settings::instance()->checkable(SettingsCheckable_ShowStatusLog);
        QAction *details_from_containers = Settings::instance()->checkable(SettingsCheckable_DetailsFromContainers);
        QAction *details_from_contents = Settings::instance()->checkable(SettingsCheckable_DetailsFromContents);
        QAction *confirm_actions = Settings::instance()->checkable(SettingsCheckable_ConfirmActions);
        QAction *auto_login = Settings::instance()->checkable(SettingsCheckable_AutoLogin);

        QMenuBar *menubar = menuBar();
        QMenu *menubar_file = menubar->addMenu("File");
        login_action = menubar_file->addAction("Login");
        menubar_file->addAction("Exit", [this]() {
            on_action_exit(this);
        });

        QMenu *menubar_view = menubar->addMenu("View");
        menubar_view->addAction(advanced_view);
        menubar_view->addAction(show_dn_column);
        menubar_view->addAction(show_status_log);

        QMenu *menubar_preferences = menubar->addMenu("Preferences");
        menubar_preferences->addAction(details_from_containers);
        menubar_preferences->addAction(details_from_contents);
        menubar_preferences->addAction(confirm_actions);
        menubar_preferences->addAction(auto_login);
    }

    // Widgets
    auto central_widget = new QWidget(this);
    setCentralWidget(central_widget);

    auto object_context_menu = new ObjectContextMenu(this);
    
    auto containers_widget = new ContainersWidget(object_context_menu, this);
    auto contents_widget = new ContentsWidget(containers_widget, object_context_menu, this);
    auto details_widget = new DetailsWidget(object_context_menu, containers_widget, contents_widget, this);

    auto status_log = new QTextEdit(this);
    status_log->setReadOnly(true);

    new LoginDialog(login_action, this);

    QStatusBar *status_bar = statusBar();
    new Status(status_bar, status_log, this);

    // NOTE: must do this after all widgets are constructed
    Settings::instance()->emit_toggle_signals();

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
        AdInterface::instance(), &AdInterface::logged_in,
        [central_widget] () {
            central_widget->setEnabled(true);
        });

    QAction *auto_login = Settings::instance()->checkable(SettingsCheckable_AutoLogin);
    if (auto_login->isChecked()) {
        const QString host = Settings::instance()->get_value(SettingsValue_Host).toString();
        const QString domain = Settings::instance()->get_value(SettingsValue_Domain).toString();

        if (!host.isEmpty()) {
            AdInterface::instance()->login(host, domain);
        }
    }    
}

void MainWindow::closeEvent(QCloseEvent *event) {
    const QByteArray geometry = saveGeometry();
    Settings::instance()->set_value(SettingsValue_MainWindowGeometry, QVariant(geometry));
}
