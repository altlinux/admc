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

#include "tabs/attributes_tab.h"
#include "attributes_tab_dialogs/attributes_tab_dialog.h"
#include "ad_interface.h"
#include "utils.h"
#include "ad_config.h"
#include "attribute_display.h"

#include <QTreeView>
#include <QVBoxLayout>

enum AttributesColumn {
    AttributesColumn_Name,
    AttributesColumn_Value,
    AttributesColumn_COUNT,
};

AttributesTab::AttributesTab() {
    model = new AttributesModel(this);

    auto view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSelectionMode(QAbstractItemView::NoSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSortingEnabled(true);
    view->setModel(model);

    const auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(view);

    connect(
        view, &QAbstractItemView::doubleClicked,
        this, &AttributesTab::on_double_clicked);
}

void AttributesTab::on_double_clicked(const QModelIndex &index) {
    const int column = index.column();
    if (column != AttributesColumn_Value) {
        return;
    }

    const QModelIndex attribute_index = index.siblingAtColumn(AttributesColumn_Name);
    QStandardItem *attribute_item = model->itemFromIndex(attribute_index);
    const QModelIndex value_index = index.siblingAtColumn(AttributesColumn_Value);
    QStandardItem *value_item = model->itemFromIndex(value_index);
    const QList<QStandardItem *> row = {attribute_item, value_item};

    const QString attribute = attribute_item->text();
    const QList<QByteArray> values = current[attribute];

    AttributesTabDialog *dialog = AttributesTabDialog::make(attribute, values);
    if (dialog != nullptr) {
        connect(
            dialog, &QDialog::accepted,
            [this, dialog, attribute, row]() {
                const QList<QByteArray> new_values = dialog->get_new_values();

                current[attribute] = new_values;
                model->load_row(row, attribute, new_values);

                emit edited();
            });

        dialog->open();
    }
}

void AttributesTab::load(const AdObject &object) {
    for (auto attribute : object.attributes()) {
        original[attribute] = object.get_bytes_list(attribute);
    }

    // Add attributes without values
    const QList<QString> object_classes = object.get_strings(ATTRIBUTE_OBJECT_CLASS);
    const QList<QString> possible_attributes = ADCONFIG()->get_possible_attributes(object_classes);
    for (const QString attribute : possible_attributes) {
        if (!original.contains(attribute)) {
            original[attribute] = QList<QByteArray>();
        }
    }

    current = original;

    model->load(current);
}

void AttributesTab::reset() {
    current = original;
    
    model->load(current);
}

bool AttributesTab::changed() const {
    return original != current;
}

void AttributesTab::apply(const QString &target) const {
    for (const QString &attribute : current.keys()) {
        const QList<QByteArray> current_values = current[attribute];
        const QList<QByteArray> original_values = original[attribute];

        if (current_values != original_values) {
            // AD()->attribute_replace_values(target, attribute, current_values);
        }
    }
}

AttributesModel::AttributesModel(QObject *parent)
: QStandardItemModel(0, AttributesColumn_COUNT, parent)
{
    set_horizontal_header_labels_from_map(this, {
        {AttributesColumn_Name, tr("Name")},
        {AttributesColumn_Value, tr("Value")}
    });
}

void AttributesModel::load_row(const QList<QStandardItem *> &row, const QString &attribute, const QList<QByteArray> &values) {
    const QString display_values = attribute_display_values(attribute, values);

    row[AttributesColumn_Name]->setText(attribute);
    row[AttributesColumn_Value]->setText(display_values);
}

void AttributesModel::load(const AdObjectAttributes &attributes) {
    removeRows(0, rowCount());

    // Populate model with attributes of new root
    for (auto attribute : attributes.keys()) {
        const QList<QStandardItem *> row = make_item_row(AttributesColumn_COUNT);
        const QList<QByteArray> values = attributes[attribute];

        appendRow(row);
        load_row(row, attribute, values);
    }

    sort(AttributesColumn_Name);
}
