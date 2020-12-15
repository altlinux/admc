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

#include "advanced_view_proxy.h"

#include "object_model.h"
#include "settings.h"

AdvancedViewProxy::AdvancedViewProxy(QObject *parent)
: QSortFilterProxyModel(parent) {
    const BoolSettingSignal *signal = SETTINGS()->get_bool_signal(BoolSetting_AdvancedView);

    connect(
        signal, &BoolSettingSignal::changed,
        this, &AdvancedViewProxy::on_setting_changed);
    on_setting_changed();
}

void AdvancedViewProxy::on_setting_changed() {
    advanced_view = SETTINGS()->get_bool(BoolSetting_AdvancedView);
    invalidateFilter();
}

bool AdvancedViewProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    if (advanced_view) {
        // When advanced view is ON, accept all objects
        return true;
    } else {
        // When advanced view is OFF, filter out "advanced
        // view only" objects
        const QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
        const bool advanced_view_only = index.data(ObjectModel::Roles::AdvancedViewOnly).toBool();

        if (advanced_view_only) {
            return false;
        } else {
            return true;
        }
    }
}
