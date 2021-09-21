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
#include "globals.h"
#include "manual_dialog.h"
#include "connection_options_dialog.h"
#include "settings.h"
#include "status.h"
#include "utils.h"
#include "console_impls/object_impl.h"
#include "console_impls/policy_impl.h"
#include "console_impls/policy_root_impl.h"
#include "console_impls/query_item_impl.h"
#include "console_impls/query_folder_impl.h"
#include "console_widget/console_widget.h"
#include "filter_dialog.h"
#include "console_impls/item_type.h"

#include <QAction>
#include <QActionGroup>
#include <QDebug>
#include <QDockWidget>
#include <QMenuBar>
#include <QStatusBar>
#include <QTextEdit>
#include <QToolBar>
#include <QModelIndex>

#define MESSAGE_LOG_OBJECT_NAME "MESSAGE_LOG_OBJECT_NAME"

MainWindow::MainWindow()
: QMainWindow() {
    setStatusBar(g_status()->status_bar());

    auto message_log_dock = new QDockWidget();
    message_log_dock->setWindowTitle(tr("Message Log"));
    message_log_dock->setWidget(g_status()->message_log());
    message_log_dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    message_log_dock->setObjectName(MESSAGE_LOG_OBJECT_NAME);
    addDockWidget(Qt::TopDockWidgetArea, message_log_dock);

    //
    // Console
    //
    console = new ConsoleWidget(this);
    setCentralWidget(console);

    object_impl = new ObjectImpl(console);
    console->register_impl(ItemType_Object, object_impl);

    auto policy_root_impl = new PolicyRootImpl(console);
    console->register_impl(ItemType_PolicyRoot, policy_root_impl);

    auto policy_impl = new PolicyImpl(console);
    console->register_impl(ItemType_Policy, policy_impl);

    auto query_item_impl = new QueryItemImpl(console);
    console->register_impl(ItemType_QueryItem, query_item_impl);

    auto query_folder_impl = new QueryFolderImpl(console);
    console->register_impl(ItemType_QueryFolder, query_folder_impl);

    object_impl->set_policy_impl(policy_impl);

    //
    // Menubar
    //
    auto menubar = new QMenuBar();
    setMenuBar(menubar);

    // Create dialogs opened from menubar
    auto manual_dialog = new ManualDialog(this);
    auto about_dialog = new AboutDialog(this);
    auto connection_options_dialog = new ConnectionOptionsDialog(this);

    //
    // Create actions
    //
    open_filter_action = new QAction(tr("&Filter objects..."), this);

    // NOTE: not using settings_make_and_connect_action()
    // because they need to be connected to a custom slot
    dev_mode_action = settings_make_action(SETTING_dev_mode, tr("Dev mode"), this);
    show_noncontainers_action = settings_make_action(SETTING_show_non_containers_in_console_tree, tr("&Show non-container objects in Console tree"), this);
    advanced_features_action = settings_make_action(SETTING_advanced_features, tr("Advanced features"), this);

    connect_action = new QAction(tr("&Connect"), this);
    auto connection_options_action = new QAction(tr("Connection options"), this);
    auto quit_action = new QAction(tr("&Quit"), this);
    quit_action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));

    auto about_action = new QAction(tr("&About ADMC"), this);

    auto confirm_actions_action = settings_make_and_connect_action(SETTING_confirm_actions, tr("&Confirm actions"), this);
    auto last_before_first_name_action = settings_make_and_connect_action(SETTING_last_name_before_first_name, tr("&Put last name before first name when creating users"), this);
    auto log_searches_action = settings_make_and_connect_action(SETTING_log_searches, tr("Log searches"), this);
    auto timestamp_log_action = settings_make_and_connect_action(SETTING_timestamp_log, tr("Timestamps in message log"), this);

    auto manual_action = new QAction(QIcon::fromTheme("help-contents"), tr("&Manual"), this);

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
    auto file_menu = menubar->addMenu(tr("&File"));
    auto action_menu = menubar->addMenu(tr("&Action"));
    auto view_menu = menubar->addMenu(tr("&View"));
    auto preferences_menu = menubar->addMenu(tr("&Preferences"));
    auto language_menu = new QMenu(tr("&Language"));
    auto help_menu = menubar->addMenu(tr("&Help"));

    auto toolbar = new QToolBar(this);
    addToolBar(toolbar);
    toolbar->setObjectName("main_window_toolbar");
    toolbar->setWindowTitle(tr("Toolbar"));

    //
    // Add actions
    //

    // File
    file_menu->addAction(connect_action);
    file_menu->addAction(connection_options_action);
    file_menu->addAction(quit_action);

    // Action
    console->add_actions(action_menu);

    // View
    view_menu->addAction(console->set_results_to_icons_action());
    view_menu->addAction(console->set_results_to_list_action());
    view_menu->addAction(console->set_results_to_detail_action());
    view_menu->addSeparator();
    view_menu->addAction(console->customize_columns_action());
    view_menu->addAction(open_filter_action);

    // Preferences
    #ifdef QT_DEBUG
        preferences_menu->addAction(dev_mode_action);
    #endif
    preferences_menu->addAction(advanced_features_action);
    preferences_menu->addAction(confirm_actions_action);
    preferences_menu->addAction(last_before_first_name_action);
    preferences_menu->addAction(log_searches_action);
    preferences_menu->addAction(timestamp_log_action);
    preferences_menu->addAction(show_noncontainers_action);
    preferences_menu->addMenu(language_menu);
    preferences_menu->addSeparator();
    preferences_menu->addAction(message_log_dock->toggleViewAction());
    preferences_menu->addAction(toolbar->toggleViewAction());
    preferences_menu->addAction(console->toggle_console_tree_action());
    preferences_menu->addAction(console->toggle_description_bar_action());

    for (const auto language : language_list) {
        QAction *language_action = language_actions[language];
        language_menu->addAction(language_action);
    }

    // Help
    help_menu->addAction(manual_action);
    help_menu->addAction(about_action);

    // Toolbar
    toolbar->addAction(console->navigate_back_action());
    toolbar->addAction(console->navigate_forward_action());
    toolbar->addAction(console->navigate_up_action());
    toolbar->addSeparator();
    toolbar->addAction(console->refresh_current_scope_action());
    toolbar->addAction(manual_action);

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
    connect(
        show_noncontainers_action, &QAction::toggled,
        this, &MainWindow::on_show_non_containers);
    connect(
        dev_mode_action, &QAction::toggled,
        this, &MainWindow::on_dev_mode);
    connect(
        advanced_features_action, &QAction::toggled,
        this, &MainWindow::on_advanced_features);

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

    connect(
        connection_options_dialog, &QDialog::accepted,
        this, &MainWindow::load_connection_options);
    load_connection_options();

    connect(
        action_menu, &QMenu::aboutToShow,
        console, &ConsoleWidget::update_actions);

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

    const QVariant console_widget_state = settings_get_variant(SETTING_console_widget_state);
    console->restore_state(console_widget_state);

    connect_to_server();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    const QByteArray geometry = saveGeometry();
    settings_set_variant(SETTING_main_window_geometry, geometry);

    const QByteArray state = saveState();
    settings_set_variant(SETTING_main_window_state, state);

    const QVariant console_state = console->save_state();
    settings_set_variant(SETTING_console_widget_state, console_state);

    QMainWindow::closeEvent(event);
}

