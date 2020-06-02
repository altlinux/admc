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
#include "constants.h"

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

    const bool dropped_is_user = attribute_value_exists(dn, "objectClass", "user");

    const bool parent_is_group = attribute_value_exists(parent_dn, "objectClass", "group");
    const bool parent_is_ou = attribute_value_exists(parent_dn, "objectClass", "organizationalUnit");
    const bool parent_is_container = attribute_value_exists(parent_dn, "objectClass", "container");

    // TODO: support dropping non-users
    // TODO: support dropping policies
    if (parent_dn == "") {
        return false;
    } else if (parent_dn == HEAD_DN) {
        return false;
    } else if (dropped_is_user && (parent_is_group || parent_is_ou || parent_is_container)) {
        return true;
    } else {
        return false;
    }
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

    const bool dropped_is_user = attribute_value_exists(dn, "objectClass", "user");

    const bool parent_is_group = attribute_value_exists(parent_dn, "objectClass", "group");
    const bool parent_is_ou = attribute_value_exists(parent_dn, "objectClass", "organizationalUnit");
    const bool parent_is_container = attribute_value_exists(parent_dn, "objectClass", "container");

    if (dropped_is_user && (parent_is_ou || parent_is_container)) {
        move_user(dn, parent_dn);
    } else if (dropped_is_user && parent_is_group) {
        add_user_to_group(parent_dn, dn);
    }

    return true;
}
