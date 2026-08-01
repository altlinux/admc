/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2026 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#include "core/settings.h"

#include "config.h"
#include "connection_options_dialog.h"

#include <QAction>
#include <QDialog>
#include <QHeaderView>
#include <QLocale>
#include <QSettings>

const QHash<QString, QVariant> setting_default_map = {
    {SETTING_advanced_features, false},
    {SETTING_confirm_actions, true},
    {SETTING_show_non_containers_in_console_tree, false},
    {SETTING_last_name_before_first_name,
        []() {
            const bool locale_is_russian = (QLocale::system().language() == QLocale::Russian);
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
};

void settings_setup_dialog_geometry(const QString setting, QDialog *dialog) {
    settings_restore_geometry(setting, dialog);

    QObject::connect(
        dialog, &QDialog::finished,
        dialog,
        [setting, dialog]() {
            const QByteArray geometry = dialog->saveGeometry();
            settings_set_variant(setting, geometry);
        });
}

bool settings_restore_geometry(const QString setting, QWidget *widget) {
    const QByteArray geometry = settings_get_byte_array(setting);
    if (!geometry.isEmpty()) {
        widget->restoreGeometry(geometry);

        return true;
    } else {
        return false;
    }
}

bool settings_restore_main_window_geometry(QWidget* widget) {
    return settings_restore_geometry(SETTING_main_window_geometry, widget);
}

void settings_save_header_state(const QString setting, QHeaderView *header) {
    const QByteArray state = header->saveState();
    settings_set_variant(setting, state);
}

bool settings_restore_header_state(const QString setting, QHeaderView *header) {
    const QByteArray state = settings_get_variant(setting).toByteArray();
    if (!state.isEmpty()) {
        header->restoreState(state);

        return true;
    } else {
        return false;
    }
}

QVariant settings_get_variant(const QString setting) {
    QSettings settings;

    const QVariant default_value = setting_default_map.value(setting, QVariant());

    const QVariant value = settings.value(setting, default_value);

    return value;
}

void settings_set_variant(const QString setting, const QVariant &value) {
    QSettings settings;

    settings.setValue(setting, value);
}

void settings_save_main_window_geometry(const QByteArray &geometry) {
    settings_set_variant(SETTING_main_window_geometry, geometry);
}

void settings_save_main_window_state(const QByteArray &state) {
    settings_set_variant(SETTING_main_window_state, state);
}

QByteArray settings_load_main_window_state() {
    return settings_get_byte_array(SETTING_main_window_state);
}

void settings_save_console_state(const QVariant &state) {
    settings_set_variant(SETTING_console_widget_state, state);
}

QVariant settings_load_console_state() {
    return settings_get_variant(SETTING_console_widget_state);
}

QString settings_load_last_opened_version() {
    return settings_get_string(SETTING_last_opened_version);
}

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
    const QString last_version = settings_load_last_opened_version();
    return (last_version != ADMC_VERSION);
}

/**
 * Get a list of remembered principals.
 */
QStringList settings_get_remembered_principals() {
    QVariant principals = settings_get_variant(SETTING_remembered_principals);
    return principals.isNull() ? QStringList() : principals.toStringList();
}

bool settings_are_creds_saved(const QString &username) {
    QStringList principals = settings_get_remembered_principals();
    return (! username.isEmpty()) && principals.contains(username);
}

const QLocale settings_get_current_locale() {
    return settings_get_variant(SETTING_locale).toLocale();
}

bool settings_get_bool(const QString &setting) {
    return settings_get_variant(setting).toBool();
}

QString settings_get_string(const QString &setting) {
    return settings_get_variant(setting).toString();
}

int settings_get_int(const QString &setting) {
    return settings_get_variant(setting).toInt();
}

QByteArray settings_get_byte_array(const QString &setting) {
    return settings_get_variant(setting).toByteArray();
}

QHash<QString, QVariant> settings_get_hash(const QString &setting) {
    return settings_get_variant(setting).toHash();
}
