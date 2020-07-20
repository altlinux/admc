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
 * Provides access to settings via enums rather than plain strings.
 * Settings are saved to file automatically when this object is
 * destructed.
 * Settings of boolean type have BoolSetting objects which emit
 * changed() signal when setting is changed
 */

#include <QObject>

class QSettings;
class QVariant;

enum SettingsValue {
    SettingsValue_Domain,    
    SettingsValue_Site,    
    SettingsValue_Host,    
    SettingsValue_MainWindowGeometry,
    SettingsValue_COUNT,    
};

enum BoolSettingType {
    BoolSettingType_AdvancedView,
    BoolSettingType_DnColumn,
    BoolSettingType_DetailsFromContainers,
    BoolSettingType_DetailsFromContents,
    BoolSettingType_ConfirmActions,
    BoolSettingType_ShowStatusLog,
    BoolSettingType_AutoLogin,
    BoolSettingType_COUNT,
};

class BoolSetting final : public QObject {
Q_OBJECT
signals:
    void changed();
};

class Settings final : public QObject {
Q_OBJECT

public:
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;
    Settings(Settings&&) = delete;
    Settings& operator=(Settings&&) = delete;

    static Settings *instance();

    void emit_toggle_signals() const;

    QVariant get_value(SettingsValue value_enum) const;
    void set_value(SettingsValue value_enum, const QVariant &value);

    const BoolSetting *bool_setting(BoolSettingType type) const;
    bool get_bool(BoolSettingType type) const;

    /** 
     * Makes action checkable
     * Connects action and bool setting so that toggling the action
     * updates setting value as well
     */ 
    void connect_action_to_bool_setting(QAction *action, BoolSettingType type);

private:
    QSettings *qsettings = nullptr;
    BoolSetting bools[BoolSettingType_COUNT];

    Settings();
};

#endif /* SETTINGS_H */
