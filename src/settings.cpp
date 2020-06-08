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

Settings::Settings(QObject *parent)
: QObject(parent)
{
    toggle_advanced_view = new QAction("Advanced View");
    toggle_advanced_view->setCheckable(true);
    toggle_advanced_view->setChecked(false);

    toggle_show_dn_column = new QAction("Show DN column");
    toggle_show_dn_column->setCheckable(true);
    toggle_show_dn_column->setChecked(false);
}

const Settings *SETTINGS() {
    ADMC *app = qobject_cast<ADMC *>(qApp);
    const Settings *settings = app->settings();
    return settings;
}
