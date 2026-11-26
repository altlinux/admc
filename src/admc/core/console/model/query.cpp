/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2026 BaseALT Ltd.
 * Copyright (C) 2026 Artyom V. Poptsov
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

/**
 * @file query.cpp
 * @brief Console query model index implementation.
 */

#include "core/console/query_item.h"
#include "core/console/model/console.h"
#include "core/console/model/query.h"

ConsoleQueryModelIndex::ConsoleQueryModelIndex(const QModelIndex& index)
    : ConsoleModelIndex(index) {
    // Do nothing.
}

QString ConsoleQueryModelIndex::get_description() const {
    return data(QueryItemRole_Description).toString();
}

QString ConsoleQueryModelIndex::get_filter() const {
    return data(QueryItemRole_Filter).toString();
}

QByteArray ConsoleQueryModelIndex::get_filter_state() const {
    return data(QueryItemRole_FilterState).toByteArray();
}

QString ConsoleQueryModelIndex::get_base() const {
    return data(QueryItemRole_Base).toString();
}

bool ConsoleQueryModelIndex::is_children_scope() const {
    return data(QueryItemRole_ScopeIsChildren).toBool();
}

QStringList ConsoleQueryModelIndex::get_sibling_name_list(
    const QModelIndex &parent_idx,
    const QModelIndex &index_to_omit)
{
    const QAbstractItemModel *model = parent_idx.model();
    const int row_count = model->rowCount(parent_idx);

    QStringList out;
    for (int row = 0; row < row_count; row++) {
        const ConsoleQueryModelIndex sibling(model->index(row, 0, parent_idx));
        const bool this_is_query_itself = (sibling == index_to_omit);
        if (this_is_query_itself) {
            continue;
        }

        out.append(sibling.get_name());
    }

    return out;
}
