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
}

void ContentsWidget::on_containers_selected_changed(const QString &dn) {
    model->removeRows(0, model->rowCount());

    // Load head
    QStandardItem *root = model->invisibleRootItem();    
    make_new_row(root, dn);
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
