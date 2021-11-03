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
#include "ui_main_window.h"

#include "about_dialog.h"
#include "adldap.h"
#include "changelog_dialog.h"
#include "config.h"
#include "connection_options_dialog.h"
#include "console_filter_dialog.h"
#include "console_impls/item_type.h"
#include "console_impls/object_impl.h"
#include "console_impls/policy_impl.h"
#include "console_impls/policy_root_impl.h"
#include "console_impls/query_folder_impl.h"
#include "console_impls/query_item_impl.h"
#include "console_widget/console_widget.h"
#include "edits/country_combo.h"
#include "globals.h"
#include "manual_dialog.h"
#include "settings.h"
#include "status.h"
#include "utils.h"

#include <QDebug>
#include <QLabel>
#include <QModelIndex>

MainWindow::MainWindow()
: QMainWindow() {
    ui = new Ui::MainWindow();
    ui->setupUi(this);

    country_combo_load_data();

    g_status()->init(ui->statusbar, ui->message_log_edit);

    client_user_label = new QLabel();
    ui->statusbar->addPermanentWidget(client_user_label);

    auto action_show_client_user = new QAction("Show Client User", this);
    action_show_client_user->setCheckable(true);
    ui->statusbar->addAction(action_show_client_user);

    filter_dialog = new ConsoleFilterDialog(this);

    // NOTE: disable console filter until connection because
    // connection is required to init the dialog
    ui->action_filter_objects->setEnabled(false);

    //
    // Console
    //
    object_impl = new ObjectImpl(ui->console);
    ui->console->register_impl(ItemType_Object, object_impl);

    auto policy_root_impl = new PolicyRootImpl(ui->console);
    ui->console->register_impl(ItemType_PolicyRoot, policy_root_impl);

    auto policy_impl = new PolicyImpl(ui->console);
    ui->console->register_impl(ItemType_Policy, policy_impl);

    query_item_impl = new QueryItemImpl(ui->console);
    ui->console->register_impl(ItemType_QueryItem, query_item_impl);

    query_folder_impl = new QueryFolderImpl(ui->console);
    ui->console->register_impl(ItemType_QueryFolder, query_folder_impl);

    object_impl->set_policy_impl(policy_impl);
    query_item_impl->set_query_folder_impl(query_folder_impl);

    // Create dialogs opened from menubar
    auto manual_dialog = new ManualDialog(this);
    auto changelog_dialog = new ChangelogDialog(this);
    auto about_dialog = new AboutDialog(this);
    auto connection_options_dialog = new ConnectionOptionsDialog(this);

    // Setup console
    const ConsoleWidgetActions console_actions = [&]() {
        ConsoleWidgetActions out;

        out.navigate_up = ui->action_navigate_up;
        out.navigate_back = ui->action_navigate_back;
        out.navigate_forward = ui->action_navigate_forward;
        out.refresh = ui->action_refresh;
        out.customize_columns = ui->action_customize_columns;
        out.view_icons = ui->action_view_icons;
        out.view_list = ui->action_view_list;
        out.view_detail = ui->action_view_detail;
        out.toggle_console_tree = ui->action_toggle_console_tree;
        out.toggle_description_bar = ui->action_toggle_description_bar;

        return out;
    }();

    ui->console->set_actions(console_actions);

    // Create language actions
    const QList<QLocale::Language> language_list = {
        QLocale::English,
        QLocale::Russian,
    };
    language_action_map = [this, language_list]() {
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

    for (const auto language : language_list) {
        QAction *language_action = language_action_map[language];
        ui->menu_language->addAction(language_action);
    }

    // NOTE: "Action" menu actions need to be filled by the
    // console
    ui->console->add_actions(ui->menu_action);

#ifndef QT_DEBUG
    ui->action_dev_mode->setVisible(false);
#endif

    // NOTE: toolbar and message log(dock widget) have built
    // in toggle actions, but there's no way to add them
    // through designer so add them here.
    ui->menu_view->insertAction(ui->action_toggle_message_log, ui->message_log->toggleViewAction());
    ui->menu_view->insertAction(ui->action_toggle_toolbar, ui->toolbar->toggleViewAction());
    ui->menu_view->removeAction(ui->action_toggle_message_log);
    ui->menu_view->removeAction(ui->action_toggle_toolbar);

    //
    // Connect actions
    //
    connect(
        ui->action_connect, &QAction::triggered,
        this, &MainWindow::connect_to_server);
    connect(
        ui->action_connection_options, &QAction::triggered,
        connection_options_dialog, &QDialog::open);
    connect(
        ui->action_quit, &QAction::triggered,
        this, &MainWindow::close);
    connect(
        ui->action_manual, &QAction::triggered,
        manual_dialog, &QDialog::show);
    connect(
        ui->action_changelog, &QAction::triggered,
        changelog_dialog, &QDialog::show);
    connect(
        ui->action_about, &QAction::triggered,
        about_dialog, &QDialog::open);

    settings_connect_action_to_bool_setting(ui->action_confirm_actions, SETTING_confirm_actions);
    settings_connect_action_to_bool_setting(ui->action_last_name_order, SETTING_last_name_before_first_name);
    settings_connect_action_to_bool_setting(ui->action_log_searches, SETTING_log_searches);
    settings_connect_action_to_bool_setting(ui->action_timestamps, SETTING_timestamp_log);
    settings_connect_action_to_bool_setting(action_show_client_user, SETTING_show_client_user);

    // NOTE: not using
    // settings_connect_action_to_bool_setting() for these
    // actions because they need custom slots
    connect(
        ui->action_show_noncontainers, &QAction::toggled,
        this, &MainWindow::on_show_non_containers);
    connect(
        ui->action_advanced_features, &QAction::toggled,
        this, &MainWindow::on_advanced_features);
    connect(
        ui->action_dev_mode, &QAction::toggled,
        this, &MainWindow::on_dev_mode);

    for (const auto language : language_action_map.keys()) {
        QAction *action = language_action_map[language];

        connect(
            action, &QAction::toggled,
            this, &MainWindow::on_language_action);
    }

    connect(
        ui->action_log_searches, &QAction::triggered,
        this, &MainWindow::on_log_searches_changed);
    on_log_searches_changed();

    connect(
        action_show_client_user, &QAction::triggered,
        this, &MainWindow::on_show_client_user);
    on_show_client_user();

    connect(
        connection_options_dialog, &QDialog::accepted,
        this, &MainWindow::load_connection_options);
    load_connection_options();

    connect(
        ui->action_filter_objects, &QAction::triggered,
        filter_dialog, &QDialog::open);
    connect(
        filter_dialog, &QDialog::accepted,
        this, &MainWindow::on_filter_dialog_accepted);

    connect(
        ui->menu_action, &QMenu::aboutToShow,
        ui->console, &ConsoleWidget::update_actions);

    const bool restored_geometry = settings_restore_geometry(SETTING_main_window_geometry, this);
    if (!restored_geometry) {
        resize(1024, 768);
    }

    const QByteArray state = settings_get_variant(SETTING_main_window_state).toByteArray();
    if (!state.isEmpty()) {
        restoreState(state);
    } else {
        // Hide message log by default
        ui->message_log->hide();
    }

    const QVariant console_widget_state = settings_get_variant(SETTING_console_widget_state);
    ui->console->restore_state(console_widget_state);

    connect_to_server();

    const bool first_time_opening_this_version = []() {
        const QString last_version = settings_get_variant(SETTING_last_opened_version).toString();

        return (last_version != ADMC_VERSION);
    }();
    if (first_time_opening_this_version) {
        settings_set_variant(SETTING_last_opened_version, ADMC_VERSION);
        changelog_dialog->show();
    }
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    const QByteArray geometry = saveGeometry();
    settings_set_variant(SETTING_main_window_geometry, geometry);

    const QByteArray state = saveState();
    settings_set_variant(SETTING_main_window_state, state);

    const QVariant console_state = ui->console->save_state();
    settings_set_variant(SETTING_console_widget_state, console_state);

    QMainWindow::closeEvent(event);
}

// NOTE: f-ns below need to manually change a setting and
// then refresh_object_tree() because setting needs to be
// changed before tree is refreshed. If you do this in a
// more convenient way by connecting as a slot, call order
// will be undefined.

void MainWindow::on_show_non_containers(bool checked) {
    settings_set_bool(SETTING_show_non_containers_in_console_tree, checked);

    refresh_object_tree();
}

void MainWindow::on_dev_mode(bool checked) {
    settings_set_bool(SETTING_dev_mode, checked);

    refresh_object_tree();
}

void MainWindow::on_advanced_features(bool checked) {
    settings_set_bool(SETTING_advanced_features, checked);

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

void MainWindow::on_show_client_user() {
    const bool visible = settings_get_bool(SETTING_show_client_user);
    client_user_label->setVisible(visible);
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

void MainWindow::on_language_action(bool checked) {
    // NOTE: since language actions are part of action
    // group, toggling one affects others, so to do this
    // slot once do this check.
    if (!checked) {
        return;
    }

    const QLocale::Language selected_language = [&]() {
        for (const QLocale::Language &language : language_action_map.keys()) {
            QAction *action = language_action_map[language];

            if (action->isChecked()) {
                return language;
            }
        }

        return QLocale::English;
    }();

    settings_set_variant(SETTING_locale, QLocale(selected_language));

    message_box_information(this, tr("Info"), tr("Restart the app to switch to the selected language."));
}

void MainWindow::connect_to_server() {
    const QString saved_dc = settings_get_variant(SETTING_dc).toString();
    AdInterface::set_dc(saved_dc);

    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    client_user_label->setText(ad.client_user());

    const QLocale locale = settings_get_variant(SETTING_locale).toLocale();
    g_adconfig->load(ad, locale);
    AdInterface::set_config(g_adconfig);

    qDebug() << "domain =" << g_adconfig->domain();

    // Load console tree's
    console_object_tree_init(ui->console, ad);
    console_policy_tree_init(ui->console);
    console_query_tree_init(ui->console);

    // Set current scope to object head to load it
    const QModelIndex object_tree_root = get_object_tree_root(ui->console);
    if (object_tree_root.isValid()) {
        ui->console->set_current_scope(object_tree_root);
    }

    // Display any errors that happened when loading the
    // console
    g_status()->display_ad_messages(ad, this);

    filter_dialog->init(ad.adconfig());
    on_filter_dialog_accepted();

    query_item_impl->init(ad.adconfig());
    query_folder_impl->init(ad.adconfig());

    // Disable connect action once connected because
    // it's not needed at that point
    ui->action_connect->setEnabled(false);

    ui->action_filter_objects->setEnabled(true);

    // NOTE: need to restore console state again after
    // successful connection because some state like column
    // visibility requires models to be populated with
    // items. Until connection to the domain, the object
    // tree is empty.
    const QVariant console_widget_state = settings_get_variant(SETTING_console_widget_state);
    ui->console->restore_state(console_widget_state);
}

void MainWindow::refresh_object_tree() {
    const QModelIndex object_tree_root = get_object_tree_root(ui->console);
    if (!object_tree_root.isValid()) {
        return;
    }

    show_busy_indicator();

    ui->console->refresh_scope(object_tree_root);

    hide_busy_indicator();
}
