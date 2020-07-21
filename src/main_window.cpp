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
#include <QActionGroup>

MainWindow::MainWindow()
: QMainWindow()
{
    // Restore last geometry
    const QByteArray geometry = Settings::instance()->get_variant(VariantSetting_MainWindowGeometry).toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }

    // Menubar
    QAction *login_action = nullptr;
    {
        QMenuBar *menubar = menuBar();
        QMenu *menubar_file = menubar->addMenu(tr("File"));
        login_action = menubar_file->addAction(tr("Login"));
        QAction *exit_action = menubar_file->addAction(tr("Exit"));
        connect(exit_action, &QAction::triggered,
            this, &MainWindow::on_action_exit);

        auto add_bool_setting_action = 
        [](QMenu *menu, QString display_text, BoolSetting type) {
            QAction *action = menu->addAction(display_text);
            Settings::instance()->connect_action_to_get_bool_signal(action, type);
        };

        QMenu *menubar_view = menubar->addMenu(tr("View"));
        add_bool_setting_action(menubar_view, tr("Advanced view"), BoolSetting_AdvancedView);
        add_bool_setting_action(menubar_view, tr("Show DN column"), BoolSetting_DnColumn);
        add_bool_setting_action(menubar_view, tr("Show status log"), BoolSetting_ShowStatusLog);

        QMenu *menubar_preferences = menubar->addMenu(tr("Preferences"));
        add_bool_setting_action(menubar_preferences, tr("Open attributes on left click in Containers window"), BoolSetting_DetailsFromContainers);
        add_bool_setting_action(menubar_preferences, tr("Open attributes on left click in Contents window"), BoolSetting_DetailsFromContents);
        add_bool_setting_action(menubar_preferences, tr("Confirm actions"), BoolSetting_ConfirmActions);
        add_bool_setting_action(menubar_preferences, tr("Login using saved session at startup"), BoolSetting_AutoLogin);

        QMenu *language_menu = menubar_preferences->addMenu(tr("Language"));
        auto language_group = new QActionGroup(language_menu);

        auto add_language_action =
        [language_menu, language_group] (QLocale::Language language) {
            QLocale locale(language);
            const QString language_name = locale.nativeLanguageName();

            const auto action = new QAction(language_name, language_group);
            action->setCheckable(true);
            language_group->addAction(action);
            language_menu->addAction(action);

            const QLocale saved_locale = Settings::instance()->get_variant(VariantSetting_Locale).toLocale();
            const QLocale::Language saved_language = saved_locale.language();
            if (language == saved_language) {
                action->setChecked(true);
            }

            connect(
                action, &QAction::toggled,
                [locale](bool checked) {
                    if (checked) {
                        Settings::instance()->set_variant(VariantSetting_Locale, locale);
                    }
                }
                );
        };

        add_language_action(QLocale::English);
        add_language_action(QLocale::Russian);
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

    const bool auto_login = Settings::instance()->get_bool(BoolSetting_AutoLogin);
    if (auto_login) {
        const QString host = Settings::instance()->get_variant(VariantSetting_Host).toString();
        const QString domain = Settings::instance()->get_variant(VariantSetting_Domain).toString();

        if (!host.isEmpty()) {
            AdInterface::instance()->login(host, domain);
        }
    }    
}

void MainWindow::closeEvent(QCloseEvent *event) {
    const QByteArray geometry = saveGeometry();
    Settings::instance()->set_variant(VariantSetting_MainWindowGeometry, QVariant(geometry));
}

void MainWindow::on_action_exit() {
    const QString text = QString(tr("Are you sure you want to exit?"));
    const bool confirmed = confirmation_dialog(text, this);

    if (confirmed) {
        QApplication::closeAllWindows();
        QApplication::quit();
    }   
}
