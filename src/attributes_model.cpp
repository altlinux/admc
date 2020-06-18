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
    setHorizontalHeaderItem(Column::Name, new QStandardItem("Name"));
    setHorizontalHeaderItem(Column::Value, new QStandardItem("Value"));
    
    change_target("");
}

// This will be called when an attribute value is edited
bool AttributesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    QModelIndex value_index = index;
    QModelIndex name_index = value_index.siblingAtColumn(AttributesModel::Column::Name);

    const QString attribute = name_index.data().toString();
    const QString value_str = value.toString();

    bool success = AD()->set_attribute(target_dn, attribute, value_str);

    if (success) {
        QStandardItemModel::setData(index, value, role);

        return true;
    } else {
        return false;
    }
}

void AttributesModel::change_target(const QString &new_target_dn) {
    target_dn = new_target_dn;

    removeRows(0, rowCount());

    // Populate model with attributes of new root
    QMap<QString, QList<QString>> attributes = AD()->get_attributes(target_dn);
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
