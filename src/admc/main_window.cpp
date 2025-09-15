/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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
#include "attribute_edits/country_combo.h"
#include "changelog_dialog.h"
#include "config.h"
#include "connection_options_dialog.h"
#include "console_impls/all_policies_folder_impl.h"
#include "console_impls/item_type.h"
#include "console_impls/object_impl.h"
#include "console_impls/policy_impl.h"
#include "console_impls/policy_ou_impl.h"
#include "console_impls/policy_root_impl.h"
#include "console_impls/query_folder_impl.h"
#include "console_impls/query_item_impl.h"
#include "console_widget/console_widget.h"
#include "fsmo/fsmo_dialog.h"
#include "globals.h"
#include "settings.h"
#include "status.h"
#include "utils.h"
#include "fsmo/fsmo_utils.h"
#include "managers/icon_manager.h"
#include "console_impls/domain_info_impl.h"
#include "auth_dialogs/krb_auth_dialog.h"
#include "managers/gplink_manager.h"

#include <QDesktopServices>
#include <QLabel>
#include <QModelIndex>

MainWindow::MainWindow(AdInterface &ad, Krb5Client &krb5_client_arg, QWidget *parent)
: QMainWindow(parent), krb5_client(&krb5_client_arg) {
    ui = new Ui::MainWindow();
    ui->setupUi(this);

    country_combo_load_data();

    init_globals();

    setup_status_bar(ad);

    setup_themes();

    setup_languages();

    restore_main_window_state();

    setup_main_window_actions();

    setup_simple_settings();

    setup_authentication_dialog();

    setup_console_actions();

    if (ad.is_connected()) {
        init_on_connect(ad);
        show_changelog_on_update();
    }
    else {
        if (!krb5_client->current_principal().isEmpty()) {
            krb5_client->logout(false);
        }
        g_status->log_messages(ad.messages());
        login_label->setText(tr("Authentication required"));
        auth_dialog->open();
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

    krb5_client->logout(!creds_is_saved(krb5_client->current_principal()));

    QMainWindow::closeEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    resize_status_message();
    QMainWindow::resizeEvent(event);
}

void MainWindow::on_log_searches_changed() {
    const bool enabled = ui->action_log_searches->isChecked();
    AdInterface::set_log_searches(enabled);
}

void MainWindow::on_show_login_changed() {
    const bool enabled = ui->action_show_login->isChecked();
    login_label->setVisible(enabled);
}

void MainWindow::open_manual() {
    const QUrl manual_url = QUrl("https://www.altlinux.org/%D0%93%D1%80%D1%83%D0%BF%D0%BF%D0%BE%D0%B2%D1%8B%D0%B5_%D0%BF%D0%BE%D0%BB%D0%B8%D1%82%D0%B8%D0%BA%D0%B8/ADMC");
    QDesktopServices::openUrl(manual_url);
}

void MainWindow::open_connection_options() {
    auto dialog = new ConnectionOptionsDialog(this);
    dialog->open();
    connect(dialog, &ConnectionOptionsDialog::host_changed,
            [this](const QString &host) {
        show_busy_indicator();
        reload_console_tree();
        hide_busy_indicator();
        g_status->add_message(tr("Connected to host ") + host, StatusType_Success);
    });
}

void MainWindow::open_changelog() {
    auto changelog_dialog = new ChangelogDialog(this);
    changelog_dialog->open();
}

void MainWindow::open_about() {
    auto about_dialog = new AboutDialog(this);
    about_dialog->open();
}

void MainWindow::edit_fsmo_roles() {
    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    auto dialog = new FSMODialog(ad, this);
    dialog->open();
    connect(dialog, &FSMODialog::master_changed, ui->console, &ConsoleWidget::fsmo_master_changed);
}

void MainWindow::reload_console_tree() {
    ui->console->refresh_scope(ui->console->domain_info_index());
}

void MainWindow::setup_themes() {
    const QStringList theme_list = g_icon_manager->available_themes();
    const QLocale current_locale = settings_get_variant(SETTING_locale).toLocale();
    auto theme_action_group = new QActionGroup(this);
    for (const QString &theme : theme_list) {
        const QString localized_name = g_icon_manager->localized_theme_name(current_locale, theme);
        const auto action = new QAction(localized_name, theme_action_group);

        action->setCheckable(true);
        theme_action_group->addAction(action);

        bool is_checked;
        const QString current_theme = settings_get_variant(SETTING_current_icon_theme).toString();
        current_theme == theme ? is_checked = true : is_checked = false;

        action->setChecked(is_checked);
        ui->menu_theme->addAction(action);

        connect(
            action, &QAction::triggered,
            this,
            [this, theme](bool checked) {
                if (checked == false)
                    return;

                g_icon_manager->set_theme(theme);

                update();

                // TODO: Replace total tree updating from base
                // by tree item icons updating corresponding its item type.
                // Add corresponding function like update_console_tree_icons(ConsoleWidget*)
                // to icon manager
                reload_console_tree();
            });
    }
}

void MainWindow::setup_languages() {
    const QList<QLocale::Language> language_list = {
        QLocale::English,
        QLocale::Russian,
    };
    auto language_group = new QActionGroup(this);
    for (const QLocale::Language &language : language_list) {
        const QLocale locale(language);

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

            return (current_locale.language() == locale.language());
        }();
        action->setChecked(is_checked);

        ui->menu_language->addAction(action);

        connect(
            action, &QAction::triggered,
            this,
            [this, language](bool checked) {
                if (checked) {
                    settings_set_variant(SETTING_locale, QLocale(language));

                    message_box_information(this, tr("Info"), tr("Restart the app to switch to the selected language."));
                }
            });
    }
}

