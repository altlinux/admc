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

QAction *make_checkable_action(const QString& text) {
    QAction *action = new QAction(text);
    action->setCheckable(true);
    action->setChecked(false);

    return action;
}

Settings::Settings(QObject *parent)
: QObject(parent)
{
    toggle_advanced_view = make_checkable_action("Advanced View");
    toggle_show_dn_column = make_checkable_action("Show DN column");
    details_on_containers_click = make_checkable_action("Open attributes on left click in Containers window");
    details_on_contents_click = make_checkable_action("Open attributes on left click in Contents window");
    confirm_actions = make_checkable_action("Confirm actions");
}

const Settings *SETTINGS() {
    ADMC *app = qobject_cast<ADMC *>(qApp);
    const Settings *settings = app->settings();
    return settings;
}