// NOTE: f-ns below need to manually change a setting and
// then refresh_object_tree() because setting needs to be
// changed before tree is refreshed. If you do this in a
// more convenient way by connecting as a slot, call order
// will be undefined.

void MainWindow::on_show_non_containers() {
    settings_set_bool(SETTING_show_non_containers_in_console_tree, show_noncontainers_action->isChecked());

    refresh_object_tree();
}

void MainWindow::on_dev_mode() {
    settings_set_bool(SETTING_dev_mode, dev_mode_action->isChecked());

    refresh_object_tree();
}

void MainWindow::on_advanced_features() {
    settings_set_bool(SETTING_advanced_features, advanced_features_action->isChecked());

    refresh_object_tree();
}

void MainWindow::on_filter_dialog_accepted() {
    if (filter_dialog->filtering_ON()) {
        const QString filter = filter_dialog->get_filter();
        object_impl->enable_filtering(filter);
    } else {
        object_impl->disable_filtering();
    }
    
    refresh_object_tree();
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
        AdInterface::set_port(port.toInt());
    } else {
        AdInterface::set_port(0);
    }

    const QString cert_strategy_string = settings_get_variant(SETTING_cert_strategy).toString();
    const QHash<QString, CertStrategy> cert_strategy_map = {
        {CERT_STRATEGY_NEVER, CertStrategy_Never},
        {CERT_STRATEGY_HARD, CertStrategy_Hard},
        {CERT_STRATEGY_DEMAND, CertStrategy_Demand},
        {CERT_STRATEGY_ALLOW, CertStrategy_Allow},
        {CERT_STRATEGY_TRY, CertStrategy_Try},
    };
    const CertStrategy cert_strategy = cert_strategy_map.value(cert_strategy_string, CertStrategy_Never);
    AdInterface::set_cert_strategy(cert_strategy);
}

void MainWindow::connect_to_server() {
    const QString saved_dc = settings_get_variant(SETTING_dc).toString();
    AdInterface::set_dc(saved_dc);

    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    // TODO: check for load failure
    const QLocale locale = settings_get_variant(SETTING_locale).toLocale();
    g_adconfig->load(ad, locale);
    AdInterface::set_config(g_adconfig);

    qDebug() << "domain =" << g_adconfig->domain();

    // Load console tree's
    console_object_tree_init(console, ad);
    console_policy_tree_init(console, ad);
    console_query_tree_init(console);

    // Set current scope to object head to load it
    const QModelIndex object_tree_root = get_object_tree_root(console);
    if (object_tree_root.isValid()) {
        console->set_current_scope(object_tree_root);
    }

    // Display any errors that happened when loading the
    // console
    g_status()->display_ad_messages(ad, this);

    // NOTE: have to create filter dialog here because it
    // requires an AD connection
    filter_dialog = new FilterDialog(ad.adconfig(), this);

    connect(
        open_filter_action, &QAction::triggered,
        filter_dialog, &QDialog::open);
    connect(
        filter_dialog, &QDialog::accepted,
        this, &MainWindow::on_filter_dialog_accepted);
    on_filter_dialog_accepted();
    
    // Disable connect action once connected because
    // it's not needed at that point
    connect_action->setEnabled(false);
}

void MainWindow::refresh_object_tree() {
    const QModelIndex object_tree_root = get_object_tree_root(console);
    if (!object_tree_root.isValid()) {
        return;
    }

    show_busy_indicator();

    console->refresh_scope(object_tree_root);

    hide_busy_indicator();
}
