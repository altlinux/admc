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

#include <Qt>

#include "query_item.h"

QString index_data_filter(const QModelIndex &index) {
    return index.data(QueryItemRole_Filter).toString();
}

QString index_data_base(const QModelIndex &index) {
    return index.data(QueryItemRole_Base).toString();
}

bool index_data_scope_is_children(const QModelIndex &index) {
    return index.data(QueryItemRole_ScopeIsChildren).toBool();
}

QString index_data_query_name(const QModelIndex &index) {
    return index.data(Qt::DisplayRole).toString();
}

QString index_data_description(const QModelIndex &index) {
    return index.data(QueryItemRole_Description).toString();
}

QByteArray index_data_filter_state(const QModelIndex &index) {
    return index.data(QueryItemRole_FilterState).toByteArray();
}
