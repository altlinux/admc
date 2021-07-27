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
DEFINE_SETTING(SETTING_find_results_state);
DEFINE_SETTING(SETTING_filter_dialog_state);

// Widget geometry
DEFINE_SETTING(SETTING_main_window_geometry);
DEFINE_SETTING(SETTING_properties_dialog_geometry);
DEFINE_SETTING(SETTING_filter_dialog_geometry);
DEFINE_SETTING(SETTING_find_object_dialog_geometry);
DEFINE_SETTING(SETTING_select_object_dialog_geometry);
DEFINE_SETTING(SETTING_select_container_dialog_geometry);
DEFINE_SETTING(SETTING_object_multi_dialog_geometry);

// Header state
DEFINE_SETTING(SETTING_results_header);
DEFINE_SETTING(SETTING_find_results_header);
DEFINE_SETTING(SETTING_attributes_tab_header_state);
DEFINE_SETTING(SETTING_select_object_header_state);
DEFINE_SETTING(SETTING_membership_tab_header_state);
DEFINE_SETTING(SETTING_organization_tab_header_state);
DEFINE_SETTING(SETTING_gpo_links_tab_header_state);
DEFINE_SETTING(SETTING_group_policy_tab_header_state);
DEFINE_SETTING(SETTING_security_tab_header_state);

// Bool
DEFINE_SETTING(SETTING_advanced_features);
DEFINE_SETTING(SETTING_confirm_actions);
DEFINE_SETTING(SETTING_dev_mode);
DEFINE_SETTING(SETTING_show_non_containers_in_console_tree);
DEFINE_SETTING(SETTING_last_name_before_first_name);
DEFINE_SETTING(SETTING_show_console_tree);
DEFINE_SETTING(SETTING_show_results_header);
DEFINE_SETTING(SETTING_log_searches);
DEFINE_SETTING(SETTING_timestamp_log);
DEFINE_SETTING(SETTING_sasl_canon);

// Other
DEFINE_SETTING(SETTING_dc);
DEFINE_SETTING(SETTING_locale);
DEFINE_SETTING(SETTING_query_folders);
DEFINE_SETTING(SETTING_query_items);
DEFINE_SETTING(SETTING_port);
DEFINE_SETTING(SETTING_cert_strategy);

QVariant settings_get_variant(const QString setting, const QVariant &default_value = QVariant());
void settings_set_variant(const QString setting, const QVariant &value);

// NOTE: returns default value if it's defined in
// settings.cpp
bool settings_get_bool(const QString setting);
void settings_set_bool(const QString setting, const bool value);

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

/** 
 * Make a checkable QAction that is connected to a bool
 * setting. Action state will be initialized to the current
 * setting value. The "connect" version of the f-n also
 * connects the action for you so that when toggling the
 * action modifies the setting.
 */
QAction *settings_make_action(const QString setting, const QString &text, QObject *parent);
QAction *settings_make_and_connect_action(const QString setting, const QString &text, QObject *parent);

#endif /* SETTINGS_H */