void MainWindow::setup_simple_settings() {
    const QHash<QString, QAction *> bool_action_map = {
        {SETTING_confirm_actions, ui->action_confirm_actions},
        {SETTING_last_name_before_first_name, ui->action_last_name_order},
        {SETTING_log_searches, ui->action_log_searches},
        {SETTING_timestamp_log, ui->action_timestamps},
        {SETTING_show_login, ui->action_show_login},
        {SETTING_show_non_containers_in_console_tree, ui->action_show_noncontainers},
        {SETTING_advanced_features, ui->action_advanced_features},
        {SETTING_load_optional_attribute_values, ui->action_load_optional_values},
        {SETTING_show_middle_name_when_creating, ui->action_show_middle_name},
        {SETTING_show_login_window_on_startup, ui->action_show_login_window_on_startup},
    };

    const QList<QString> simple_setting_list = {
        SETTING_confirm_actions,
        SETTING_last_name_before_first_name,
        SETTING_log_searches,
        SETTING_timestamp_log,
        SETTING_show_login,
        SETTING_load_optional_attribute_values,
        SETTING_show_middle_name_when_creating,
        SETTING_show_login_window_on_startup
    };

    for (const QString &setting : bool_action_map.keys()) {
        QAction *action = bool_action_map[setting];

        const bool setting_enabled = settings_get_variant(setting).toBool();
        action->setChecked(setting_enabled);
    }

    // Connect setting actions so that they update setting
    // values
    for (const QString &setting : simple_setting_list) {
        QAction *action = bool_action_map[setting];

        connect(
            action, &QAction::toggled,
            this,
            [setting](bool checked) {
                settings_set_variant(setting, checked);
            });
    }

    // NOTE: Call these slots now to load initial state
    connect(
        ui->action_log_searches, &QAction::triggered,
        this, &MainWindow::on_log_searches_changed);
    on_log_searches_changed();
    connect(
        ui->action_show_login, &QAction::triggered,
        this, &MainWindow::on_show_login_changed);
    on_show_login_changed();
}

void MainWindow::setup_complex_settings(ObjectImpl *obj_impl) {
    const QHash<QString, QAction*> complex_settings_map = {
        {SETTING_show_non_containers_in_console_tree, ui->action_show_noncontainers},
        {SETTING_advanced_features, ui->action_advanced_features}
    };

    for (const QString &setting : complex_settings_map.keys()) {
        QAction *action = complex_settings_map[setting];

        // NOTE: For complex settings, we need to refresh object
        // tree after setting changes. Because call order of
        // slots is undefined we can't just make multiple slots,
        // so need to use a custom slot.
        if (obj_impl) {
            action->setDisabled(false);
            connect(
                action, &QAction::toggled,
                this,
                [setting, obj_impl](bool checked) {
                    settings_set_variant(setting, checked);

                    obj_impl->refresh_tree();
                });
        }
        else {
            action->setDisabled(true);
        }
    }
}

