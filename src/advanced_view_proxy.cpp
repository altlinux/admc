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

    const BoolSettingSignal *advanced_view_setting = Settings::instance()->get_bool_signal(BoolSetting_AdvancedView);
    connect(
        advanced_view_setting, &BoolSettingSignal::changed,
        this, &AdvancedViewProxy::on_advanced_view_toggled);
    on_advanced_view_toggled();
}

void AdvancedViewProxy::on_advanced_view_toggled() {
    advanced_view_is_on = Settings::instance()->get_bool(BoolSetting_AdvancedView);

    invalidateFilter();
}

bool AdvancedViewProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {

    const QModelIndex base_index = sourceModel()->index(source_row, 0, source_parent);
    const QString dn = get_dn_from_index(base_index, dn_column);

    // Hide advanced view only objects if advanced view is OFF
    if (!advanced_view_is_on) {
        bool advanced_view_only = AdInterface::instance()->attribute_get(dn, ATTRIBUTE_SHOW_IN_ADVANCED_VIEW_ONLY) == "TRUE";

        if (advanced_view_only) {
            return false;
        }
    }

    return true;
}
