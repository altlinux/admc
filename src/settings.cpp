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

QString bool_to_string(BoolSettingType type);
QString value_to_string(SettingsValue type);

Settings *Settings::instance() {
    static Settings settings;
    return &settings;
}

// Call this after widget construction is finished so that widgets
// load checkable state
void Settings::emit_toggle_signals() const {
    // for (auto c : checkables) {
    //     const bool checked = c->isChecked();
    //     emit c->toggled(checked);
    // }
}

const BoolSetting *Settings::bool_setting(BoolSettingType type) const {
    return &bools[type];
}

bool Settings::get_bool(BoolSettingType type) const {
    const QString setting_str = bool_to_string(type);
    const bool value = qsettings->value(setting_str, false).toBool();

    return value;
}

QVariant Settings::get_value(SettingsValue value_enum) const {
    const QString name = value_to_string(value_enum);
    const QVariant value = qsettings->value(name); 

    return value;
}

void Settings::set_value(SettingsValue value_enum, const QVariant &value) {
    const QString name = value_to_string(value_enum);
    qsettings->setValue(name, value);
}

Settings::Settings() {
    qsettings = new QSettings(ADMC_ORGANIZATION, ADMC_APPLICATION_NAME, this);
    printf("%s\n", qPrintable(qsettings->fileName()));
}

void Settings::connect_action_to_bool_setting(QAction *action, BoolSettingType type) {
    action->setCheckable(true);

    const QString setting_str = bool_to_string(type);
    
    // Init action state to saved value
    const bool saved_value = qsettings->value(setting_str, false).toBool();
    action->setChecked(saved_value);

    // Update saved value when action is toggled
    connect(
        action, &QAction::toggled,
        [this, type, setting_str](bool checked) {
            qsettings->setValue(setting_str, checked);

            emit bools[type].changed();
        });
}

#define CASE_ENUM_TO_STRING(ENUM) case ENUM: return #ENUM

// Convert enum to string literal via macro
// BoolSettingType_Foo => "BoolSettingType_Foo"
QString bool_to_string(BoolSettingType type) {
    switch (type) {
        CASE_ENUM_TO_STRING(BoolSettingType_AdvancedView);
        CASE_ENUM_TO_STRING(BoolSettingType_DnColumn);
        CASE_ENUM_TO_STRING(BoolSettingType_DetailsFromContainers);
        CASE_ENUM_TO_STRING(BoolSettingType_DetailsFromContents);
        CASE_ENUM_TO_STRING(BoolSettingType_ConfirmActions);
        CASE_ENUM_TO_STRING(BoolSettingType_ShowStatusLog);
        CASE_ENUM_TO_STRING(BoolSettingType_AutoLogin);
        CASE_ENUM_TO_STRING(BoolSettingType_COUNT);
    }
    return "";
}

QString value_to_string(SettingsValue type) {
    switch (type) {
        CASE_ENUM_TO_STRING(SettingsValue_Domain);
        CASE_ENUM_TO_STRING(SettingsValue_Site);
        CASE_ENUM_TO_STRING(SettingsValue_Host);
        CASE_ENUM_TO_STRING(SettingsValue_MainWindowGeometry);
        CASE_ENUM_TO_STRING(SettingsValue_COUNT);
    }
    return "";
}
