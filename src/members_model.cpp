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

MembersModel::MembersModel(QObject *parent)
: QStandardItemModel(0, Column::COUNT, parent)
{
    change_target(QString(""));

    connect(
        &ad_interface, &AdInterface::delete_entry_complete,
        this, &MembersModel::on_delete_entry_complete);
    connect(
        &ad_interface, &AdInterface::move_user_complete,
        this, &MembersModel::on_move_user_complete);
    connect(
        &ad_interface, &AdInterface::load_attributes_complete,
        this, &MembersModel::on_load_attributes_complete);
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

void MembersModel::on_delete_entry_complete(const QString &dn) {
    // Clear data if current target was deleted
    // NOTE: if dn is member of target, then the update will propagate
    // through on_load_attributes_complete
    if (target_dn == dn) {
        change_target(QString(""));
    }
}

void MembersModel::on_move_user_complete(const QString &user_dn, const QString &container_dn, const QString &new_dn) {
    // Switch to the entry at new dn (entry stays the same)
    if (target_dn == user_dn) {
        change_target(new_dn);
    }
}

void MembersModel::on_load_attributes_complete(const QString &dn) {
    // Reload entry since attributes were updated
    if (target_dn == dn) {
        change_target(dn);
    }
}
