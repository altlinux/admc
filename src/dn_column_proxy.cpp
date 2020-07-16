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

#include "dn_column_proxy.h"
#include "settings.h"

#include <QAction>

DnColumnProxy::DnColumnProxy(int dn_column_arg, QObject *parent)
: QSortFilterProxyModel(parent)
{
    dn_column = dn_column_arg;

    const QAction *dn_column_action = Settings::instance()->checkable(SettingsCheckable_DnColumn);
    connect(
        dn_column_action, &QAction::toggled,
        this, &DnColumnProxy::on_toggle_show_dn_column);
}

void DnColumnProxy::on_toggle_show_dn_column(bool checked) {
    show_dn_column = checked;

    invalidateFilter();
}

bool DnColumnProxy::filterAcceptsColumn(int source_column, const QModelIndex &parent) const {
    if (!show_dn_column && source_column == dn_column) {
        return false;
    } else {
        return true;
    }
}
