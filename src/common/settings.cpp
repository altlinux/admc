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

#include <QAction>
#include <QCheckBox>
#include <QWidget>
#include <QSettings>
#include <QLocale>
#include <QCoreApplication>

bool bool_default_value(const BoolSetting setting);
QString bool_to_string(const BoolSetting setting);
QString variant_to_string(const VariantSetting setting);

Settings *Settings::instance() {
    static Settings settings;
    return &settings;
}

const BoolSettingSignal *Settings::get_bool_signal(const BoolSetting setting) const {
    return &bools[setting];
}

bool Settings::get_bool(const BoolSetting setting) const {
    const QString setting_str = bool_to_string(setting);
    const bool value = qsettings->value(setting_str, false).toBool();

    return value;
}

void Settings::set_bool(const BoolSetting setting, const bool value) {
    const QString name = bool_to_string(setting);
    qsettings->setValue(name, value);
}

QVariant Settings::get_variant(const VariantSetting setting) const {
    const QString name = variant_to_string(setting);
    const QVariant value = qsettings->value(name); 

    return value;
}

void Settings::set_variant(const VariantSetting setting, const QVariant &value) {
    const QString name = variant_to_string(setting);
    qsettings->setValue(name, value);
}

bool Settings::contains_variant(const VariantSetting setting) const {
    const QString name = variant_to_string(setting);
    return qsettings->contains(name);
}

Settings::Settings() {
    qsettings = new QSettings(this);
}

void Settings::connect_action_to_bool_setting(QAction *action, const BoolSetting setting) {
    action->setCheckable(true);

    const QString setting_str = bool_to_string(setting);

    const bool default_value = bool_default_value(setting);
    
    // Init action state to saved value
    const bool saved_value = qsettings->value(setting_str, default_value).toBool();
    action->setChecked(saved_value);

    // Update saved value when action is toggled
    connect(
        action, &QAction::toggled,
        [this, setting, setting_str](bool checked) {
            qsettings->setValue(setting_str, checked);

            emit bools[setting].changed();
        });
}

void Settings::connect_checkbox_to_bool_setting(QCheckBox *check, const BoolSetting setting) {
    const QString setting_str = bool_to_string(setting);

    const bool default_value = bool_default_value(setting);
    
    // Init action state to saved value
    const bool saved_value = qsettings->value(setting_str, default_value).toBool();
    check->setChecked(saved_value);

    connect(
        check, &QCheckBox::stateChanged,
        [this, setting, setting_str, check]() {
            const bool checked = check->isChecked();
            qsettings->setValue(setting_str, checked);

            emit bools[setting].changed();
        });
}

void Settings::restore_geometry(QWidget *widget, const VariantSetting geometry_setting) {
    const QByteArray geometry = get_variant(VariantSetting_MainWindowGeometry).toByteArray();
    if (!geometry.isEmpty()) {
        widget->restoreGeometry(geometry);
    }
}

void Settings::save_geometry(QWidget *widget, const VariantSetting geometry_setting) {
    const QByteArray geometry = widget->saveGeometry();
    set_variant(VariantSetting_MainWindowGeometry, QVariant(geometry));
}

// NOTE: DON'T define "default:" branch, want to be warned and forced to define a default value for all settings
bool bool_default_value(const BoolSetting setting) {
    switch (setting) {
        case BoolSetting_AdvancedView: return false;
        case BoolSetting_ConfirmActions: return true;
        case BoolSetting_DevMode: return false;
        case BoolSetting_DetailsIsDocked: return false;
        case BoolSetting_ShowNonContainersInContainersTree: return false;
        case BoolSetting_LastNameBeforeFirstName: {
            const bool locale_is_russian = (QLocale::system().language() == QLocale::Russian);
            if (locale_is_russian) {
                return true;
            } else {
                return false;
            }
        }
        case BoolSetting_QuickFind: return false;
        case BoolSetting_ShowStatusLog: return false;
        case BoolSetting_ShowContainers: return true;
        case BoolSetting_ShowContentsHeader: return true;

        case BoolSetting_COUNT: {}
    }

    return false;
}

#define CASE_ENUM_TO_STRING(ENUM) case ENUM: return #ENUM

// Convert enum to string literal via macro
// BoolSetting_Foo => "BoolSetting_Foo"
QString bool_to_string(const BoolSetting setting) {
    switch (setting) {
        CASE_ENUM_TO_STRING(BoolSetting_AdvancedView);
        CASE_ENUM_TO_STRING(BoolSetting_ConfirmActions);
        CASE_ENUM_TO_STRING(BoolSetting_DevMode);
        CASE_ENUM_TO_STRING(BoolSetting_DetailsIsDocked);
        CASE_ENUM_TO_STRING(BoolSetting_ShowNonContainersInContainersTree);
        CASE_ENUM_TO_STRING(BoolSetting_LastNameBeforeFirstName);
        CASE_ENUM_TO_STRING(BoolSetting_QuickFind);
        CASE_ENUM_TO_STRING(BoolSetting_ShowStatusLog);
        CASE_ENUM_TO_STRING(BoolSetting_ShowContainers);
        CASE_ENUM_TO_STRING(BoolSetting_ShowContentsHeader);
        CASE_ENUM_TO_STRING(BoolSetting_COUNT);
    }
    return "";
}

QString variant_to_string(const VariantSetting setting) {
    switch (setting) {
        CASE_ENUM_TO_STRING(VariantSetting_Domain);
        CASE_ENUM_TO_STRING(VariantSetting_Site);
        CASE_ENUM_TO_STRING(VariantSetting_MainWindowGeometry);
        CASE_ENUM_TO_STRING(VariantSetting_MainWindowGeometry);
        CASE_ENUM_TO_STRING(VariantSetting_Locale);
        CASE_ENUM_TO_STRING(VariantSetting_COUNT);
    }
    return "";
}

Settings *SETTINGS() {
    return Settings::instance();
}