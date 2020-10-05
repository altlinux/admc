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

#include "attributes_tab.h"
#include "ad_interface.h"
#include "utils.h"
#include "server_configuration.h"

#include <QTreeView>
#include <QVBoxLayout>

enum AttributesColumn {
    AttributesColumn_Name,
    AttributesColumn_Value,
    AttributesColumn_COUNT,
};

AttributesTab::AttributesTab()
: DetailsTab()
{
    model = new AttributesModel(this);

    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSelectionMode(QAbstractItemView::NoSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSortingEnabled(true);
    view->setModel(model);

    const auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(view);
}

bool AttributesTab::changed() const {
    return false;
}

bool AttributesTab::verify() {
    return true;
}

void AttributesTab::apply(const QString &target) {

}

void AttributesTab::load(const QString &target, const Attributes &attributes) {
    model->reload(target, attributes);
}

bool AttributesTab::accepts_target(const Attributes &attributes) const {
    return true;
}

AttributesModel::AttributesModel(AttributesTab *attributes_tab_arg)
: QStandardItemModel(0, AttributesColumn_COUNT, attributes_tab_arg)
{
    attributes_tab = attributes_tab_arg;

    set_horizontal_header_labels_from_map(this, {
        {AttributesColumn_Name, tr("Name")},
        {AttributesColumn_Value, tr("Value")}
    });
}

// This will be called when an attribute value is edited
bool AttributesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    // TODO: need to store changes to be apply when apply() is called
    return true;
}

void AttributesModel::reload(const QString &target, const Attributes &attributes) {
    removeRows(0, rowCount());

    // Populate model with attributes of new root
    for (auto attribute : attributes.keys()) {
        const QList<QByteArray> values = attributes[attribute];

        for (auto value : values) {
            const QString display_value = attribute_get_display_value(attribute, value);

            auto name_item = new QStandardItem(attribute);
            auto value_item = new QStandardItem(display_value);

            name_item->setEditable(false);

            appendRow({name_item, value_item});
        }
    }

    // Add attributes without values
    const QList<QString> possible_attributes = get_possible_attributes(attributes);
    for (const QString attribute : possible_attributes) {
        if (!attributes.contains(attribute)) {
            auto name_item = new QStandardItem(attribute);
            auto value_item = new QStandardItem("<unset>");
            
            appendRow({name_item, value_item});
        }
    }
}
