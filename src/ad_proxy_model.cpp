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
#include "entry_model.h"
#include "ad_interface.h"
#include "settings.h"

#include <QAction>

AdProxyModel::AdProxyModel(EntryModel *model_arg, QObject *parent)
: QSortFilterProxyModel(parent)
{
    model = model_arg;

    setSourceModel(model);

    connect(
        SETTINGS()->toggle_advanced_view, &QAction::triggered,
        this, &AdProxyModel::on_advanced_view_toggled);
}

void AdProxyModel::on_advanced_view_toggled(bool) {
    invalidateFilter();
}

bool AdProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    const QModelIndex index = model->index(source_row, 0, source_parent);
    const QString dn = model->get_dn_from_index(index);

    // Hide advanced view only entries if advanced view is OFF
    const bool advanced_view_is_on = SETTINGS()->toggle_advanced_view->isChecked();
    if (!advanced_view_is_on) {
        bool advanced_view_only = AD()->get_attribute(dn, "showInAdvancedViewOnly") == "TRUE";

        if (advanced_view_only) {
            return false;
        }
    }

    // Hide non-containers
    if (only_show_containers) {
        bool is_container = AD()->is_container_like(dn) || AD()->is_container(dn);

        if (!is_container) {
            return false;
        }
    }

    return true;
}
