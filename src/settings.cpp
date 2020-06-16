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

QAction *make_checkable_action(const QSettings &settings, const QString& text) {
    QAction *action = new QAction(text);
    action->setCheckable(true);

    // Load checked state from settings
    bool checked = settings.value(text, false).toBool();
    action->setChecked(checked);

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

    // Save settings before the app quits
    connect(
        qApp, &QCoreApplication::aboutToQuit,
        this, &Settings::save_settings);
}

void Settings::save_settings() {
    const QString settings_file_path = get_settings_file_path();
    QSettings settings(settings_file_path, QSettings::NativeFormat);

    QList<QAction *> checkable_actions = {
        toggle_advanced_view,
        toggle_show_dn_column,
        details_on_containers_click,
        details_on_contents_click,
        confirm_actions,
    };
    for (auto action : checkable_actions) {
        const bool checked = action->isChecked();
        const QString text = action->text();
        settings.setValue(text, checked);
    }
}

const Settings *SETTINGS() {
    ADMC *app = qobject_cast<ADMC *>(qApp);
    const Settings *settings = app->settings();
    return settings;
}
