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

#ifndef SETTINGS_H
#define SETTINGS_H

/**
 * Provides access to settings via enums rather than plain
 * strings. Settings are saved to file automatically when
 * this object is destructed. Settings of boolean type have
 * BoolSettingSignal objects which emit changed() signal
 * when setting is changed.
 */

#include <QObject>
#include <QSettings>

#include "utils.h"

class QAction;
class QSettings;
class QVariant;
class QWidget;
class QCheckBox;
class QHeaderView;

enum VariantSetting {
    // ADMC
    VariantSetting_Domain,    
    VariantSetting_Site,    
    VariantSetting_Locale,
    VariantSetting_ResultsHeader,
    VariantSetting_FindResultsHeader,
    VariantSetting_AttributesHeader,

    // GPGUI

    // Shared
    VariantSetting_MainWindowGeometry,

    VariantSetting_COUNT,    
};

enum BoolSetting {
    // ADMC
    BoolSetting_AdvancedView,
    BoolSetting_ConfirmActions,
    BoolSetting_DevMode,
    BoolSetting_ShowNonContainersInConsoleTree,
    BoolSetting_LastNameBeforeFirstName,

    BoolSetting_ShowStatusLog,
    BoolSetting_ShowConsoleTree,
    BoolSetting_ShowResultsHeader,

    // GPGUI

    BoolSetting_COUNT,
};

class BoolSettingSignal final : public QObject {
Q_OBJECT
signals:
    void changed();
};

class Settings {

public:
    static Settings *instance();

    QVariant get_variant(const VariantSetting setting) const;
    void set_variant(const VariantSetting setting, const QVariant &value);
    bool contains_variant(const VariantSetting setting) const;

    const BoolSettingSignal *get_bool_signal(const BoolSetting setting) const;
    bool get_bool(const BoolSetting setting) const;
    void set_bool(const BoolSetting setting, const bool value);

    /** 
     * Connect action and bool setting so that toggling
     * the action updates the setting value
     * Action becomes checkable
     */ 
    void connect_action_to_bool_setting(QAction *action, const BoolSetting setting);

    void connect_checkbox_to_bool_setting(QCheckBox *check, const BoolSetting setting);

    void restore_geometry(QWidget *widget, const VariantSetting geometry_setting);
    void save_geometry(QWidget *widget, const VariantSetting geometry_setting);

    void setup_header_state(QHeaderView* header, const VariantSetting setting);

    void connect_toggle_widget(QWidget *widget, const BoolSetting setting);

private:
    QSettings qsettings;
    BoolSettingSignal bools[BoolSetting_COUNT];

    Settings();

    DISABLE_COPY_MOVE(Settings);
};

Settings *SETTINGS();

#endif /* SETTINGS_H */
