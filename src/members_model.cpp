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

#include "members_model.h"
#include "ad_interface.h"
#include "entry_drag_drop.h"

#include <QMimeData>

MembersModel::MembersModel(QObject *parent)
: QStandardItemModel(0, Column::COUNT, parent)
{
    change_target(QString(""));
}

void MembersModel::change_target(const QString &new_target_dn) {
    target_dn = new_target_dn;

    clear();
    setHorizontalHeaderItem(Column::Name, new QStandardItem("Name"));

    // Populate model with members of new root
    const QList<QString> members = get_attribute_multi(target_dn, "member");
    for (auto dn : members) {
        const QString name = get_attribute(dn, "name");

        const auto name_item = new QStandardItem(name);
        const auto dn_item = new QStandardItem(dn);

        appendRow({name_item, dn_item});
    }
}

QString MembersModel::get_dn_of_index(const QModelIndex &index) {
    QModelIndex dn_index = index.siblingAtColumn(Column::DN);
    QString dn = dn_index.data().toString();

    return dn;
}

QMimeData *MembersModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *data = QStandardItemModel::mimeData(indexes);

    if (indexes.size() > 0) {
        QModelIndex index = indexes[0];
        QString dn = get_dn_of_index(index);

        data->setText(dn);
    }

    return data;
}

bool MembersModel::canDropMimeData(const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent) const {
    const QString dn = data->text();
    const QString parent_dn = get_dn_of_index(parent);

    return can_drop_entry(dn, parent_dn);
}

bool MembersModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) {
    if (row != -1 || column != -1) {
        return true;
    }

    if (canDropMimeData(data, action, row, column, parent)) {
        QString dn = data->text();
        QString parent_dn = get_dn_of_index(parent);

        drop_entry(dn, parent_dn);
    }

    return true;
}
