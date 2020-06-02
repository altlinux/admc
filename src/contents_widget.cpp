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
#include "ad_interface.h"
#include "ad_model.h"
#include "ad_proxy_model.h"

#include <QTreeView>
#include <QLabel>
#include <QLayout>

ContentsWidget::ContentsWidget(AdModel* model)
: EntryWidget(AdModel::Column::COUNT, AdModel::Column::DN)
{   
    proxy = new AdProxyModel(model, this);

    view->setAcceptDrops(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setDragDropMode(QAbstractItemView::DragDrop);
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
    column_hidden[AdModel::Column::DN] = true;
    update_column_visibility();
};

// Both contents and containers share the same source model, but have different proxy's to it
// So need to map from containers proxy to source then back to proxy of contents
void ContentsWidget::on_selected_container_changed(const QModelIndex &source_index) {
    QModelIndex index = proxy->mapFromSource(source_index);
    view->setRootIndex(index);

    // NOTE: have to hide columns after setRootIndex
    update_column_visibility();
}
