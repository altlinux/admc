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
    setHorizontalHeaderItem(Column::Name, new QStandardItem("Name"));
    setHorizontalHeaderItem(Column::DN, new QStandardItem("DN"));
}

QModelIndex MembersModel::change_target(const QString &new_target_dn) {
    target_dn = new_target_dn;

    removeRows(0, rowCount());

    // Create root item to represent group itself
    const QString grp_name = AD()->get_attribute(target_dn, "name");
    const auto grp_name_item = new QStandardItem(grp_name);
    const auto grp_dn_item = new QStandardItem(target_dn);
    appendRow({grp_name_item, grp_dn_item});

    // Populate model with members of new root
    const QList<QString> members = AD()->get_attribute_multi(target_dn, "member");
    for (auto dn : members) {
        const QString name = AD()->get_attribute(dn, "name");

        const auto name_item = new QStandardItem(name);
        const auto dn_item = new QStandardItem(dn);

        grp_name_item->appendRow({name_item, dn_item});
    }

    QModelIndex grp_index = grp_name_item->index();

    return grp_index;
}
