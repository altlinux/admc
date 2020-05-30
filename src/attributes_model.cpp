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

#include "attributes_model.h"
#include "ad_model.h"
#include "ad_interface.h"

AttributesModel::AttributesModel(QObject *parent)
: QStandardItemModel(0, Column::COUNT, parent)
{
    change_target(QString(""));

    QObject::connect(
        &ad_interface, &AdInterface::delete_entry_complete,
        this, &AttributesModel::on_delete_entry_complete);
    QObject::connect(
        &ad_interface, &AdInterface::move_user_complete,
        this, &AttributesModel::on_move_user_complete);
    QObject::connect(
        &ad_interface, &AdInterface::load_attributes_complete,
        this, &AttributesModel::on_load_attributes_complete);
}

// This will be called when an attribute value is edited
bool AttributesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    QModelIndex value_index = index;
    QModelIndex name_index = value_index.siblingAtColumn(AttributesModel::Column::Name);

    const QString attribute = name_index.data().toString();
    const QString value_str = value.toString();

    bool success = set_attribute(target_dn, attribute, value_str);

    // TODO: probably should add on_load_attributes_complete slot
    // and setData in there, maybe even just reload whole model
    // on each attribute edit to be safe

    if (success) {
        QStandardItemModel::setData(index, value, role);

        return true;
    } else {
        return false;
    }
}

void AttributesModel::change_target(const QString &new_target_dn) {
    target_dn = new_target_dn;

    // Clear old data
    clear();
    
    // NOTE: need to reset headers after clearing
    setHorizontalHeaderItem(Column::Name, new QStandardItem("Name"));
    setHorizontalHeaderItem(Column::Value, new QStandardItem("Value"));

    // Populate model with attributes of new root
    QMap<QString, QList<QString>> attributes = get_attributes(target_dn);
    for (auto attribute : attributes.keys()) {
        QList<QString> values = attributes[attribute];

        for (auto value : values) {
            auto name_item = new QStandardItem(attribute);
            auto value_item = new QStandardItem(value);

            name_item->setEditable(false);

            appendRow({name_item, value_item});
        }
    }
}

void AttributesModel::on_delete_entry_complete(const QString &dn) {
    // Clear data if current target was deleted
    if (target_dn == dn) {
        change_target(QString(""));
    }
}

void AttributesModel::on_move_user_complete(const QString &user_dn, const QString &container_dn, const QString &new_dn) {
    // Switch to the entry at new dn (entry stays the same)
    if (target_dn == user_dn) {
        change_target(new_dn);
    }
}

void AttributesModel::on_load_attributes_complete(const QString &dn) {
    // Reload entry since attributes were updated
    if (target_dn == dn) {
        change_target(dn);
    }
}
