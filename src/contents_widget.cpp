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

#include "contents_widget.h"
#include "containers_widget.h"
#include "ad_interface.h"
#include "ad_model.h"
#include "entry_proxy_model.h"
#include "entry_model.h"

#include <QTreeView>
#include <QLabel>
#include <QLayout>

ContentsWidget::ContentsWidget(EntryModel *model_arg, ContainersWidget *containers_widget, QWidget *parent)
: EntryWidget(model_arg, parent)
{   
    model = model_arg;
    model->setHorizontalHeaderItem(Column::Name, new QStandardItem("Name"));
    model->setHorizontalHeaderItem(Column::Category, new QStandardItem("Category"));
    model->setHorizontalHeaderItem(Column::Description, new QStandardItem("Description"));
    model->setHorizontalHeaderItem(Column::DN, new QStandardItem("DN"));

    proxy = new EntryProxyModel(model, this);

    view->setAcceptDrops(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setRootIsDecorated(false);
    view->setItemsExpandable(false);
    view->setExpandsOnDoubleClick(false);
    view->setModel(proxy);

    // Insert label into layout
    const auto label = new QLabel("Contents");
    layout()->removeWidget(view);
    layout()->addWidget(label);
    layout()->addWidget(view);

    column_hidden[Column::Name] = false;
    column_hidden[Column::Category] = false;
    column_hidden[Column::Description] = false;
    update_column_visibility();

    connect(
        containers_widget, &ContainersWidget::selected_changed,
        this, &ContentsWidget::on_containers_selected_changed);

    connect(
        AD(), &AdInterface::create_entry_complete,
        this, &ContentsWidget::on_create_entry_complete);
    connect(
        AD(), &AdInterface::delete_entry_complete,
        this, &ContentsWidget::on_delete_entry_complete);
    connect(
        AD(), &AdInterface::dn_changed,
        this, &ContentsWidget::on_dn_changed);
    connect(
        AD(), &AdInterface::attributes_changed,
        this, &ContentsWidget::on_attributes_changed);
}

void ContentsWidget::make_new_row(QStandardItem *parent, const QString &dn) {
    auto row = QList<QStandardItem *>();

    for (int i = 0; i < Column::COUNT; i++) {
        row.push_back(new QStandardItem());
    }

    parent->appendRow(row);

    // Load values into row
    QString name = AD()->get_attribute(dn, "name");

    // NOTE: this is given as raw DN and contains '-' where it should
    // have spaces, so convert it
    QString category = AD()->get_attribute(dn, "objectCategory");
    category = extract_name_from_dn(category);
    category = category.replace('-', ' ');

    QString description = AD()->get_attribute(dn, "description");

    row[Column::Name]->setText(name);
    row[Column::Category]->setText(category);
    row[Column::Description]->setText(description);

    QIcon icon = get_entry_icon(dn);
    row[0]->setIcon(icon);
}

void ContentsWidget::on_containers_selected_changed(const QString &dn) {
    model->removeRows(0, model->rowCount());

    // Load head
    head_dn = dn;
    QStandardItem *root = model->invisibleRootItem();    
    make_new_row(root, head_dn);
    QStandardItem *head = root->child(0, 0);

    const QModelIndex head_index = head->index();
    const QModelIndex proxy_head_index = proxy->mapFromSource(head_index);
    view->setRootIndex(proxy_head_index);

    // Load children
    QList<QString> children = AD()->load_children(dn);
    for (auto child_dn : children) {
        make_new_row(head, child_dn);
    }

    update_column_visibility();
}

void ContentsWidget::on_delete_entry_complete(const QString &dn) {
    QList<QStandardItem *> items = findItems(dn, Qt::MatchExactly | Qt::MatchRecursive, ContentsWidget::Column::DN);

    if (items.size() > 0) {
        QStandardItem *dn_item = items[0];
        QModelIndex dn_index = dn_item->index();
        
        removeRow(dn_index.row(), dn_index.parent());
    }
}

void ContentsWidget::on_dn_changed(const QString &old_dn, const QString &new_dn) {
    const bool head_changed = (old_dn == head_dn);

    const QString old_parent = extract_parent_dn_from_dn(old_dn);
    const QString new_parent = extract_parent_dn_from_dn(new_dn);
    const bool child_changed = (old_parent == head_dn || new_parent == head_dn);

    if (!head_changed && !child_changed) {
        return;
    }

    QList<QStandardItem *> old_items = findItems(old_dn, Qt::MatchExactly | Qt::MatchRecursive, ContentsWidget::Column::DN);

    if (items.size() == 0) {
        return;
    }

    // Update DN
    QStandardItem *dn_item = old_items[0];
    QModelIndex dn_index = dn_item->index();
    dn_item->setText(new_dn);

    if (child_changed) {
        if (new_parent != head_dn) {
            QStandardItem *dn_item = items[0];
            dn_item->setText(new_dn);
        }
    }
}

void ContentsWidget::on_create_entry_complete(const QString &dn, NewEntryType type) {
    // Load entry to model if it's parent has already been fetched
    // If it hasn't been fetched, then this new entry will be loaded with all other children when the parent is fetched
    QString parent_dn = extract_parent_dn_from_dn(dn);
    QList<QStandardItem *> items = findItems(parent_dn, Qt::MatchExactly | Qt::MatchRecursive, Column::DN);

    if (items.size() > 0) {
        QStandardItem *dn_item = items[0];
        QModelIndex dn_index = dn_item->index();
        QModelIndex parent_index = dn_index.siblingAtColumn(0);
        QStandardItem *parent = itemFromIndex(parent_index);

        bool fetched_already = !canFetchMore(parent_index);
        if (fetched_already) {
            make_new_row(parent, dn);
        }
    }
}

void ContentsWidget::on_attributes_changed(const QString &dn) {
    // Compose row based on dn
    QList<QStandardItem *> items = findItems(dn, Qt::MatchExactly | Qt::MatchRecursive, ContentsWidget::Column::DN);

    if (items.size() == 0) {
        return;
    }

    QStandardItem *dn_item = items[0];
    QModelIndex dn_index = dn_item->index();

    // Compose indexes for all columns
    auto indexes = QList<QModelIndex>(); 
    for (int i = 0; i < ContentsWidget::Column::COUNT; i++) {
        QModelIndex index = dn_index.siblingAtColumn(i);
        indexes.push_back(index);
    }

    // Compose the row of items from indexes
    auto row = QList<QStandardItem *>();
    for (int i = 0; i < ContentsWidget::Column::COUNT; i++) {
        QModelIndex index = indexes[i];
        QStandardItem *item = itemFromIndex(index);
        row.push_back(item);
    }

    load_row(row, dn);
}
