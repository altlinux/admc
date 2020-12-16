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

#include "containers_proxy.h"

#include "settings.h"
#include "object_model.h"

ContainersProxy::ContainersProxy(QObject *parent)
: QSortFilterProxyModel(parent) {
    const BoolSettingSignal *show_non_containers_signal = SETTINGS()->get_bool_signal(BoolSetting_ShowNonContainersInContainersTree);
    connect(
        show_non_containers_signal, &BoolSettingSignal::changed,
        this, &ContainersProxy::on_show_non_containers);
    on_show_non_containers();
}

void ContainersProxy::on_show_non_containers() {
    show_non_containers = SETTINGS()->get_bool(BoolSetting_ShowNonContainersInContainersTree);
    invalidateFilter();
}

bool ContainersProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    if (show_non_containers) {
        return true;
    } else {
        const QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
        const bool is_container = index.data(ObjectModel::Roles::IsContainer).toBool();

        return is_container;
    }
}
