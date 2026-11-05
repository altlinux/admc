/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2026 BaseALT Ltd.
 * Copyright (C) 2020 Igor Chudov
 * Copyright (C) 2020-2022 Dmitry Degtyarev
 * Copyright (C) 2025-2026 Semyon Knyazev
 * Copyright (C) 2026 Artyom V. Poptsov
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

#include <QAction>
#include <QDialog>
#include <QHeaderView>
#include <QLocale>
#include <QSettings>

#include "core/config.h"
#include "core/settings.h"
#include "ui/dialog/connection_options.h"

/**
 * Default values hash map for the application settings.
 */
const QHash<QString, QVariant> SETTIN_DEFAULT_MAP = {
    {SETTING_advanced_features, false},
    {SETTING_confirm_actions, true},
    {SETTING_show_non_containers_in_console_tree, false},
    {SETTING_last_name_before_first_name,
        []() {
            const bool locale_is_russian =
                (QLocale::system().language() == QLocale::Russian);
            if (locale_is_russian) {
                return true;
            } else {
                return false;
            }
        }()},
    {SETTING_log_searches, false},
    {SETTING_timestamp_log, true},
    {SETTING_sasl_nocanon, true},
    {SETTING_show_login, true},
    {SETTING_host, QString()},
    {SETTING_object_filter, QString()},
    {SETTING_object_filter_enabled, false},
    {SETTING_cert_strategy, CERT_STRATEGY_NEVER_define},
    {SETTING_object_display_limit, 1000},

    {SETTING_feature_profile_tab, false},
    {SETTING_feature_dev_mode, false},
    {SETTING_feature_current_locale_first, false},
    {SETTING_was_last_exit_logout, false},
};

/**
 * Restore previously saved geometry if it exists for the specified dialog.  Add
 * a handler to the "finished" signal for the dialog so its geometry will be
 * saved automatically when the dialog is finished.
 *
 * @param setting A dialog geometry setting.
 * @param dialog A dialog to load geometry for.
 */
void settings_setup_dialog_geometry(const QString setting, QDialog *dialog) {
    settings_restore_geometry(setting, dialog);

    QObject::connect(
        dialog, &QDialog::finished,
        dialog,
        [setting, dialog]() {
            const QByteArray GEOMETRY = dialog->saveGeometry();
            settings_set_variant(setting, GEOMETRY);
        });
}

/**
 * Restore the geometry for the specified widget from the application settings.
 *
 * @param setting A setting which holds the widget geometry.
 * @param widget A widget which geometry should be restored.
 * @return True if the geometry is restored from the settings, false otherwise.
 */
bool settings_restore_geometry(const QString setting, QWidget *widget) {
    const QByteArray GEOMETRY = settings_get_byte_array(setting);
    if (! GEOMETRY.isEmpty()) {
        widget->restoreGeometry(GEOMETRY);

        return true;
    } else {
        return false;
    }
}

/**
 * Restore the main window geometry.
 *
 * @param widget A main window widget.
 * @return True if the geometry is restored, false otherwise.
 */
bool settings_restore_main_window_geometry(QWidget* widget) {
    return settings_restore_geometry(SETTING_main_window_geometry, widget);
}

/**
 * Save the header view state in the application settings.
 *
 * @param setting A header view setting name.
 * @param header A header for which the state must be saved.
 */
void settings_save_header_state(const QString setting, QHeaderView *header) {
    const QByteArray STATE = header->saveState();
    settings_set_variant(setting, STATE);
}

/**
 * Restore the header view state from the application settings.
 *
 * @param setting A header view setting name.
 * @param header A header view for which the state must be restored.
 * @return True if the state is restored, false otherwise.
 */
bool settings_restore_header_state(const QString setting, QHeaderView *header) {
    const QByteArray STATE = settings_get_byte_array(setting);
    if (! STATE.isEmpty()) {
        header->restoreState(STATE);

        return true;
    } else {
        return false;
    }
}

/**
 * Get a specified setting the application settings.
 *
 * @param settings A setting to get.
 * @return The QVariant value for the specified setting.  When setting value is
 * not set, return the default value for the setting from the default values
 * hash map.
 */
