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
#include "status.h"
#include "settings.h"
#include "adldap.h"
#include "globals.h"
#include "central_widget.h"
#include "about_dialog.h"
#include "manual_dialog.h"

#include <QApplication>
#include <QString>
#include <QStatusBar>
#include <QTextEdit>
#include <QAction>
#include <QDesktopWidget>
#include <QMenuBar>
#include <QActionGroup>
#include <QMessageBox>
#include <QDockWidget>

#define MESSAGE_LOG_OBJECT_NAME "MESSAGE_LOG_OBJECT_NAME"

MainWindow::MainWindow()
: QMainWindow()
{
    setStatusBar(g_status->status_bar());

    g_status->status_bar()->showMessage(tr("Ready"));

    message_log_dock = new QDockWidget();
    message_log_dock->setWindowTitle(tr("Message Log"));
    message_log_dock->setWidget(g_status->message_log());
    message_log_dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    message_log_dock->setObjectName(MESSAGE_LOG_OBJECT_NAME);
    addDockWidget(Qt::TopDockWidgetArea, message_log_dock);

    central_widget = new CentralWidget();
    setCentralWidget(central_widget);
    central_widget->setEnabled(false);

    setup_menubar();

    const QByteArray geometry = g_settings->get_variant(VariantSetting_MainWindowGeometry).toByteArray();
    restoreGeometry(geometry);

    const QByteArray state = g_settings->get_variant(VariantSetting_MainWindowState).toByteArray();
    restoreState(state);

    connect_to_server();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    const QByteArray geometry = saveGeometry();
    g_settings->set_variant(VariantSetting_MainWindowGeometry, geometry);

    const QByteArray state = saveState();
    g_settings->set_variant(VariantSetting_MainWindowState, state);
}

void MainWindow::setup_menubar() {
    auto menubar = new QMenuBar();
    setMenuBar(menubar);

    // Create dialogs opened from menubar
    auto manual_dialog = new ManualDialog(this);
    auto about_dialog = new AboutDialog(this);

    //
    // Create actions
    //
    connect_action = new QAction(tr("&Connect"), this);
    auto quit_action = new QAction(tr("&Quit"), this);

    auto manual_action = new QAction(tr("&Manual"), this);
    auto about_action = new QAction(tr("&About ADMC"), this);

    auto advanced_features_action = new QAction(tr("&Advanced Features"), this);
    auto confirm_actions_action = new QAction(tr("&Confirm actions"), this);
    auto last_before_first_name_action = new QAction(tr("&Put last name before first name when creating users"), this);
    auto toggle_console_tree_action = new QAction(tr("Console Tree"), this);
    auto toggle_description_bar_action = new QAction(tr("Description Bar"), this);

    const QList<QLocale::Language> language_list = {
        QLocale::English,
        QLocale::Russian,
    };
    const QHash<QLocale::Language, QAction *> language_actions =
    [this, language_list]() {
        QHash<QLocale::Language, QAction *> out;

        auto language_group = new QActionGroup(this);
        for (const auto language : language_list) {
            QLocale locale(language);
            const QString language_name =
            [locale]() {
            // NOTE: Russian nativeLanguageName starts with lowercase letter for some reason
                QString name_out = locale.nativeLanguageName();

                const QChar first_letter_uppercased = name_out[0].toUpper();

                name_out.replace(0, 1, first_letter_uppercased);

                return name_out;
            }();

            const auto action = new QAction(language_name, language_group);
            action->setCheckable(true);
            language_group->addAction(action);

            const bool is_checked =
            [=]() {
                const QLocale current_locale = g_settings->get_variant(VariantSetting_Locale).toLocale();

                return (current_locale == locale);
            }();
            action->setChecked(is_checked);

            out[language] = action;
        }

        return out;
    }();

    //
    // Create menus
    //
    // NOTE: for menu's that are obtained from console, we
    // don't add actions. Instead the console adds actions
    // to them.
    auto file_menu = menubar->addMenu(tr("&File"));
    auto action_menu = menubar->addMenu(tr("&Action"));
    auto navigation_menu = menubar->addMenu(tr("&Navigation"));
    auto view_menu = menubar->addMenu(tr("&View"));
    auto preferences_menu = menubar->addMenu(tr("&Preferences"));
    auto language_menu = new QMenu(tr("&Language"));
    auto help_menu = menubar->addMenu(tr("&Help"));

    //
    // Fill menus
    //
    file_menu->addAction(connect_action);
    file_menu->addAction(quit_action);

    central_widget->add_actions_to_action_menu(action_menu);
    central_widget->add_actions_to_navigation_menu(navigation_menu);
    central_widget->add_actions_to_view_menu(view_menu);

    preferences_menu->addAction(advanced_features_action);
    preferences_menu->addAction(confirm_actions_action);
    preferences_menu->addAction(last_before_first_name_action);
    preferences_menu->addMenu(language_menu);
    preferences_menu->addSeparator();
    preferences_menu->addAction(message_log_dock->toggleViewAction());
    preferences_menu->addAction(toggle_console_tree_action);
    preferences_menu->addAction(toggle_description_bar_action);

    for (const auto language : language_list) {
        QAction *language_action = language_actions[language];
        language_menu->addAction(language_action);
    }

    help_menu->addAction(manual_action);
    help_menu->addAction(about_action);
    
    //
    // Connect actions
    //

    connect(
        connect_action, &QAction::triggered,
        this, &MainWindow::connect_to_server);
    connect(
        quit_action, &QAction::triggered,
        this, &MainWindow::quit);
    connect(
        manual_action, &QAction::triggered,
        manual_dialog, &QDialog::open);
    connect(
        about_action, &QAction::triggered,
        about_dialog, &QDialog::open);
    g_settings->connect_action_to_bool_setting(advanced_features_action, BoolSetting_AdvancedFeatures);
    g_settings->connect_action_to_bool_setting(confirm_actions_action, BoolSetting_ConfirmActions);
    g_settings->connect_action_to_bool_setting(last_before_first_name_action, BoolSetting_LastNameBeforeFirstName);
    g_settings->connect_action_to_bool_setting(toggle_console_tree_action, BoolSetting_ShowConsoleTree);
    g_settings->connect_action_to_bool_setting(toggle_description_bar_action, BoolSetting_ShowResultsHeader);

    for (const auto language : language_actions.keys()) {
        QAction *action = language_actions[language];

        connect(
            action, &QAction::toggled,
            [this, language](bool checked) {
                if (checked) {
                    g_settings->set_variant(VariantSetting_Locale, QLocale(language));

                    QMessageBox::information(this, tr("Info"), tr("App needs to be restarted for the language option to take effect."));
                }
            });
    }

    // Open action menu as context menu for central widget
    connect(
        central_widget, &CentralWidget::context_menu,
        [action_menu](const QPoint pos) {
            action_menu->exec(pos);
        });
}

void MainWindow::connect_to_server() {
    AdInterface ad;
    if (ad_connected(ad)) {
        // TODO: check for load failure
        const QLocale locale = g_settings->get_variant(VariantSetting_Locale).toLocale();
        g_adconfig->load(ad, locale);

        AdInterface::set_permanent_adconfig(g_adconfig);

        g_status->display_ad_messages(ad, this);

        central_widget->go_online(ad);
        central_widget->setEnabled(true);
        connect_action->setEnabled(false);
    }
}

void MainWindow::quit() {
    close();
}
