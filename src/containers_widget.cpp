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

#include "containers_widget.h"
#include "ad_model.h"
#include "ad_proxy_model.h"

#include <QTreeView>
#include <QLabel>

ContainersWidget::ContainersWidget(AdModel *model)
: EntryWidget(AdModel::Column::COUNT, AdModel::Column::DN)
{
    proxy = new AdProxyModel(model, this);
    proxy->only_show_containers = true;
    
    view->setAcceptDrops(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setDragDropMode(QAbstractItemView::DragDrop);
    view->setRootIsDecorated(true);
    view->setItemsExpandable(true);
    view->setExpandsOnDoubleClick(true);
    view->setModel(proxy);

    column_hidden[AdModel::Column::Name] = false;
    column_hidden[AdModel::Column::Category] = true;
    column_hidden[AdModel::Column::Description] = true;
    column_hidden[AdModel::Column::DN] = true;
    update_column_visibility();

    label->setText("Containers");

    connect(
        view->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &ContainersWidget::on_selection_changed);
};

void ContainersWidget::on_selection_changed(const QItemSelection &selected, const QItemSelection &) {
    // Transform selected index into source index and pass it on
    // to selected_container_changed() signal
    const QList<QModelIndex> indexes = selected.indexes();

    if (indexes.size() > 0) {
        QModelIndex index = indexes[0];
        QModelIndex source_index = proxy->mapToSource(index);

        emit selected_container_changed(source_index);
    }
}