QVariant settings_get_variant(const QString setting) {
    QSettings settings;
    const QVariant DEFAULT_VALUE =
        SETTIN_DEFAULT_MAP.value(setting, QVariant());
    return settings.value(setting, DEFAULT_VALUE);
}

/**
 * Get a specified setting the application settings.
 *
 * @param settings A setting to get.
 * @param value A default value for the setting.
 * @return The QVariant value for the specified setting.  When setting value is
 * not set, return the default value for the setting.
 */
void settings_set_variant(const QString setting, const QVariant &value) {
    QSettings settings;
    settings.setValue(setting, value);
}

/**
 * Save the main window geometry.
 *
 * @param geometry A geometry byte array to store.
 */
void settings_save_main_window_geometry(const QByteArray &geometry) {
    settings_set_variant(SETTING_main_window_geometry, geometry);
}

/**
 * Save the main window state.
 *
 * @param state A state byte array to store.
 */
void settings_save_main_window_state(const QByteArray &state) {
    settings_set_variant(SETTING_main_window_state, state);
}

/**
 * Load the main window state from the application settings.
 *
 * @return A main window state byte array.
 */
QByteArray settings_load_main_window_state() {
    return settings_get_byte_array(SETTING_main_window_state);
}

/**
 * Save the console state in the application settings.
 *
 * @param state A state to save.
 */
void settings_save_console_state(const QVariant &state) {
    settings_set_variant(SETTING_console_widget_state, state);
}

/**
 * Load the console state from the application settings.
 *
 * @return A console state.
 */
QVariant settings_load_console_state() {
    return settings_get_variant(SETTING_console_widget_state);
}

/**
 * Get the last opened application version.
 */
QString settings_load_last_opened_version() {
    return settings_get_string(SETTING_last_opened_version);
}

/**
 * Save the last opened application version.
 */
void settings_save_last_opened_version() {
    settings_set_variant(SETTING_last_opened_version, ADMC_VERSION);
}

/**
 * Check if the current ADMC version matches the version number stored in the
 * settings.
 *
 * @return True if ADMC has been updated (thus the current version does not
 * match the saved one), false otherwise.
 */
bool settings_has_admc_been_updated() {
    const QString LAST_VERSION = settings_load_last_opened_version();
    return (LAST_VERSION != ADMC_VERSION);
}

/**
 * Get a list of remembered principals.
 */
QStringList settings_get_remembered_principals() {
    QVariant principals = settings_get_variant(SETTING_remembered_principals);
    return principals.isNull() ? QStringList() : principals.toStringList();
}

/**
 * Check if credentials are saved for the given username.
 *
 * @param username A username to check.
 * @return True if the credentials are saved, false otherwise.
 */
bool settings_are_creds_saved(const QString &username) {
    QStringList principals = settings_get_remembered_principals();
    return (! username.isEmpty()) && principals.contains(username);
}

/**
 * Get the current application locale from the settings.
 *
 * @return The current application locale.
 */
const QLocale settings_get_current_locale() {
    return settings_get_variant(SETTING_locale).toLocale();
}

/**
 * Get a boolean value for the specified setting.
 *
 * @param setting A setting to get.
 * @return A boolean value for the setting.
 */
bool settings_get_bool(const QString &setting) {
    return settings_get_variant(setting).toBool();
}

/**
 * Get a string value for the specified setting.
 *
 * @param setting A setting to get.
 * @return A string value for the setting.
 */
QString settings_get_string(const QString &setting) {
    return settings_get_variant(setting).toString();
}

/**
 * Get an integer value for the specified setting.
 *
 * @param setting A setting to get.
 * @return An integer value for the setting.
 */
int settings_get_int(const QString &setting) {
    return settings_get_variant(setting).toInt();
}

/**
 * Get an byte array value for the specified setting.
 *
 * @param setting A setting to get.
 * @return A byte array value for the setting.
 */
QByteArray settings_get_byte_array(const QString &setting) {
    return settings_get_variant(setting).toByteArray();
}

/**
 * Get a hash map value for the specified setting.
 *
 * @param setting A setting to get.
 * @return A hash map value for the setting.
 */
QHash<QString, QVariant> settings_get_hash(const QString &setting) {
    return settings_get_variant(setting).toHash();
}
