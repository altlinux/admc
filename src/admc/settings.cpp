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

QString bool_to_string(BoolSetting type);
QString variant_to_string(VariantSetting type);

Settings *Settings::instance() {
    static Settings settings;
    return &settings;
}

const BoolSettingSignal *Settings::get_bool_signal(BoolSetting type) const {
    return &bools[type];
}

bool Settings::get_bool(BoolSetting type) const {
    const QString setting_str = bool_to_string(type);
    const bool value = qsettings->value(setting_str, false).toBool();

    return value;
}

QVariant Settings::get_variant(VariantSetting type) const {
    const QString name = variant_to_string(type);
    const QVariant value = qsettings->value(name); 

    return value;
}

void Settings::set_variant(VariantSetting type, const QVariant &value) {
    const QString name = variant_to_string(type);
    qsettings->setValue(name, value);
}

Settings::Settings() {
    qsettings = new QSettings(ADMC_ORGANIZATION, ADMC_APPLICATION_NAME, this);
}

void Settings::connect_action_to_get_bool_signal(QAction *action, BoolSetting type) {
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
// BoolSetting_Foo => "BoolSetting_Foo"
QString bool_to_string(BoolSetting type) {
    switch (type) {
        CASE_ENUM_TO_STRING(BoolSetting_AdvancedView);
        CASE_ENUM_TO_STRING(BoolSetting_DnColumn);
        CASE_ENUM_TO_STRING(BoolSetting_DetailsFromContainers);
        CASE_ENUM_TO_STRING(BoolSetting_DetailsFromContents);
        CASE_ENUM_TO_STRING(BoolSetting_ConfirmActions);
        CASE_ENUM_TO_STRING(BoolSetting_ShowStatusLog);
        CASE_ENUM_TO_STRING(BoolSetting_AutoLogin);
        CASE_ENUM_TO_STRING(BoolSetting_DevMode);
        CASE_ENUM_TO_STRING(BoolSetting_DetailsIsDocked);
        CASE_ENUM_TO_STRING(BoolSetting_COUNT);
    }
    return "";
}

QString variant_to_string(VariantSetting type) {
    switch (type) {
        CASE_ENUM_TO_STRING(VariantSetting_Domain);
        CASE_ENUM_TO_STRING(VariantSetting_Site);
        CASE_ENUM_TO_STRING(VariantSetting_Host);
        CASE_ENUM_TO_STRING(VariantSetting_MainWindowGeometry);
        CASE_ENUM_TO_STRING(VariantSetting_Locale);
        CASE_ENUM_TO_STRING(VariantSetting_COUNT);
    }
    return "";
}
