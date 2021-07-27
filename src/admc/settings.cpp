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

#include "settings.h"

#include "config.h"

#include <QAction>
#include <QHeaderView>
#include <QLocale>
#include <QDialog>
#include <QSettings>

const QHash<QString, bool> bool_setting_default_map = {
    {SETTING_advanced_features, false},
    {SETTING_confirm_actions, true},
    {SETTING_dev_mode, false},
    {SETTING_show_non_containers_in_console_tree, false},
    {SETTING_last_name_before_first_name, [&]() {
        const bool locale_is_russian = (QLocale::system().language() == QLocale::Russian);
        if (locale_is_russian) {
            return true;
        } else {
            return false;
        }
    }()},
    {SETTING_show_console_tree, true},
    {SETTING_show_results_header, true},
    {SETTING_log_searches, false},
    {SETTING_timestamp_log, true},
    {SETTING_sasl_nocanon, true},
};

bool settings_get_bool(const QString setting) {
    QSettings settings;

    const bool default_value = bool_setting_default_map.value(setting, false);
    const bool value = settings.value(setting, default_value).toBool();

    return value;
}

void settings_set_bool(const QString setting, const bool value) {
    QSettings settings;

    settings.setValue(setting, value);
}

void settings_setup_dialog_geometry(const QString setting, QDialog *dialog) {
    settings_restore_geometry(setting, dialog);

    QDialog::connect(
        dialog, &QDialog::finished,
        [=]() {
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

QVariant settings_get_variant(const QString setting, const QVariant &default_value) {
    QSettings settings;
    const QVariant value = settings.value(setting, default_value);

    return value;
}

void settings_set_variant(const QString setting, const QVariant &value) {
    QSettings settings;

    settings.setValue(setting, value);
}

void settings_connect_action_to_bool_setting(QAction *action, const QString setting) {
    action->setCheckable(true);

    // Init action state to saved value
    const bool saved_value = settings_get_bool(setting);
    action->setChecked(saved_value);

    // Update saved value when action is toggled
    QObject::connect(
        action, &QAction::toggled,
        [setting](bool checked) {
            settings_set_bool(setting, checked);
        });
}

QAction *settings_make_action(const QString setting, const QString &text, QObject *parent) {
    auto action = new QAction(text, parent);
    action->setCheckable(true);

    // Init action state to saved value
    const bool saved_value = settings_get_bool(setting);
    action->setChecked(saved_value);

    return action;
}

QAction *settings_make_and_connect_action(const QString setting, const QString &text, QObject *parent) {
    auto action = settings_make_action(setting, text, parent);

    // Update saved value when action is toggled
    QObject::connect(
        action, &QAction::toggled,
        [setting](bool checked) {
            settings_set_bool(setting, checked);
        });

    return action;
}
