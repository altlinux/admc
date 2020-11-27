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

#include "object_model.h"
#include "ad_interface.h"
#include "utils.h"

#include <QMimeData>
#include <QString>

ObjectModel::ObjectModel(int column_count, int dn_column_in, QObject *parent)
: QStandardItemModel(0, column_count, parent)
, dn_column(dn_column_in)
{

}

QMimeData *ObjectModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *data = QStandardItemModel::mimeData(indexes);

    if (indexes.size() > 0) {
        QModelIndex index = indexes[0];
        QString dn = get_dn_from_index(index, dn_column);

        data->setText(dn);
    }

    return data;
}

bool ObjectModel::canDropMimeData(const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent) const {
    const QString dn = data->text();
    const QString target_dn = get_dn_from_index(parent, dn_column);

    const bool can_drop = AD()->object_can_drop(dn, target_dn);

    return can_drop;
}

bool ObjectModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) {
    if (row != -1 || column != -1) {
        return true;
    }

    if (!canDropMimeData(data, action, row, column, parent)) {
        return true;
    }

    const QString dn = data->text();
    const QString target_dn = get_dn_from_index(parent, dn_column);

    AD()->object_drop(dn, target_dn);

    return true;
}

QList<QStandardItem *> ObjectModel::find_row(const QString &dn) {
    // Find dn item
    const QList<QStandardItem *> dn_items = findItems(dn, Qt::MatchExactly | Qt::MatchRecursive, dn_column);
    if (dn_items.isEmpty()) {
        return QList<QStandardItem *>();
    }

    const QStandardItem *dn_item = dn_items[0];
    const QStandardItem *parent = dn_item->parent();
    const int column_count = parent->columnCount();
    const int row_i = dn_item->row();

    // Compose the row
    auto row = QList<QStandardItem *>();
    for (int col_i = 0; col_i < column_count; col_i++) {
        QStandardItem *item = parent->child(row_i, col_i);
        row.append(item);
    }

    return row;
}

QStandardItem *ObjectModel::find_item(const QString &dn, int col) {
    QList<QStandardItem *> row = find_row(dn);
    QStandardItem *item = row.value(col);

    return item;
}
