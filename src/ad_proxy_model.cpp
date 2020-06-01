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

#include "ad_proxy_model.h"
#include "ad_model.h"
#include "ad_interface.h"
#include "actions.h"

AdProxyModel::AdProxyModel(AdModel *model, QObject *parent)
: QSortFilterProxyModel(parent)
{
    setSourceModel(model);

    connect(
        &action_advanced_view, &QAction::triggered,
        this, &AdProxyModel::on_advanced_view_toggled);
}

void AdProxyModel::on_advanced_view_toggled(bool) {
    invalidateFilter();
}

bool AdProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    const QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    // Hide advanced view only entries if advanced view is OFF
    const bool advanced_view_only = index.data(AdModel::Roles::AdvancedViewOnly).toBool();
    const bool advanced_view_is_on = action_advanced_view.isChecked();
    if (advanced_view_only && !advanced_view_is_on) {
        return false;
    }

    if (only_show_containers) {
        // Hide non-containers
        const bool is_container = index.data(AdModel::Roles::IsContainer).toBool();
        if (!is_container) {
            return false;
        }
    }

    return true;
}
