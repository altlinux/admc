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
#include <QCheckBox>
#include <QHeaderView>
#include <QLocale>
#include <QDialog>

bool bool_default_value(const BoolSetting setting);
QString bool_to_string(const BoolSetting setting);
QString variant_to_string(const VariantSetting setting);

Settings::Settings()
: qsettings(ADMC_ORGANIZATION, ADMC_APPLICATION_NAME) {
}

const BoolSettingSignal *Settings::get_bool_signal(const BoolSetting setting) const {
    return &bools[setting];
}

bool Settings::get_bool(const BoolSetting setting) const {
    const QString setting_str = bool_to_string(setting);
    const bool default_value = bool_default_value(setting);
    const bool value = qsettings.value(setting_str, default_value).toBool();

    return value;
}

void Settings::set_bool(const BoolSetting setting, const bool value) {
    const QString name = bool_to_string(setting);
    qsettings.setValue(name, value);
}

void Settings::save_geometry(const VariantSetting setting, QWidget *widget) {
    const QByteArray geometry = widget->saveGeometry();
    set_variant(setting, geometry);
}

void Settings::setup_dialog_geometry(const VariantSetting setting, QDialog *dialog) {
    restore_geometry(setting, dialog);

    QDialog::connect(
        dialog, &QDialog::finished,
        [=]() {
            save_geometry(setting, dialog);
        });
}

bool Settings::restore_geometry(const VariantSetting setting, QWidget *widget) {
    if (contains_variant(setting)) {
        const QByteArray geometry = get_variant(setting).toByteArray();
        widget->restoreGeometry(geometry);
        
        return true;
    } else {
        return false;
    }
}

void Settings::save_header_state(const VariantSetting setting, QHeaderView *header) {
    const QByteArray state = header->saveState();
    set_variant(setting, state);
}

bool Settings::restore_header_state(const VariantSetting setting, QHeaderView *header) {
    if (contains_variant(setting)) {
        const QByteArray state = get_variant(setting).toByteArray();
        header->restoreState(state);

        return true;
    } else {
        return false;
    }
}

QVariant Settings::get_variant(const VariantSetting setting) const {
    const QString name = variant_to_string(setting);
    const QVariant value = qsettings.value(name);

    return value;
}

void Settings::set_variant(const VariantSetting setting, const QVariant &value) {
    const QString name = variant_to_string(setting);
    qsettings.setValue(name, value);
}

bool Settings::contains_variant(const VariantSetting setting) const {
    const QString name = variant_to_string(setting);
    return qsettings.contains(name);
}

void Settings::connect_action_to_bool_setting(QAction *action, const BoolSetting setting) {
    action->setCheckable(true);

    // Init action state to saved value
    const bool saved_value = get_bool(setting);
    action->setChecked(saved_value);

    // Update saved value when action is toggled
    QObject::connect(
        action, &QAction::toggled,
        [this, setting](bool checked) {
            set_bool(setting, checked);

            emit bools[setting].changed();
        });
}

void Settings::connect_checkbox_to_bool_setting(QCheckBox *check, const BoolSetting setting) {
    // Init action state to saved value
    const bool saved_value = get_bool(setting);
    check->setChecked(saved_value);

    // Update saved value when checkbox is toggled
    QObject::connect(
        check, &QCheckBox::stateChanged,
        [this, setting, check]() {
            const bool checked = check->isChecked();
            set_bool(setting, checked);

            emit bools[setting].changed();
        });
}

void Settings::setup_header_state(QHeaderView *header, const VariantSetting setting) {
    if (contains_variant(setting)) {
        auto header_width = get_variant(setting).toByteArray();
        header->restoreState(header_width);
    } else {
        header->setDefaultSectionSize(200);
    }

    QObject::connect(
        header, &QHeaderView::destroyed,
        [this, header, setting]() {
            set_variant(setting, header->saveState());
        });
}

void Settings::connect_toggle_widget(QWidget *widget, const BoolSetting setting) {
    const BoolSettingSignal *signal = get_bool_signal(setting);

    auto on_changed = [=]() {
        const bool visible = get_bool(setting);
        widget->setVisible(visible);
    };

    QObject::connect(
        signal, &BoolSettingSignal::changed,
        on_changed);
    on_changed();
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

        case BoolSetting_AttributeFilterUnset: return true;
        case BoolSetting_AttributeFilterReadOnly: return true;
        case BoolSetting_AttributeFilterMandatory: return true;
        case BoolSetting_AttributeFilterOptional: return true;
        case BoolSetting_AttributeFilterSystemOnly: return true;
        case BoolSetting_AttributeFilterConstructed: return true;
        case BoolSetting_AttributeFilterBacklink: return true;

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
        CASE_ENUM_TO_STRING(BoolSetting_AttributeFilterUnset);
        CASE_ENUM_TO_STRING(BoolSetting_AttributeFilterReadOnly);
        CASE_ENUM_TO_STRING(BoolSetting_AttributeFilterMandatory);
        CASE_ENUM_TO_STRING(BoolSetting_AttributeFilterOptional);
        CASE_ENUM_TO_STRING(BoolSetting_AttributeFilterSystemOnly);
        CASE_ENUM_TO_STRING(BoolSetting_AttributeFilterConstructed);
        CASE_ENUM_TO_STRING(BoolSetting_AttributeFilterBacklink);
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
        CASE_ENUM_TO_STRING(VariantSetting_AttributesHeader);
        CASE_ENUM_TO_STRING(VariantSetting_AttributesTabFilter);
        CASE_ENUM_TO_STRING(VariantSetting_QueryFolders);
        CASE_ENUM_TO_STRING(VariantSetting_QueryItems);
        CASE_ENUM_TO_STRING(VariantSetting_PropertiesDialogGeometry);
        CASE_ENUM_TO_STRING(VariantSetting_FilterDialogGeometry);
        CASE_ENUM_TO_STRING(VariantSetting_FindObjectDialogGeometry);
        CASE_ENUM_TO_STRING(VariantSetting_SelectObjectDialogGeometry);
        CASE_ENUM_TO_STRING(VariantSetting_SelectContainerDialogGeometry);
        CASE_ENUM_TO_STRING(VariantSetting_ObjectMultiDialogGeometry);
        CASE_ENUM_TO_STRING(VariantSetting_ConsoleWidgetState);
        CASE_ENUM_TO_STRING(VariantSetting_ObjectResultsState);
        CASE_ENUM_TO_STRING(VariantSetting_QueryResultsState);
        CASE_ENUM_TO_STRING(VariantSetting_PolicyResultsState);
        CASE_ENUM_TO_STRING(VariantSetting_PolicyContainerResultsState);
        CASE_ENUM_TO_STRING(VariantSetting_FindResultsHeaderState);
        CASE_ENUM_TO_STRING(VariantSetting_SelectObjectHeaderState);
        CASE_ENUM_TO_STRING(VariantSetting_MembershipTabHeaderState);
        CASE_ENUM_TO_STRING(VariantSetting_OrganizationTabHeaderState);
        CASE_ENUM_TO_STRING(VariantSetting_GpoLinksTabHeaderState);
        CASE_ENUM_TO_STRING(VariantSetting_GroupPolicyTabHeaderState);
        CASE_ENUM_TO_STRING(VariantSetting_SecurityTabHeaderState);
        CASE_ENUM_TO_STRING(VariantSetting_COUNT);
    }
    return "";
}
