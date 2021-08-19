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
#include "connection_options_dialog.h"
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
#include <QToolBar>

#define MESSAGE_LOG_OBJECT_NAME "MESSAGE_LOG_OBJECT_NAME"

MainWindow::MainWindow()
: QMainWindow() {
    toolbar = new QToolBar(this);
    toolbar->setObjectName("main_window_toolbar");
    toolbar->setWindowTitle(tr("Toolbar"));
    addToolBar(toolbar);

    setStatusBar(g_status()->status_bar());

    connection_options_dialog = new ConnectionOptionsDialog(this);

    manual_action = new QAction(QIcon::fromTheme("help-faq"), tr("&Manual"), this);

    message_log_dock = new QDockWidget();
    message_log_dock->setWindowTitle(tr("Message Log"));
    message_log_dock->setWidget(g_status()->message_log());
    message_log_dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    message_log_dock->setObjectName(MESSAGE_LOG_OBJECT_NAME);
    addDockWidget(Qt::TopDockWidgetArea, message_log_dock);

    setCentralWidget(new QWidget(this));

    setup_menubar();

    const bool restored_geometry = settings_restore_geometry(SETTING_main_window_geometry, this);
    if (!restored_geometry) {
        resize(1024, 768);
    }

    const QByteArray state = settings_get_variant(SETTING_main_window_state).toByteArray();
    if (!state.isEmpty()) {
        restoreState(state);
    } else {
        message_log_dock->hide();
    }

    connect(
        connection_options_dialog, &QDialog::accepted,
        this, &MainWindow::load_connection_options);
    load_connection_options();

    connect_to_server();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    const QByteArray geometry = saveGeometry();
    settings_set_variant(SETTING_main_window_geometry, geometry);

    const QByteArray state = saveState();
    settings_set_variant(SETTING_main_window_state, state);

    QMainWindow::closeEvent(event);
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
    auto connection_options_action = new QAction(tr("Connection options"), this);
    auto quit_action = new QAction(tr("&Quit"), this);
    quit_action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));

    auto about_action = new QAction(tr("&About ADMC"), this);

    auto confirm_actions_action = settings_make_and_connect_action(SETTING_confirm_actions, tr("&Confirm actions"), this);
    auto last_before_first_name_action = settings_make_and_connect_action(SETTING_last_name_before_first_name, tr("&Put last name before first name when creating users"), this);
    auto log_searches_action = settings_make_and_connect_action(SETTING_log_searches, tr("Log searches"), this);
    auto timestamp_log_action = settings_make_and_connect_action(SETTING_timestamp_log, tr("Timestamps in message log"), this);

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
                const QLocale current_locale = settings_get_variant(SETTING_locale).toLocale();

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
    action_menu = menubar->addMenu(tr("&Action"));
    navigation_menu = menubar->addMenu(tr("&Navigation"));
    view_menu = menubar->addMenu(tr("&View"));
    preferences_menu = menubar->addMenu(tr("&Preferences"));
    auto language_menu = new QMenu(tr("&Language"));
    auto help_menu = menubar->addMenu(tr("&Help"));

    //
    // Fill menus
    //
    file_menu->addAction(connect_action);
    file_menu->addAction(connection_options_action);
    file_menu->addAction(quit_action);

    preferences_menu->addAction(confirm_actions_action);
    preferences_menu->addAction(last_before_first_name_action);
    preferences_menu->addAction(log_searches_action);
    preferences_menu->addAction(timestamp_log_action);
    preferences_menu->addMenu(language_menu);
    preferences_menu->addSeparator();
    preferences_menu->addAction(message_log_dock->toggleViewAction());
    preferences_menu->addAction(toolbar->toggleViewAction());

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
        connection_options_action, &QAction::triggered,
        connection_options_dialog, &QDialog::open);
    connect(
        quit_action, &QAction::triggered,
        this, &MainWindow::close);
    connect(
        manual_action, &QAction::triggered,
        manual_dialog, &QDialog::show);
    connect(
        about_action, &QAction::triggered,
        about_dialog, &QDialog::open);

    for (const auto language : language_actions.keys()) {
        QAction *action = language_actions[language];

        connect(
            action, &QAction::toggled,
            [this, language](bool checked) {
                if (checked) {
                    settings_set_variant(SETTING_locale, QLocale(language));

                    message_box_information(this, tr("Info"), tr("Restart the app to switch to the selected language."));
                }
            });
    }

    connect(
        log_searches_action, &QAction::toggled,
        this, &MainWindow::on_log_searches_changed);
    on_log_searches_changed();
}

void MainWindow::connect_to_server() {
    const QString saved_dc = settings_get_variant(SETTING_dc).toString();
    AdInterface::set_dc(saved_dc);

    AdInterface ad;
    if (ad_connected(ad)) {
        // TODO: check for load failure
        const QLocale locale = settings_get_variant(SETTING_locale).toLocale();
        g_adconfig->load(ad, locale);

        qDebug() << "domain =" << g_adconfig->domain();

        AdInterface::set_permanent_adconfig(g_adconfig);

        g_status()->display_ad_messages(ad, this);

        auto central_widget = new CentralWidget(ad);
        setCentralWidget(central_widget);

        central_widget->add_actions(action_menu, navigation_menu, view_menu, preferences_menu, toolbar);
        
        toolbar->addAction(manual_action);

        connect_action->setEnabled(false);
    }
}

void MainWindow::on_log_searches_changed() {
    const bool log_searches_ON = settings_get_bool(SETTING_log_searches);

    AdInterface::set_log_searches(log_searches_ON);
}

void MainWindow::load_connection_options() {
    const QVariant sasl_nocanon = settings_get_variant(SETTING_sasl_nocanon);
    if (sasl_nocanon.isValid()) {
        AdInterface::set_sasl_nocanon(sasl_nocanon.toBool());
    } else {
        AdInterface::set_sasl_nocanon(true);
    }

    const QVariant port = settings_get_variant(SETTING_port);
    if (port.isValid()) {
        AdInterface::set_port(port.toString());
    } else {
        AdInterface::set_port(QString());
    }

    const QString cert_strategy_string = settings_get_variant(SETTING_cert_strategy).toString();
    const QHash<QString, CertStrategy> cert_strategy_map = {
        {"never", CertStrategy_Never},
        {"hard", CertStrategy_Hard},
        {"demand", CertStrategy_Demand},
        {"allow", CertStrategy_Allow},
        {"try", CertStrategy_Try},
    };
    const CertStrategy cert_strategy = cert_strategy_map.value(cert_strategy_string, CertStrategy_Never);
    AdInterface::set_cert_strategy(cert_strategy);
}
