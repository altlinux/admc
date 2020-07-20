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

DnColumnProxy::DnColumnProxy(int dn_column_arg, QObject *parent)
: QSortFilterProxyModel(parent)
{
    dn_column = dn_column_arg;

    const BoolSetting *dn_column_setting = Settings::instance()->bool_setting(BoolSettingType_DnColumn);
    connect(
        dn_column_setting, &BoolSetting::changed,
        this, &DnColumnProxy::on_toggle_show_dn_column);
    on_toggle_show_dn_column();
}

void DnColumnProxy::on_toggle_show_dn_column() {
    show_dn_column = Settings::instance()->get_bool(BoolSettingType_DnColumn);

    invalidateFilter();
}

bool DnColumnProxy::filterAcceptsColumn(int source_column, const QModelIndex &parent) const {
    if (!show_dn_column && source_column == dn_column) {
        return false;
    } else {
        return true;
    }
}