void MainWindow::init_globals() {
    g_status->init(ui->statusbar, ui->message_log_edit);
    const QMap<QString, QAction*> category_action_map = {
        {OBJECT_CATEGORY_OU, ui->action_create_ou},
        {OBJECT_CATEGORY_PERSON, ui->action_create_user},
        {OBJECT_CATEGORY_GROUP, ui->action_create_group},
        {ADMC_CATEGORY_GO_PREVIOUS_ACTION, ui->action_navigate_back},
        {ADMC_CATEGORY_GO_NEXT_ACTION, ui->action_navigate_forward},
        {ADMC_CATEGORY_MANUAL_ACTION, ui->action_manual},
        {ADMC_CATEGORY_GO_UP_ACTION, ui->action_navigate_up},
        {ADMC_CATEGORY_REFRESH_ACTION, ui->action_refresh}
    };

    g_icon_manager->init(category_action_map);
}

void MainWindow::setup_console_actions() {
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
}

void MainWindow::setup_main_window_actions() {
    // NOTE: toolbar and message log(dock widget) have built
    // in toggle actions, but there's no way to add them
    // through designer so add them here.
    ui->menu_view->insertAction(ui->action_toggle_message_log, ui->message_log->toggleViewAction());
    ui->menu_view->insertAction(ui->action_toggle_toolbar, ui->toolbar->toggleViewAction());
    ui->menu_view->removeAction(ui->action_toggle_message_log);
    ui->menu_view->removeAction(ui->action_toggle_toolbar);

    //
    // Connect
    //
    connect(
        ui->action_connection_options, &QAction::triggered,
        this, &MainWindow::open_connection_options);
    connect(
        ui->action_edit_fsmo_roles, &QAction::triggered,
        this, &MainWindow::edit_fsmo_roles);
    connect(
        ui->action_quit, &QAction::triggered,
        this, &MainWindow::close);
    connect(
        ui->action_manual, &QAction::triggered,
        this, &MainWindow::open_manual);
    connect(
        ui->action_changelog, &QAction::triggered,
        this, &MainWindow::open_changelog);
    connect(
        ui->action_about, &QAction::triggered,
        this, &MainWindow::open_about);
    connect(
        ui->action_logout, &QAction::triggered,
        this, &MainWindow::on_logout);
}

void MainWindow::restore_console_widget_state() {
    // NOTE: must restore state after everything is setup
    const QVariant console_widget_state = settings_get_variant(SETTING_console_widget_state);
    ui->console->restore_state(console_widget_state);
}

void MainWindow::restore_main_window_state() {
    const bool restored_geometry = settings_restore_geometry(SETTING_main_window_geometry, this);
    if (!restored_geometry) {
        resize(1024, 768);
        center_widget(this);
    }

    const QByteArray state = settings_get_variant(SETTING_main_window_state).toByteArray();
    if (!state.isEmpty()) {
        restoreState(state);
    } else {
        // Hide message log by default
        ui->message_log->hide();
    }
}

void MainWindow::show_changelog_on_update() {
    const bool first_time_opening_this_version = []() {
        const QString last_version = settings_get_variant(SETTING_last_opened_version).toString();

        return (last_version != ADMC_VERSION);
    }();
    if (first_time_opening_this_version) {
        settings_set_variant(SETTING_last_opened_version, ADMC_VERSION);
        open_changelog();
    }
}

void MainWindow::setup_status_bar(const AdInterface &ad) {
    login_label = new QLabel();
    login_label->setText(ad.client_user());
    ui->statusbar->addPermanentWidget(login_label);

    ui->statusbar->addAction(ui->action_show_login);
    ui->statusbar->addAction(ui->action_change_user);
    ui->statusbar->addAction(ui->action_logout);

    connect(ui->statusbar, &QStatusBar::messageChanged, [this](const QString &) {
       resize_status_message();
    });
}

