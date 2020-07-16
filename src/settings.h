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

#include <QObject>

class QAction;
class QSettings;
class QVariant;

enum SettingsValue {
    SettingsValue_Domain,    
    SettingsValue_Site,    
    SettingsValue_Host,    
    SettingsValue_MainWindowGeometry,
    SettingsValue_COUNT,    
};

enum SettingsCheckable {
    SettingsCheckable_AdvancedView,
    SettingsCheckable_DnColumn,
    SettingsCheckable_DetailsFromContainers,
    SettingsCheckable_DetailsFromContents,
    SettingsCheckable_ConfirmActions,
    SettingsCheckable_ShowStatusLog,
    SettingsCheckable_AutoLogin,
    SettingsCheckable_COUNT,
};

class Settings;

class Settings final : public QObject {
Q_OBJECT

public:
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;
    Settings(Settings&&) = delete;
    Settings& operator=(Settings&&) = delete;

    static Settings *instance();

    void emit_toggle_signals() const;
    QAction *checkable(SettingsCheckable c) const;
    QVariant get_value(SettingsValue value_enum) const;
    void set_value(SettingsValue value_enum, const QVariant &value);

private:
    QSettings *qsettings = nullptr;
    QAction *checkables[SettingsCheckable_COUNT];

    Settings();
};

#endif /* SETTINGS_H */
