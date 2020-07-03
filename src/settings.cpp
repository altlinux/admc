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

QString checkable_text(SettingsCheckable checkable) {
    switch (checkable) {
        case SettingsCheckable_AdvancedView: return "Advanced View";
        case SettingsCheckable_DnColumn: return "Show DN column";
        case SettingsCheckable_DetailsFromContainers: return "Open attributes on left click in Containers window";
        case SettingsCheckable_DetailsFromContents: return "Open attributes on left click in Contents window";
        case SettingsCheckable_ConfirmActions: return "Confirm actions";
        case SettingsCheckable_ShowStatusLog: return "Show status log";
        case SettingsCheckable_COUNT: return "COUNT";
    }
    return "";
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
    
    for (int i = 0; i < SettingsCheckable_COUNT; i++) {
        const SettingsCheckable checkable = (SettingsCheckable) i;
        const QString text = checkable_text(checkable);

        QAction *action = new QAction(text);
        action->setCheckable(true);

        bool checked = settings.value(text, false).toBool();
        action->setChecked(checked);

        checkables[i] = action;
    }

    // Save settings before the app quits
    connect(
        qApp, &QCoreApplication::aboutToQuit,
        this, &Settings::save_settings);
}

void Settings::emit_toggle_signals() const {
    for (auto c : checkables) {
        const bool checked = c->isChecked();
        emit c->toggled(checked);
    }
}

const QAction *checkable(SettingsCheckable c) const {
    return checkables[c];
}

void Settings::save_settings() {
    const QString settings_file_path = get_settings_file_path();
    QSettings settings(settings_file_path, QSettings::NativeFormat);

    for (auto c : checkables) {
        const bool checked = c->isChecked();
        const QString text = c->text();
        settings.setValue(text, checked);
    }
}

const Settings *SETTINGS() {
    ADMC *app = qobject_cast<ADMC *>(qApp);
    const Settings *settings = app->settings();
    return settings;
}
