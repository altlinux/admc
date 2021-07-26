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

bool bool_default_value(const BoolSetting setting);
QString bool_to_string(const BoolSetting setting);
QString variant_to_string(const VariantSetting setting);

bool settings_get_bool(const BoolSetting setting) {
    QSettings settings;

    const QString setting_str = bool_to_string(setting);
    const bool default_value = bool_default_value(setting);
    const bool value = settings.value(setting_str, default_value).toBool();

    return value;
}

void settings_set_bool(const BoolSetting setting, const bool value) {
    QSettings settings;
   
    const QString name = bool_to_string(setting);
    settings.setValue(name, value);
}

void settings_setup_dialog_geometry(const VariantSetting setting, QDialog *dialog) {
    settings_restore_geometry(setting, dialog);

    QDialog::connect(
        dialog, &QDialog::finished,
        [=]() {
            const QByteArray geometry = dialog->saveGeometry();
            settings_set_variant(setting, geometry);
        });
}

bool settings_restore_geometry(const VariantSetting setting, QWidget *widget) {
    const QByteArray geometry = settings_get_variant(setting).toByteArray();
    if (!geometry.isEmpty()) {
        widget->restoreGeometry(geometry);
        
        return true;
    } else {
        return false;
    }
}

void settings_save_header_state(const VariantSetting setting, QHeaderView *header) {
    const QByteArray state = header->saveState();
    settings_set_variant(setting, state);
}

bool settings_restore_header_state(const VariantSetting setting, QHeaderView *header) {
    const QByteArray state = settings_get_variant(setting).toByteArray();
    if (!state.isEmpty()) {
        header->restoreState(state);

        return true;
    } else {
        return false;
    }
}

QVariant settings_get_variant(const VariantSetting setting, const QVariant &default_value) {
    QSettings settings;
    const QString name = variant_to_string(setting);
    const QVariant value = settings.value(name, default_value);

    return value;
}

void settings_set_variant(const VariantSetting setting, const QVariant &value) {
    QSettings settings;
    const QString name = variant_to_string(setting);
    settings.setValue(name, value);
}

void settings_connect_action_to_bool_setting(QAction *action, const BoolSetting setting) {
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

QAction *settings_make_action(const BoolSetting setting, const QString &text, QObject *parent) {
    auto action = new QAction(text, parent);
    action->setCheckable(true);

    // Init action state to saved value
    const bool saved_value = settings_get_bool(setting);
    action->setChecked(saved_value);

    return action;
}

QAction *settings_make_and_connect_action(const BoolSetting setting, const QString &text, QObject *parent) {
    auto action = settings_make_action(setting, text, parent);

    // Update saved value when action is toggled
    QObject::connect(
        action, &QAction::toggled,
        [setting](bool checked) {
            settings_set_bool(setting, checked);
        });

    return action;
}


// NOTE: DON'T define "default:" branch, want to be warned and forced to define a default value for all settings
bool bool_default_value(const BoolSetting setting) {
    switch (setting) {
        case BoolSetting_AdvancedFeatures: return false;
        case BoolSetting_ConfirmActions: return true;
        case BoolSetting_DevMode: return false;
        case BoolSetting_ShowNonContainersInConsoleTree: return false;
        case BoolSetting_LastNameBeforeFirstName: {
            const bool locale_is_russian = (QLocale::system().language() == QLocale::Russian);
            if (locale_is_russian) {
                return true;
            } else {
                return false;
            }
        }
        case BoolSetting_ShowConsoleTree: return true;
        case BoolSetting_ShowResultsHeader: return true;
        case BoolSetting_LogSearches: return false;
        case BoolSetting_TimestampLog: return true;

        case BoolSetting_COUNT: break;
    }

    return false;
}

#define CASE_ENUM_TO_STRING(ENUM) \
case ENUM: return #ENUM

// Convert enum to string literal via macro
// BoolSetting_Foo => "BoolSetting_Foo"
QString bool_to_string(const BoolSetting setting) {
    switch (setting) {
        CASE_ENUM_TO_STRING(BoolSetting_AdvancedFeatures);
        CASE_ENUM_TO_STRING(BoolSetting_ConfirmActions);
        CASE_ENUM_TO_STRING(BoolSetting_DevMode);
        CASE_ENUM_TO_STRING(BoolSetting_ShowNonContainersInConsoleTree);
        CASE_ENUM_TO_STRING(BoolSetting_LastNameBeforeFirstName);
        CASE_ENUM_TO_STRING(BoolSetting_ShowConsoleTree);
        CASE_ENUM_TO_STRING(BoolSetting_ShowResultsHeader);
        CASE_ENUM_TO_STRING(BoolSetting_LogSearches);
        CASE_ENUM_TO_STRING(BoolSetting_TimestampLog);
        CASE_ENUM_TO_STRING(BoolSetting_COUNT);
    }
    return "";
}

QString variant_to_string(const VariantSetting setting) {
    switch (setting) {
        CASE_ENUM_TO_STRING(VariantSetting_DC);
        CASE_ENUM_TO_STRING(VariantSetting_MainWindowGeometry);
        CASE_ENUM_TO_STRING(VariantSetting_MainWindowState);
        CASE_ENUM_TO_STRING(VariantSetting_Locale);
        CASE_ENUM_TO_STRING(VariantSetting_ResultsHeader);
        CASE_ENUM_TO_STRING(VariantSetting_FindResultsHeader);
        CASE_ENUM_TO_STRING(VariantSetting_AttributesTabHeaderState);
        CASE_ENUM_TO_STRING(VariantSetting_AttributesTabFilterState);
        CASE_ENUM_TO_STRING(VariantSetting_QueryFolders);
        CASE_ENUM_TO_STRING(VariantSetting_QueryItems);
        CASE_ENUM_TO_STRING(VariantSetting_PropertiesDialogGeometry);
        CASE_ENUM_TO_STRING(VariantSetting_FilterDialogGeometry);
        CASE_ENUM_TO_STRING(VariantSetting_FindObjectDialogGeometry);
        CASE_ENUM_TO_STRING(VariantSetting_SelectObjectDialogGeometry);
        CASE_ENUM_TO_STRING(VariantSetting_SelectContainerDialogGeometry);
        CASE_ENUM_TO_STRING(VariantSetting_ObjectMultiDialogGeometry);
        CASE_ENUM_TO_STRING(VariantSetting_ConsoleWidgetState);
        CASE_ENUM_TO_STRING(VariantSetting_PolicyResultsState);
        CASE_ENUM_TO_STRING(VariantSetting_FindResultsViewState);
        CASE_ENUM_TO_STRING(VariantSetting_SelectObjectHeaderState);
        CASE_ENUM_TO_STRING(VariantSetting_MembershipTabHeaderState);
        CASE_ENUM_TO_STRING(VariantSetting_OrganizationTabHeaderState);
        CASE_ENUM_TO_STRING(VariantSetting_GpoLinksTabHeaderState);
        CASE_ENUM_TO_STRING(VariantSetting_GroupPolicyTabHeaderState);
        CASE_ENUM_TO_STRING(VariantSetting_SecurityTabHeaderState);
        CASE_ENUM_TO_STRING(VariantSetting_FilterDialogState);
        CASE_ENUM_TO_STRING(VariantSetting_COUNT);
    }
    return "";
}
