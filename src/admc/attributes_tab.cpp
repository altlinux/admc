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

#include <QTreeView>
#include <QVBoxLayout>

enum AttributesColumn {
    AttributesColumn_Name,
    AttributesColumn_Value,
    AttributesColumn_COUNT,
};

AttributesTab::AttributesTab(DetailsWidget *details_widget_arg)
: DetailsTab(details_widget_arg)
{
    model = new AttributesModel(this);

    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSelectionMode(QAbstractItemView::NoSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
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

void AttributesTab::apply() {

}

void AttributesTab::reload() {
    model->reload();
}

bool AttributesTab::accepts_target() const {
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
    QModelIndex value_index = index;
    QModelIndex name_index = value_index.siblingAtColumn(AttributesColumn_Name);

    const QString target = attributes_tab->target();
    const QString attribute = name_index.data().toString();
    const QString value_str = value.toString();

    const bool replace_success = AdInterface::instance()->attribute_replace(target, attribute, value_str);

    if (replace_success) {
        QStandardItemModel::setData(index, value, role);

        return true;
    } else {
        return false;
    }
}

void AttributesModel::reload() {
    removeRows(0, rowCount());

    // Populate model with attributes of new root
    const QString target = attributes_tab->target();
    QMap<QString, QList<QString>> attributes = AdInterface::instance()->get_all_attributes(target);
    for (auto attribute : attributes.keys()) {
        QList<QString> values = attributes[attribute];

        for (auto value : values) {
            const QString value_display =
            [target, attribute, value]() {
                if (attribute == ATTRIBUTE_OBJECT_SID) {
                    const QByteArray bytes = AdInterface::instance()->attribute_get_binary(target, attribute);
                    return object_sid_to_display_string(bytes);
                } else {
                    return value;
                }
            }();

            auto name_item = new QStandardItem(attribute);
            auto value_item = new QStandardItem(value_display);

            name_item->setEditable(false);

            appendRow({name_item, value_item});
        }
    }
}
