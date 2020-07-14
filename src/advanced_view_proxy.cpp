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
#include "ad_interface.h"
#include "settings.h"
#include "utils.h"

#include <QAction>

AdvancedViewProxy::AdvancedViewProxy(int dn_column_arg, QObject *parent)
: QSortFilterProxyModel(parent)
{
    dn_column = dn_column_arg;

    const QAction *advanced_view_action = SETTINGS()->checkable(SettingsCheckable_AdvancedView);
    connect(
        advanced_view_action, &QAction::toggled,
        this, &AdvancedViewProxy::on_advanced_view_toggled);
}

void AdvancedViewProxy::on_advanced_view_toggled(bool) {
    const QAction *advanced_view_action = SETTINGS()->checkable(SettingsCheckable_AdvancedView);
    advanced_view_is_on = advanced_view_action->isChecked();

    invalidateFilter();
}

bool AdvancedViewProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {

    const QModelIndex base_index = sourceModel()->index(source_row, 0, source_parent);
    const QString dn = get_dn_from_index(base_index, dn_column);

    // Hide advanced view only entries if advanced view is OFF
    if (!advanced_view_is_on) {
        bool advanced_view_only = AD()->attribute_get(dn, "showInAdvancedViewOnly") == "TRUE";

        if (advanced_view_only) {
            return false;
        }
    }

    return true;
}
