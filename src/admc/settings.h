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

#ifndef SETTINGS_H
#define SETTINGS_H

/**
 * Utility f-ns for saving and loading settings using
 * QSettings.
 */

#include <QVariant>

class QAction;
class QVariant;
class QWidget;
class QHeaderView;
class QDialog;
class QString;
class QObject;

enum VariantSetting {
    VariantSetting_DC,
    VariantSetting_Locale,
    VariantSetting_ResultsHeader,
    VariantSetting_FindResultsHeader,
    VariantSetting_AttributesTabHeaderState,
    VariantSetting_MainWindowGeometry,
    VariantSetting_MainWindowState,
    VariantSetting_AttributesTabFilterState,
    VariantSetting_QueryFolders,
    VariantSetting_QueryItems,
    VariantSetting_PropertiesDialogGeometry,
    VariantSetting_FilterDialogGeometry,
    VariantSetting_FindObjectDialogGeometry,
    VariantSetting_SelectObjectDialogGeometry,
    VariantSetting_SelectContainerDialogGeometry,
    VariantSetting_ObjectMultiDialogGeometry,
    VariantSetting_ConsoleWidgetState,
    VariantSetting_PolicyResultsState,
    VariantSetting_FindResultsViewState,
    VariantSetting_SelectObjectHeaderState,
    VariantSetting_MembershipTabHeaderState,
    VariantSetting_OrganizationTabHeaderState,
    VariantSetting_GpoLinksTabHeaderState,
    VariantSetting_GroupPolicyTabHeaderState,
    VariantSetting_SecurityTabHeaderState,
    VariantSetting_FilterDialogState,

    VariantSetting_COUNT,
};

enum BoolSetting {
    BoolSetting_AdvancedFeatures,
    BoolSetting_ConfirmActions,
    BoolSetting_DevMode,
    BoolSetting_ShowNonContainersInConsoleTree,
    BoolSetting_LastNameBeforeFirstName,
    BoolSetting_ShowConsoleTree,
    BoolSetting_ShowResultsHeader,
    BoolSetting_LogSearches,
    BoolSetting_TimestampLog,

    BoolSetting_COUNT,
};

QVariant settings_get_variant(const VariantSetting setting, const QVariant &default_value = QVariant());
void settings_set_variant(const VariantSetting setting, const QVariant &value);

// NOTE: returns default value if it's defined in
// settings.cpp
bool settings_get_bool(const BoolSetting setting);
void settings_set_bool(const BoolSetting setting, const bool value);

// Does two things. First it restores previously saved
// geometry, if it exists. Then it connects to dialogs
// finished() signal so that it's geometry is saved when
// dialog is finished.
void settings_setup_dialog_geometry(const VariantSetting setting, QDialog *dialog);

// NOTE: If setting is present, restore is performed,
// otherwise f-n does nothing and returns false. You
// should check for the return and perform default
// sizing in the false case.
bool settings_restore_geometry(const VariantSetting setting, QWidget *widget);

void settings_save_header_state(const VariantSetting setting, QHeaderView *header);
bool settings_restore_header_state(const VariantSetting setting, QHeaderView *header);

/** 
 * Make a checkable QAction that is connected to a bool
 * setting. Action state will be initialized to the current
 * setting value. The "connect" version of the f-n also
 * connects the action for you so that when toggling the
 * action modifies the setting.
 */
QAction *settings_make_action(const BoolSetting setting, const QString &text, QObject *parent);
QAction *settings_make_and_connect_action(const BoolSetting setting, const QString &text, QObject *parent);

#endif /* SETTINGS_H */
