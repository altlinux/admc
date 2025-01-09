/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#include "console_widget/scope_proxy_model.h"

#include "console_widget/console_widget_p.h"

// This tricks the view into thinking that an item in tree
// has children while the item is unfetched. This causes the
// expander to be shown while the item is unfetched. After
// the item is fetched, normal behavior is restored.
bool ScopeProxyModel::hasChildren(const QModelIndex &parent) const {
    const bool was_fetched = parent.data(ConsoleRole_WasFetched).toBool();
    const bool unfetched = !was_fetched;

    if (unfetched) {
        return true;
    } else {
        return QSortFilterProxyModel::hasChildren(parent);
    }
}

bool ScopeProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    const QModelIndex source_index = sourceModel()->index(source_row, 0, source_parent);

    const bool is_scope = source_index.data(ConsoleRole_IsScope).toBool();

    return is_scope;
}

bool ScopeProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
    const int left_sort_index = left.data(ConsoleRole_SortIndex).toInt();
    const int right_sort_index = right.data(ConsoleRole_SortIndex).toInt();

    if (left_sort_index != right_sort_index) {
        const bool out = (left_sort_index < right_sort_index);

        return out;
    }

    const bool out = QSortFilterProxyModel::lessThan(left, right);

    return out;
}
