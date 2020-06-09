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

#include "entry_model.h"
#include "ad_interface.h"

#include <QMimeData>

EntryModel::EntryModel(int column_count, int dn_column_in, QObject *parent)
: QStandardItemModel(0, column_count, parent)
, dn_column(dn_column_in)
{

}

QString EntryModel::get_dn_from_index(const QModelIndex &index) const {
    QModelIndex dn_index = index.siblingAtColumn(dn_column);
    QString dn = dn_index.data().toString();

    return dn;
}

QMimeData *EntryModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *data = QStandardItemModel::mimeData(indexes);

    if (indexes.size() > 0) {
        QModelIndex index = indexes[0];
        QString dn = get_dn_from_index(index);

        data->setText(dn);
    }

    return data;
}

bool EntryModel::canDropMimeData(const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent) const {
    const QString dn = data->text();
    const QString parent_dn = get_dn_from_index(parent);

    const bool can_drop = AD()->can_drop_entry(dn, parent_dn);

    return can_drop;
}

bool EntryModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) {
    if (row != -1 || column != -1) {
        return true;
    }

    if (!canDropMimeData(data, action, row, column, parent)) {
        return true;
    }

    const QString dn = data->text();
    const QString parent_dn = get_dn_from_index(parent);

    AD()->drop_entry(dn, parent_dn);

    return true;
}
