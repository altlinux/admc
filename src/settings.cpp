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
#include "admc.h"

#include <QAction>
#include <QSettings>
#include <QApplication>
#include <QList>

QString string_name(SettingString string) {
    switch (string) {
        case SettingString_Domain: return "domain";
        case SettingString_Site: return "site";
        case SettingString_Host: return "host";
        case SettingString_COUNT: return "COUNT";
    }
    return "";
}

QAction *Settings::make_checkable_action(const QSettings &settings, const QString& text) {
    QAction *action = new QAction(text);
    action->setCheckable(true);

    // Load checked state from settings
    bool checked = settings.value(text, false).toBool();
    action->setChecked(checked);

    checkable_actions.append(action);

    return action;
}

QString get_settings_file_path() {
    // NOTE: save to app dir for now for easier debugging
    QString settings_file_path = QApplication::applicationDirPath() + "/settings.ini";
    return settings_file_path;
}

Settings::Settings(QObject *parent)
: QObject(parent)
{
    const QString settings_file_path = get_settings_file_path();
    const QSettings settings(settings_file_path, QSettings::NativeFormat);
    
    toggle_advanced_view = make_checkable_action(settings, "Advanced View");
    toggle_show_dn_column = make_checkable_action(settings, "Show DN column");
    details_on_containers_click = make_checkable_action(settings, "Open attributes on left click in Containers window");
    details_on_contents_click = make_checkable_action(settings, "Open attributes on left click in Contents window");
    confirm_actions = make_checkable_action(settings, "Confirm actions");
    toggle_show_status_log = make_checkable_action(settings, "Show status log");
    auto_login = make_checkable_action(settings, "Login using saved session at startup");

    // Load strings
    for (int i = 0; i < SettingString_COUNT; i++) {
        const SettingString string = (SettingString) i;
        const QString name = string_name(string);
        const QString value = settings.value(name, "").toString();

        strings[i] = value;
    }

    connect(
        qApp, &QCoreApplication::aboutToQuit,
        this, &Settings::save_settings);
}

void Settings::emit_toggle_signals() const {
    for (auto action : checkable_actions) {
        const bool checked = action->isChecked();
        emit action->toggled(checked);
    }
}

void Settings::set_string(SettingString string, const QString &value) {
    strings[string] = value;
}

QString Settings::get_string(SettingString string) {
    const QString value = strings[string];

    return value;
}

void Settings::save_settings() {
    const QString settings_file_path = get_settings_file_path();
    QSettings settings(settings_file_path, QSettings::NativeFormat);

    for (auto action : checkable_actions) {
        const bool checked = action->isChecked();
        const QString text = action->text();
        settings.setValue(text, checked);
    }

    for (int i = 0; i < SettingString_COUNT; i++) {
        const SettingString string = (SettingString) i;
        const QString name = string_name(string);
        const QString value = strings[string];

        settings.setValue(name, value);
    }
}

Settings *SETTINGS() {
    ADMC *app = qobject_cast<ADMC *>(qApp);
    Settings *settings = app->settings();
    return settings;
}
