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
#include "object_context_menu.h"
#include "utils.h"
#include "settings.h"
#include "details_dialog.h"
#include "ad_config.h"
#include "ad_interface.h"
#include "ad_object.h"
#include "filter.h"
#include "object_model.h"

#include <QTreeView>
#include <QIcon>
#include <QSet>
#include <QStack>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QDebug>

QStandardItem *make_row(QStandardItem *parent, const AdObject &object);

ContainersWidget::ContainersWidget(QWidget *parent)
: QWidget(parent)
{
    model = new ObjectModel(ObjectModel::Column::COUNT, ObjectModel::Column::DN, this);

    proxy = new ContainersFilterProxy(this);

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
    view->sortByColumn(ObjectModel::Column::Name, Qt::AscendingOrder);

    proxy->setSourceModel(model);
    view->setModel(proxy);

    setup_column_toggle_menu(view, model, {ObjectModel::Column::Name});

    // Insert label into layout
    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(view);

    connect(
        view->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &ContainersWidget::on_selection_changed);

    QObject::connect(
        view, &QWidget::customContextMenuRequested,
        this, &ContainersWidget::on_context_menu);
};

// Transform selected index into source index and pass it on
// to selected_container_changed() signal
void ContainersWidget::on_selection_changed(const QItemSelection &selected, const QItemSelection &) {
    const QList<QModelIndex> indexes = selected.indexes();
    if (indexes.isEmpty()) {
        return;
    }

    // Fetch selected object to remove expander if it has not children
    const QModelIndex index_proxy = indexes[0];
    const QModelIndex index = proxy->mapToSource(index_proxy);
    model->fetchMore(index);

    const QString dn = get_dn_from_index(index, ObjectModel::Column::DN);

    emit selected_changed(dn);
}

void ContainersWidget::on_context_menu(const QPoint pos) {
    const QString dn = get_dn_from_pos(pos, view, ObjectModel::Column::DN);
    if (dn.isEmpty()) {
        return;
    }

    ObjectContextMenu context_menu(dn, view);
    exec_menu_from_view(&context_menu, view, pos);
}

void ContainersWidget::showEvent(QShowEvent *event) {
    resize_columns(view,
    {
        {ObjectModel::Column::Name, 0.5},
        {ObjectModel::Column::DN, 0.5},
    });
}

ContainersFilterProxy::ContainersFilterProxy(QObject *parent)
: QSortFilterProxyModel(parent) {
    const BoolSettingSignal *show_non_containers_signal = SETTINGS()->get_bool_signal(BoolSetting_ShowNonContainersInContainersTree);
    connect(
        show_non_containers_signal, &BoolSettingSignal::changed,
        this, &ContainersFilterProxy::on_show_non_containers);
    on_show_non_containers();
}

void ContainersFilterProxy::on_show_non_containers() {
    // NOTE: get the setting here and save it instead of in
    // filterAcceptsRow() for better perfomance
    show_non_containers = SETTINGS()->get_bool(BoolSetting_ShowNonContainersInContainersTree);
    invalidateFilter();
}

bool ContainersFilterProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    if (show_non_containers) {
        return true;
    } else {
        const QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
        const bool is_container = index.data(ObjectModel::Roles::IsContainer).toBool();

        return is_container;
    }
}
