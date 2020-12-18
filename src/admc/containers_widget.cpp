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

#include "object_model.h"
#include "containers_proxy.h"
#include "advanced_view_proxy.h"
#include "object_context_menu.h"
#include "utils.h"
#include "details_dialog.h"
#include "ad_config.h"
#include "ad_interface.h"
#include "ad_object.h"
#include "filter.h"

#include <QTreeView>
#include <QVBoxLayout>
#include <QDebug>

QStandardItem *make_row(QStandardItem *parent, const AdObject &object);

ContainersWidget::ContainersWidget(ObjectModel *model, QWidget *parent)
: QWidget(parent)
{
    view = new QTreeView(this);
    view->setAcceptDrops(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setRootIsDecorated(true);
    view->setItemsExpandable(true);
    view->setExpandsOnDoubleClick(true);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setDragDropMode(QAbstractItemView::DragDrop);
    view->setAllColumnsShowFocus(true);
    view->setSortingEnabled(true);
    view->sortByColumn(ADCONFIG()->get_column_index(ATTRIBUTE_NAME), Qt::AscendingOrder);

    advanced_view_proxy = new AdvancedViewProxy(this);
    containers_proxy = new ContainersProxy(this);

    containers_proxy->setSourceModel(model);
    advanced_view_proxy->setSourceModel(containers_proxy);
    view->setModel(advanced_view_proxy);

    setup_column_toggle_menu(view, model, {ADCONFIG()->get_column_index(ATTRIBUTE_NAME)});

    // Insert label into layout
    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(view);

    ObjectContextMenu::setup(view, ADCONFIG()->get_column_index(ATTRIBUTE_DN));

    connect(
        view->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &ContainersWidget::on_selection_changed);
};

// Transform selected index into source index and pass it on
// to selected_container_changed() signal
void ContainersWidget::on_selection_changed(const QItemSelection &selected, const QItemSelection &) {
    const QList<QModelIndex> indexes = selected.indexes();
    if (indexes.isEmpty()) {
        return;
    }

    // Convert view's (advanced view proxy) index to source index
    const QModelIndex advanced_view_proxy_index = indexes[0];
    const QModelIndex containers_proxy_index = advanced_view_proxy->mapToSource(advanced_view_proxy_index);
    const QModelIndex source_index = containers_proxy->mapToSource(containers_proxy_index);

    emit selected_changed(source_index);
}

void ContainersWidget::showEvent(QShowEvent *event) {
    resize_columns(view,
    {
        {ADCONFIG()->get_column_index(ATTRIBUTE_NAME), 0.5},
        {ADCONFIG()->get_column_index(ATTRIBUTE_DN), 0.5},
    });
}
