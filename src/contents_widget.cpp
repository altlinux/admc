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
#include "advanced_view_proxy.h"
#include "entry_model.h"
#include "entry_context_menu.h"
#include "dn_column_proxy.h"
#include "utils.h"

#include <QTreeView>
#include <QLabel>
#include <QLayout>

ContentsWidget::ContentsWidget(ContainersWidget *containers_widget, EntryContextMenu *entry_context_menu, QWidget *parent)
: QWidget(parent)
{   
    model = new ContentsModel(this);

    const auto proxy = new AdvancedViewProxy(ContentsModel::Column::DN, this);
    proxy->setSourceModel(model);   

    const auto dn_column_proxy = new DnColumnProxy(ContentsModel::Column::DN, this);
    dn_column_proxy->setSourceModel(proxy);

    view = new QTreeView(this);
    view->setModel(dn_column_proxy);
    view->setAcceptDrops(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setRootIsDecorated(false);
    view->setItemsExpandable(false);
    view->setExpandsOnDoubleClick(false);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setDragDropMode(QAbstractItemView::DragDrop);
    entry_context_menu->connect_view(view, ContentsModel::Column::DN);

    // Insert label into layout
    const auto label = new QLabel("Contents");

    const auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->removeWidget(view);
    layout->addWidget(label);
    layout->addWidget(view);

    connect(
        containers_widget, &ContainersWidget::selected_changed,
        this, &ContentsWidget::on_containers_selected_changed);

    connect(
        view, &QAbstractItemView::clicked,
        [this] (const QModelIndex &index) {
            const QString dn = get_dn_from_index(index, ContentsModel::Column::DN);

            emit clicked_dn(dn);
        });
}

void ContentsWidget::on_containers_selected_changed(const QString &dn) {
    model->change_target(dn);

    set_root_to_head(view);
}

ContentsModel::ContentsModel(QObject *parent)
: EntryModel(Column::COUNT, Column::DN, parent)
{
    setHorizontalHeaderItem(Column::Name, new QStandardItem("Name"));
    setHorizontalHeaderItem(Column::Category, new QStandardItem("Category"));
    setHorizontalHeaderItem(Column::Description, new QStandardItem("Description"));
    setHorizontalHeaderItem(Column::DN, new QStandardItem("DN"));

    connect(
        AD(), &AdInterface::create_entry_complete,
        this, &ContentsModel::on_create_entry_complete);
    connect(
        AD(), &AdInterface::dn_changed,
        this, &ContentsModel::on_dn_changed);
    connect(
        AD(), &AdInterface::attributes_changed,
        this, &ContentsModel::on_attributes_changed);
}

void ContentsModel::change_target(const QString &dn) {
    removeRows(0, rowCount());

    if (dn == "") {
        return;
    }

    // Load head
    target_dn = dn;
    QStandardItem *root = invisibleRootItem();    
    make_new_row(root, target_dn);
    QStandardItem *head = item(0, 0);

    // Load children
    QList<QString> children = AD()->load_children(dn);
    for (auto child_dn : children) {
        make_new_row(head, child_dn);
    }
}

void ContentsModel::on_create_entry_complete(const QString &dn, NewEntryType type) {
    QString parent_dn = extract_parent_dn_from_dn(dn);

    if (parent_dn == target_dn) {
        QStandardItem *head = item(0, 0);
        make_new_row(head, dn);
    }
}

void ContentsModel::on_dn_changed(const QString &old_dn, const QString &new_dn) {
    const bool deleted = (new_dn == "");

    if (deleted && old_dn == target_dn) {
        change_target("");

        return;
    }

    const QString new_parent = extract_parent_dn_from_dn(new_dn);
    const bool is_child = (old_dn != target_dn);
    if (is_child && new_parent != target_dn) {
        // Remove row
        QList<QStandardItem *> row = find_row(old_dn);

        if (!row.isEmpty()) {
            const QStandardItem *item = row[0];
            item = row[2];
            QStandardItem *parent = item->parent();
            const int row_i = item->row();

            parent->removeRow(row_i);
        }
    } else {
        // Update DN
        QStandardItem *dn_item = find_item(old_dn, Column::DN);

        if (dn_item != nullptr) {
            dn_item->setText(new_dn);
        }
    }
}

void ContentsModel::on_attributes_changed(const QString &dn) {
    QList<QStandardItem *> row = find_row(dn);

    if (!row.isEmpty()) {
        load_row(row, dn);
    }
}

void ContentsModel::load_row(QList<QStandardItem *> row, const QString &dn) {
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
    row[Column::DN]->setText(dn);

    QIcon icon = get_entry_icon(dn);
    row[0]->setIcon(icon);
}

void ContentsModel::make_new_row(QStandardItem *parent, const QString &dn) {
    auto row = QList<QStandardItem *>();
    for (int i = 0; i < Column::COUNT; i++) {
        row.push_back(new QStandardItem());
    }

    load_row(row, dn);

    parent->appendRow(row);
}
