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

QString checkable_display_text(SettingsCheckable checkable);
QString checkable_to_string(SettingsCheckable checkable);
QString value_name(SettingsValue value_enum);

Settings *Settings::instance() {
    static Settings settings;
    return &settings;
}

// Call this after widget construction is finished so that widgets
// load checkable state
void Settings::emit_toggle_signals() const {
    for (auto c : checkables) {
        const bool checked = c->isChecked();
        emit c->toggled(checked);
    }
}

QAction *Settings::checkable(SettingsCheckable c) const {
    return checkables[c];
}

QVariant Settings::get_value(SettingsValue value_enum) const {
    const QString name = value_name(value_enum);
    const QVariant value = qsettings->value(name); 

    return value;
}

void Settings::set_value(SettingsValue value_enum, const QVariant &value) {
    const QString name = value_name(value_enum);
    qsettings->setValue(name, value); 
}

Settings::Settings() {
    qsettings = new QSettings(ADMC_ORGANIZATION, ADMC_APPLICATION_NAME, this);

    for (int i = 0; i < SettingsCheckable_COUNT; i++) {
        const SettingsCheckable checkable = (SettingsCheckable) i;
        
        const QString display_text = checkable_display_text(checkable);
        QAction *action = new QAction(display_text);
        action->setCheckable(true);

        const QString checkable_str = checkable_to_string(checkable);
        const bool was_checked = qsettings->value(checkable_str, false).toBool();
        action->setChecked(was_checked);

        checkables[i] = action;

        // Update value in qsettings when action is toggled
        connect(
            action, &QAction::toggled,
            [this, checkable_str] (bool checked) {
                qsettings->setValue(checkable_str, checked);
            }
            );
    }
}

// Display text for checkable that's attached to corresponding action
// Is translated
QString checkable_display_text(SettingsCheckable checkable) {
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

// Convert enum to string literal via macro and return
// UNTRANSLATED value
// SettingsCheckable_Foo => "SettingsCheckable_Foo"
QString checkable_to_string(SettingsCheckable checkable) {
    #define CASE_ENUM_TO_STRING(ENUM) case ENUM: return #ENUM

    switch (checkable) {
        CASE_ENUM_TO_STRING(SettingsCheckable_AdvancedView);
        CASE_ENUM_TO_STRING(SettingsCheckable_DnColumn);
        CASE_ENUM_TO_STRING(SettingsCheckable_DetailsFromContainers);
        CASE_ENUM_TO_STRING(SettingsCheckable_DetailsFromContents);
        CASE_ENUM_TO_STRING(SettingsCheckable_ConfirmActions);
        CASE_ENUM_TO_STRING(SettingsCheckable_ShowStatusLog);
        CASE_ENUM_TO_STRING(SettingsCheckable_AutoLogin);
        CASE_ENUM_TO_STRING(SettingsCheckable_COUNT);
    }
    return "";

    #undef CASE_ENUM_TO_STRING
}

QString value_name(SettingsValue value_enum) {
    switch (value_enum) {
        case SettingsValue_Domain: return "domain";
        case SettingsValue_Site: return "site";
        case SettingsValue_Host: return "host";
        case SettingsValue_MainWindowGeometry: return "main window geometry";
        case SettingsValue_COUNT: return "COUNT";
    }
    return "";
}
