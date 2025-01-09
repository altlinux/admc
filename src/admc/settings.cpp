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

#include "settings.h"

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

    {SETTING_feature_logon_computers, false},
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
    const QByteArray geometry = settings_get_variant(setting).toByteArray();
    if (!geometry.isEmpty()) {
        widget->restoreGeometry(geometry);

        return true;
    } else {
        return false;
    }
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
