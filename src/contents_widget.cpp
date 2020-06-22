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
    model->setHorizontalHeaderItem(AdModel::Column::Name, new QStandardItem("Name"));
    model->setHorizontalHeaderItem(AdModel::Column::Category, new QStandardItem("Category"));
    model->setHorizontalHeaderItem(AdModel::Column::Description, new QStandardItem("Description"));
    model->setHorizontalHeaderItem(AdModel::Column::DN, new QStandardItem("DN"));

    const auto proxy = new EntryProxyModel(model, this);

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

    column_hidden[AdModel::Column::Name] = false;
    column_hidden[AdModel::Column::Category] = false;
    column_hidden[AdModel::Column::Description] = false;
    update_column_visibility();

    connect(
        containers_widget, &ContainersWidget::selected_changed,
        this, &ContentsWidget::on_containers_selected_changed);
}

void ContentsWidget::on_containers_selected_changed(const QString &dn) {
    model->removeRows(0, model->rowCount());

    // Load head
    QStandardItem *head;
    {
        const QString name = AD()->get_attribute(dn, "name");
        const auto name_item = new QStandardItem(name);
        const auto category_item = new QStandardItem("category");
        const auto description_item = new QStandardItem("descr");
        const auto dn_item = new QStandardItem(dn);
        model->appendRow({name_item, category_item, description_item, dn_item});

        head = name_item;
    }

    // Load children
    QList<QString> children = AD()->load_children(dn);
    for (auto child_dn : children) {
        printf("%s\n", qPrintable(child_dn));
        const QString name = AD()->get_attribute(child_dn, "name");
        const auto name_item = new QStandardItem(name);
        const auto category_item = new QStandardItem("category");
        const auto description_item = new QStandardItem("descr");
        const auto dn_item = new QStandardItem(child_dn);

        head->appendRow({name_item, category_item, description_item, dn_item});
    }

    view->setRootIndex(head->index());

    update_column_visibility();
}
