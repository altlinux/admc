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

#ifndef SETTINGS_H
#define SETTINGS_H

/**
 * Utility f-ns for saving and loading settings using
 * QSettings.
 */

#include <QVariant>

class QAction;
class QVariant;
class QWidget;
class QHeaderView;
class QDialog;
class QObject;

// NOTE: define setting names this way to guarantee that all
// setting names are unique
// const QString SETTING = "SETTING";
#define DEFINE_SETTING(SETTING) \
    const QString SETTING = #SETTING;

// Widget state
DEFINE_SETTING(SETTING_main_window_state);
DEFINE_SETTING(SETTING_attributes_tab_filter_state);
DEFINE_SETTING(SETTING_console_widget_state);
DEFINE_SETTING(SETTING_policy_results_state);
DEFINE_SETTING(SETTING_policy_ou_results_state);
DEFINE_SETTING(SETTING_inheritance_widget_state);
DEFINE_SETTING(SETTING_find_results_state);
DEFINE_SETTING(SETTING_console_filter_dialog_state);
DEFINE_SETTING(SETTING_select_object_advanced_dialog_console_state);
DEFINE_SETTING(SETTING_find_object_dialog_console_state);
DEFINE_SETTING(SETTING_find_policy_dialog_console_state);

// Widget geometry
DEFINE_SETTING(SETTING_main_window_geometry);
DEFINE_SETTING(SETTING_properties_dialog_geometry);
DEFINE_SETTING(SETTING_console_filter_dialog_geometry);
DEFINE_SETTING(SETTING_find_object_dialog_geometry);
DEFINE_SETTING(SETTING_select_object_dialog_geometry);
DEFINE_SETTING(SETTING_select_container_dialog_geometry);
DEFINE_SETTING(SETTING_object_multi_dialog_geometry);
DEFINE_SETTING(SETTING_connection_options_dialog_geometry);
DEFINE_SETTING(SETTING_changelog_dialog_geometry);
DEFINE_SETTING(SETTING_error_log_dialog_geometry);
DEFINE_SETTING(SETTING_select_well_known_trustee_dialog_geometry);
DEFINE_SETTING(SETTING_select_object_match_dialog_geometry);
DEFINE_SETTING(SETTING_edit_query_item_dialog_geometry);
DEFINE_SETTING(SETTING_create_user_dialog_geometry);
DEFINE_SETTING(SETTING_create_group_dialog_geometry);
DEFINE_SETTING(SETTING_create_computer_dialog_geometry);
DEFINE_SETTING(SETTING_create_ou_dialog_geometry);
DEFINE_SETTING(SETTING_rename_user_dialog_geometry);
DEFINE_SETTING(SETTING_rename_group_dialog_geometry);
DEFINE_SETTING(SETTING_rename_other_dialog_geometry);
DEFINE_SETTING(SETTING_rename_policy_dialog_geometry);
DEFINE_SETTING(SETTING_create_query_folder_dialog_geometry);
DEFINE_SETTING(SETTING_create_query_item_dialog_geometry);
DEFINE_SETTING(SETTING_edit_query_folder_dialog_geometry);
DEFINE_SETTING(SETTING_password_dialog_geometry);
DEFINE_SETTING(SETTING_create_policy_dialog_geometry);
DEFINE_SETTING(SETTING_select_object_advanced_dialog_geometry);
DEFINE_SETTING(SETTING_select_policy_dialog_geometry);
DEFINE_SETTING(SETTING_filter_dialog_geometry);
DEFINE_SETTING(SETTING_class_filter_dialog_geometry);
DEFINE_SETTING(SETTING_logon_hours_dialog_geometry);
DEFINE_SETTING(SETTING_logon_computers_dialog_geometry);
DEFINE_SETTING(SETTING_bool_attribute_dialog_geometry);
DEFINE_SETTING(SETTING_datetime_attribute_dialog_geometry);
DEFINE_SETTING(SETTING_list_attribute_dialog_geometry);
DEFINE_SETTING(SETTING_octet_attribute_dialog_geometry);
DEFINE_SETTING(SETTING_string_attribute_dialog_geometry);
DEFINE_SETTING(SETTING_number_attribute_dialog_geometry);
DEFINE_SETTING(SETTING_hex_number_attribute_dialog_geometry);
DEFINE_SETTING(SETTING_fsmo_dialog_geometry);
DEFINE_SETTING(SETTING_create_shared_folder_dialog_geometry);
DEFINE_SETTING(SETTING_create_contact_dialog_geometry);
DEFINE_SETTING(SETTING_find_policy_dialog_geometry);
DEFINE_SETTING(SETTING_time_span_attribute_dialog_geometry);

