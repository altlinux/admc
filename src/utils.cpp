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

#include "utils.h"

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

// Converts index all the way down to source index, going through whatever chain of proxies is present
QModelIndex convert_to_source(const QModelIndex &index, const QAbstractItemModel *model) {
    QModelIndex current_index = index;
    const QAbstractItemModel *current_model = model;

    while (true) {
        const QSortFilterProxyModel *proxy = qobject_cast<const QSortFilterProxyModel *>(current_model);

        if (proxy != nullptr) {
            current_index = proxy->mapToSource(current_index);

            current_model = proxy->sourceModel();
        } else {
            return current_index;
        }
    }
}
