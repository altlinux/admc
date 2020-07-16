/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
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
#include <QSettings>
#include <QList>
#include <QStandardPaths>

Settings *Settings::instance() {
    static Settings settings;
    return &settings;
}

QSettings *Settings::qsettings() {
    static QSettings qsettings(ADMC_ORGANIZATION, ADMC_APPLICATION_NAME);
    return &qsettings;
}

QString checkable_text(SettingsCheckable checkable) {
    switch (checkable) {
        case SettingsCheckable_AdvancedView: return "Advanced View";
        case SettingsCheckable_DnColumn: return "Show DN column";
        case SettingsCheckable_DetailsFromContainers: return "Open attributes on left click in Containers window";
        case SettingsCheckable_DetailsFromContents: return "Open attributes on left click in Contents window";
        case SettingsCheckable_ConfirmActions: return "Confirm actions";
        case SettingsCheckable_ShowStatusLog: return "Show status log";
        case SettingsCheckable_AutoLogin: return "Login using saved session at startup";
        case SettingsCheckable_COUNT: return "COUNT";
    }
    return "";
}

QString string_name(SettingString string) {
    switch (string) {
        case SettingString_Domain: return "domain";
        case SettingString_Site: return "site";
        case SettingString_Host: return "host";
        case SettingString_COUNT: return "COUNT";
    }
    return "";
}

void Settings::emit_toggle_signals() const {
    for (auto c : checkables) {
        const bool checked = c->isChecked();
        emit c->toggled(checked);
    }
}

QAction *Settings::checkable(SettingsCheckable c) const {
    return checkables[c];
}

void Settings::set_string(SettingString string, const QString &value) {
    strings[string] = value;
}

QString Settings::get_string(SettingString string) const {
    const QString value = strings[string];

    return value;
}

Settings::Settings() {
    for (int i = 0; i < SettingsCheckable_COUNT; i++) {
        const SettingsCheckable checkable = (SettingsCheckable) i;
        const QString text = checkable_text(checkable);

        QAction *action = new QAction(text);
        action->setCheckable(true);

        bool checked = qsettings()->value(text, false).toBool();
        action->setChecked(checked);

        checkables[i] = action;
    }

    for (int i = 0; i < SettingString_COUNT; i++) {
        const SettingString string = (SettingString) i;
        const QString name = string_name(string);
        const QString value = qsettings()->value(name, "").toString();

        strings[i] = value;
    }
}

void Settings::save_settings() {
    for (auto c : checkables) {
        const bool checked = c->isChecked();
        const QString text = c->text();
        qsettings()->setValue(text, checked);
    }

    for (int i = 0; i < SettingString_COUNT; i++) {
        const SettingString string = (SettingString) i;
        const QString name = string_name(string);
        const QString value = strings[string];

        qsettings()->setValue(name, value);
    }
}
