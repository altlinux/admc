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

#include <QMimeData>

MembersModel::MembersModel(QObject *parent)
: EntryModel(Column::COUNT, Column::DN, parent)
{
    change_target(QString(""));
}

void MembersModel::change_target(const QString &new_target_dn) {
    target_dn = new_target_dn;

    clear();
    setHorizontalHeaderItem(Column::Name, new QStandardItem("Name"));
    setHorizontalHeaderItem(Column::DN, new QStandardItem("DN"));

    // Populate model with members of new root
    const QList<QString> members = get_attribute_multi(target_dn, "member");
    for (auto dn : members) {
        const QString name = get_attribute(dn, "name");

        const auto name_item = new QStandardItem(name);
        const auto dn_item = new QStandardItem(dn);

        appendRow({name_item, dn_item});
    }
}

QString MembersModel::get_dn_from_index(const QModelIndex &index) const {
    // NOTE: the invisible root item is considered to be 
    // the target group
    // This is so that drops onto invisible root work as drops
    // onto the target group
    if (index == invisibleRootItem()->index()) {
        return target_dn;
    } else {
        return EntryModel::get_dn_from_index(index);
    }
}