void MainWindow::init_on_connect(AdInterface &ad) {
    load_g_adconfig(ad);

    auto domain_info_impl = new DomainInfoImpl(ui->console);
    ui->console->register_impl(ItemType_DomainInfo, domain_info_impl);

    auto object_impl = new ObjectImpl(ui->console);
    ui->console->register_impl(ItemType_Object, object_impl);

    auto policy_root_impl = new PolicyRootImpl(ui->console);
    ui->console->register_impl(ItemType_PolicyRoot, policy_root_impl);

    auto all_policies_folder_impl = new AllPoliciesFolderImpl(ui->console);
    ui->console->register_impl(ItemType_AllPoliciesFolder, all_policies_folder_impl);

    auto policy_ou_impl = new PolicyOUImpl(ui->console);
    ui->console->register_impl(ItemType_PolicyOU, policy_ou_impl);

    auto policy_impl = new PolicyImpl(ui->console);
    ui->console->register_impl(ItemType_Policy, policy_impl);

    auto query_item_impl = new QueryItemImpl(ui->console);
    ui->console->register_impl(ItemType_QueryItem, query_item_impl);

    auto query_folder_impl = new QueryFolderImpl(ui->console);
    ui->console->register_impl(ItemType_QueryFolder, query_folder_impl);

    query_item_impl->set_query_folder_impl(query_folder_impl);

    object_impl->set_toolbar_actions(ui->action_create_user, ui->action_create_group, ui->action_create_ou);

    setup_complex_settings(object_impl);

    connect(
        ui->action_filter_objects, &QAction::triggered,
                object_impl, &ObjectImpl::open_console_filter_dialog);

    domain_info_impl->load_domain_info_item(ad);
    ui->console->set_current_scope(ui->console->domain_info_index());
    console_object_tree_init(ui->console, ad);
    console_policy_tree_init(ui->console);
    console_query_tree_init(ui->console);
    console_tree_add_password_settings(ui->console, ad);
    g_gplink_manager->update();
    ui->console->expand_item(ui->console->domain_info_index());

    restore_console_widget_state();

    // NOTE: "Action" menu actions need to be filled by the
    // console
    ui->console->setup_menubar_action_menu(ui->menu_action);

    // Set current scope to object head to load it
    const QModelIndex object_tree_root = get_object_tree_root(ui->console);
    if (object_tree_root.isValid()) {
        ui->console->set_current_scope(object_tree_root);
    }

    // Display any errors that happened when loading the
    // console
    g_status->display_ad_messages(ad, this);

    if (!current_dc_is_master_for_role(ad, FSMORole_PDCEmulation)) {
        g_status->add_message(tr("You are connected to DC without PDC-Emulator role"), StatusType_Success);
    }
    else {
        ui->statusbar->clearMessage();
    }

    login_label->setText(ad.client_user());

    inited = true;
}

void MainWindow::setup_authentication_dialog() {
    auth_dialog = new KrbAuthDialog(this, krb5_client);
    connect(auth_dialog, &KrbAuthDialog::authenticated, this, [this]() {
        AdInterface ad;
        if (inited) {
            show_busy_indicator();
            load_g_adconfig(ad);
            ui->console->hide_scope_and_results(false);
            disable_actions_on_logout(false);
            reload_console_tree();
            login_label->setText(ad.client_user());
            hide_busy_indicator();
        }
        else {
            show_busy_indicator();
            init_on_connect(ad);
            hide_busy_indicator();
        }
        g_status->add_message(tr("Logged in successfully"), StatusType_Success);

        auth_dialog->hide();
    });

    connect(ui->action_change_user, &QAction::triggered, this, &MainWindow::on_change_user);
}

void MainWindow::on_change_user() {
    auth_dialog->show();
    auth_dialog->adjustSize();
}

void MainWindow::on_logout() {
    ui->console->clear_scope_tree();
    ui->console->hide_scope_and_results(true);

    disable_actions_on_logout(true);

    login_label->setText(tr("Authentication required"));

    bool delete_creds = !creds_is_saved(krb5_client->current_principal());
    auth_dialog->logout(delete_creds);
    krb5_client->logout(delete_creds);

    ui->console->refresh_scope(ui->console->domain_info_index());
    auth_dialog->show();
}

void MainWindow::disable_actions_on_logout(bool disable) {
    const QList<QAction*> actions_to_disable = {
        ui->action_navigate_back,
        ui->action_navigate_forward,
        ui->action_navigate_up,
        ui->action_create_user,
        ui->action_create_ou,
        ui->action_create_group,
        ui->action_refresh,
        ui->action_connection_options,
        ui->action_edit_fsmo_roles
    };

    for (auto action : actions_to_disable) {
        action->setDisabled(disable);
    }

    ui->menu_action->setDisabled(disable);
    ui->menu_theme->setDisabled(disable);
}

void MainWindow::resize_status_message() {
    const QString elided_message = ui->statusbar->fontMetrics().elidedText(ui->statusbar->currentMessage(),
                                                                           Qt::ElideRight,
                                                                           width()/2);
    ui->statusbar->setToolTip(ui->statusbar->currentMessage());
    ui->statusbar->blockSignals(true);
    ui->statusbar->showMessage(elided_message);
    ui->statusbar->blockSignals(false);
}