// Header state
DEFINE_SETTING(SETTING_results_header);
DEFINE_SETTING(SETTING_find_results_header);
DEFINE_SETTING(SETTING_attributes_tab_header_state);
DEFINE_SETTING(SETTING_select_object_header_state);
DEFINE_SETTING(SETTING_membership_tab_header_state);
DEFINE_SETTING(SETTING_organization_tab_header_state);
DEFINE_SETTING(SETTING_common_permissions_header_state);
DEFINE_SETTING(SETTING_extended_permissions_header_state);
DEFINE_SETTING(SETTING_delegation_permissions_header_state);
DEFINE_SETTING(SETTING_creation_deletion_permissions_header_state);
DEFINE_SETTING(SETTING_read_write_permissions_header_state);
DEFINE_SETTING(SETTING_select_object_match_header_state);

// Bool
DEFINE_SETTING(SETTING_advanced_features);
DEFINE_SETTING(SETTING_confirm_actions);
DEFINE_SETTING(SETTING_show_non_containers_in_console_tree);
DEFINE_SETTING(SETTING_last_name_before_first_name);
DEFINE_SETTING(SETTING_log_searches);
DEFINE_SETTING(SETTING_timestamp_log);
DEFINE_SETTING(SETTING_sasl_nocanon);
DEFINE_SETTING(SETTING_show_login);
DEFINE_SETTING(SETTING_show_password);
DEFINE_SETTING(SETTING_domain_is_default);
DEFINE_SETTING(SETTING_load_optional_attribute_values);
DEFINE_SETTING(SETTING_show_middle_name_when_creating);

// Other
DEFINE_SETTING(SETTING_host);
DEFINE_SETTING(SETTING_locale);
DEFINE_SETTING(SETTING_query_folders);
DEFINE_SETTING(SETTING_query_items);
DEFINE_SETTING(SETTING_port);
DEFINE_SETTING(SETTING_cert_strategy);
DEFINE_SETTING(SETTING_last_opened_version);
DEFINE_SETTING(SETTING_object_filter);
DEFINE_SETTING(SETTING_object_filter_enabled);
DEFINE_SETTING(SETTING_object_display_limit);
DEFINE_SETTING(SETTING_custom_domain);
DEFINE_SETTING(SETTING_current_icon_theme);
DEFINE_SETTING(SETTING_custom_icon_themes_path)

// Feature flags
//
// NOTE: this set of settings is not editable anywhere
// within the app. Instead it should be manually edited
// by hand in ADMC.conf. If you want to forcefully
// enable it for a new version, set it manually in
// main.cpp so that it overwrites default value as well
// as any values defined in .conf file.
DEFINE_SETTING(SETTING_feature_logon_computers);
DEFINE_SETTING(SETTING_feature_profile_tab);
DEFINE_SETTING(SETTING_feature_dev_mode);
DEFINE_SETTING(SETTING_feature_current_locale_first);

QVariant settings_get_variant(const QString setting);
void settings_set_variant(const QString setting, const QVariant &value);

// Does two things. First it restores previously saved
// geometry, if it exists. Then it connects to dialogs
// finished() signal so that it's geometry is saved when
// dialog is finished.
void settings_setup_dialog_geometry(const QString setting, QDialog *dialog);

// NOTE: If setting is present, restore is performed,
// otherwise f-n does nothing and returns false. You
// should check for the return and perform default
// sizing in the false case.
bool settings_restore_geometry(const QString setting, QWidget *widget);

void settings_save_header_state(const QString setting, QHeaderView *header);
bool settings_restore_header_state(const QString setting, QHeaderView *header);

#endif /* SETTINGS_H */
