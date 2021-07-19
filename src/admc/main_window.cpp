/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

#include "about_dialog.h"
#include "adldap.h"
#include "central_widget.h"
#include "globals.h"
#include "manual_dialog.h"
#include "settings.h"
#include "status.h"
#include "utils.h"

#include <QAction>
#include <QActionGroup>
#include <QDebug>
#include <QDockWidget>
#include <QMenuBar>
#include <QStatusBar>
#include <QTextEdit>

#define MESSAGE_LOG_OBJECT_NAME "MESSAGE_LOG_OBJECT_NAME"

MainWindow::MainWindow()
: QMainWindow() {
    setStatusBar(g_status()->status_bar());

    message_log_dock = new QDockWidget();
    message_log_dock->setWindowTitle(tr("Message Log"));
    message_log_dock->setWidget(g_status()->message_log());
    message_log_dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    message_log_dock->setObjectName(MESSAGE_LOG_OBJECT_NAME);
    addDockWidget(Qt::TopDockWidgetArea, message_log_dock);

    central_widget = new CentralWidget();
    setCentralWidget(central_widget);
    central_widget->setEnabled(false);

    setup_menubar();

    if (g_settings->contains_variant(VariantSetting_MainWindowGeometry)) {
        const QByteArray geometry = g_settings->get_variant(VariantSetting_MainWindowGeometry).toByteArray();
        restoreGeometry(geometry);
    } else {
        resize(1024, 768);
    }

    if (g_settings->contains_variant(VariantSetting_MainWindowState)) {
        const QByteArray state = g_settings->get_variant(VariantSetting_MainWindowState).toByteArray();
        restoreState(state);
    } else {
        message_log_dock->hide();
    }

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
    auto log_searches_action = new QAction(tr("Log searches"), this);
    auto timestamp_log_action = new QAction(tr("Timestamps in message log"), this);
    auto toggle_console_tree_action = new QAction(tr("Console Tree"), this);
    auto toggle_description_bar_action = new QAction(tr("Description Bar"), this);

    const QList<QLocale::Language> language_list = {
        QLocale::English,
        QLocale::Russian,
    };
    const QHash<QLocale::Language, QAction *> language_actions = [this, language_list]() {
        QHash<QLocale::Language, QAction *> out;

        auto language_group = new QActionGroup(this);
        for (const auto language : language_list) {
            QLocale locale(language);
            const QString language_name = [locale]() {
                // NOTE: Russian nativeLanguageName starts with lowercase letter for some reason
                QString name_out = locale.nativeLanguageName();

                const QChar first_letter_uppercased = name_out[0].toUpper();

                name_out.replace(0, 1, first_letter_uppercased);

                return name_out;
            }();

            const auto action = new QAction(language_name, language_group);
            action->setCheckable(true);
            language_group->addAction(action);

            const bool is_checked = [=]() {
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
    preferences_menu->addAction(log_searches_action);
    preferences_menu->addAction(timestamp_log_action);
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
    g_settings->connect_action_to_bool_setting(log_searches_action, BoolSetting_LogSearches);
    g_settings->connect_action_to_bool_setting(timestamp_log_action, BoolSetting_TimestampLog);
    g_settings->connect_action_to_bool_setting(toggle_console_tree_action, BoolSetting_ShowConsoleTree);
    g_settings->connect_action_to_bool_setting(toggle_description_bar_action, BoolSetting_ShowResultsHeader);

    for (const auto language : language_actions.keys()) {
        QAction *action = language_actions[language];

        connect(
            action, &QAction::toggled,
            [this, language](bool checked) {
                if (checked) {
                    g_settings->set_variant(VariantSetting_Locale, QLocale(language));

                    message_box_information(this, tr("Info"), tr("Restart the app to switch to the selected language."));
                }
            });
    }

    connect(
        g_settings->get_bool_signal(BoolSetting_LogSearches), &BoolSettingSignal::changed,
        this, &MainWindow::on_log_searches_changed);
    on_log_searches_changed();
}

void MainWindow::connect_to_server() {
    const QString saved_dc = g_settings->get_variant(VariantSetting_DC).toString();
    AdInterface::set_dc(saved_dc);

    AdInterface ad;
    if (ad_connected(ad)) {
        // TODO: check for load failure
        const QLocale locale = g_settings->get_variant(VariantSetting_Locale).toLocale();
        g_adconfig->load(ad, locale);

        qDebug() << "domain =" << g_adconfig->domain();

        AdInterface::set_permanent_adconfig(g_adconfig);

        g_status()->display_ad_messages(ad, this);

        central_widget->go_online(ad);
        
        central_widget->setEnabled(true);
        connect_action->setEnabled(false);
    }
}

void MainWindow::quit() {
    close();
}

void MainWindow::on_log_searches_changed() {
    const bool log_searches_ON = g_settings->get_bool(BoolSetting_LogSearches);

    AdInterface::set_log_searches(log_searches_ON);
}
