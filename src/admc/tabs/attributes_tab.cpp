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
#include "edit_dialogs/edit_dialog.h"
#include "ad_interface.h"
#include "ad_config.h"
#include "utils.h"
#include "attribute_display.h"

#include <QTreeView>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QMenu>

enum AttributesColumn {
    AttributesColumn_Name,
    AttributesColumn_Value,
    AttributesColumn_COUNT,
};

AttributesTab::AttributesTab() {
    model = new QStandardItemModel(0, AttributesColumn_COUNT, this);
    set_horizontal_header_labels_from_map(model, {
        {AttributesColumn_Name, tr("Name")},
        {AttributesColumn_Value, tr("Value")}
    });

    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSelectionMode(QAbstractItemView::NoSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSortingEnabled(true);
    view->setContextMenuPolicy(Qt::CustomContextMenu);

    proxy = new AttributesTabProxy(this);

    proxy->setSourceModel(model);
    view->setModel(proxy);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(view);

    connect(
        view, &QWidget::customContextMenuRequested,
        this, &AttributesTab::on_context_menu);
    connect(
        view, &QAbstractItemView::doubleClicked,
        this, &AttributesTab::on_double_clicked);
}

void AttributesTab::on_double_clicked(const QModelIndex &proxy_index) {
    const QModelIndex index = proxy->mapToSource(proxy_index);
    
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

    EditDialog *dialog = EditDialog::make(attribute, values);
    if (dialog != nullptr) {
        connect(
            dialog, &QDialog::accepted,
            [this, dialog, attribute, row]() {
                const QList<QByteArray> new_values = dialog->get_new_values();

                current[attribute] = new_values;
                load_row(row, attribute, new_values);

                emit edited();
            });

        dialog->open();
    }
}

// TODO: add options to show/hide attributes based on if they are in the "may contain" group (optional). Need to go through all parent classes to figure out the full set of such attributes.
void AttributesTab::on_context_menu(const QPoint pos) {
    QMenu menu;

    QAction *hide_unset = menu.addAction(tr("Hide unset"),
        [this](bool checked) {
            proxy->hide_unset = checked;
            proxy->invalidate();
        });
    hide_unset->setCheckable(true);
    hide_unset->setChecked(proxy->hide_unset);

    QAction *hide_read_only = menu.addAction(tr("Hide read only"),
        [this](bool checked) {
            proxy->hide_read_only = checked;
            proxy->invalidate();
        });
    hide_read_only->setCheckable(true);
    hide_read_only->setChecked(proxy->hide_read_only);

    exec_menu_from_view(&menu, view, pos);
}

void AttributesTab::load(const AdObject &object) {
    for (auto attribute : object.attributes()) {
        original[attribute] = object.get_values(attribute);
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

    model->removeRows(0, model->rowCount());

    proxy->unset_map.clear();
    proxy->read_only_map.clear();

    for (auto attribute : original.keys()) {
        const QList<QStandardItem *> row = make_item_row(AttributesColumn_COUNT);
        const QList<QByteArray> values = original[attribute];

        model->appendRow(row);
        load_row(row, attribute, values);
    }

    model->sort(AttributesColumn_Name);
}

void AttributesTab::apply(const QString &target) const {
    for (const QString &attribute : current.keys()) {
        const QList<QByteArray> current_values = current[attribute];
        const QList<QByteArray> original_values = original[attribute];

        if (current_values != original_values) {
            AD()->attribute_replace_values(target, attribute, current_values);
        }
    }
}

void AttributesTab::load_row(const QList<QStandardItem *> &row, const QString &attribute, const QList<QByteArray> &values) {
    const QString display_values = attribute_display_values(attribute, values);
    const bool unset = values.isEmpty();
    const bool read_only = ADCONFIG()->get_attribute_is_system_only(attribute);

    row[AttributesColumn_Name]->setText(attribute);
    row[AttributesColumn_Value]->setText(display_values);

    proxy->unset_map[attribute] = unset;
    proxy->read_only_map[attribute] = read_only;
}

bool AttributesTabProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    auto source = sourceModel();
    const QString attribute = source->index(source_row, AttributesColumn_Name, source_parent).data().toString();
    const bool read_only = read_only_map[attribute];
    const bool unset = unset_map[attribute];

    if (hide_unset && unset) {
        return false;
    } else if (hide_read_only && read_only) {
        return false;
    } else {
        return true;
    }
}
