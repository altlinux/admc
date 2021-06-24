/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

#include "console_widget/results_view.h"

#include <QHeaderView>
#include <QListView>
#include <QSortFilterProxyModel>
#include <QStackedWidget>
#include <QTreeView>
#include <QVBoxLayout>

ResultsView::ResultsView(QWidget *parent)
: QWidget(parent) {
    // Setup child views
    m_detail_view = new QTreeView();
    m_detail_view->sortByColumn(0, Qt::AscendingOrder);
    m_detail_view->setSortingEnabled(true);
    m_detail_view->header()->setDefaultSectionSize(200);
    m_detail_view->setRootIsDecorated(false);

    auto list_view = new QListView();
    list_view->setViewMode(QListView::ListMode);

    auto icons_view = new QListView();
    icons_view->setViewMode(QListView::IconMode);
    icons_view->setGridSize(QSize(100, 100));
    icons_view->setIconSize(QSize(64, 64));

    // Put child views into map
    views[ResultsViewType_Icons] = icons_view;
    views[ResultsViewType_List] = list_view;
    views[ResultsViewType_Detail] = m_detail_view;
    
    proxy_model = new QSortFilterProxyModel(this);
    proxy_model->setSortCaseSensitivity(Qt::CaseInsensitive);

    // Perform common setup on child views
    for (auto view : views.values()) {
        view->setEditTriggers(QAbstractItemView::NoEditTriggers);
        view->setContextMenuPolicy(Qt::CustomContextMenu);
        view->setSelectionMode(QAbstractItemView::ExtendedSelection);
        view->setDragDropMode(QAbstractItemView::DragDrop);
        view->setDragDropOverwriteMode(true);

        // NOTE: a proxy is model is inserted between
        // results views and results models for more
        // efficient sorting. If results views and models
        // are connected directly, deletion of results
        // models becomes extremely slow.
        view->setModel(proxy_model);
    }

    stacked_widget = new QStackedWidget();

    for (auto view : views.values()) {
        stacked_widget->addWidget(view);
    }

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);
    layout->addWidget(stacked_widget);

    for (auto view : views.values()) {
        connect(
            view, &QTreeView::activated,
            [=](const QModelIndex &proxy_index) {
                const QModelIndex &source_index = proxy_model->mapToSource(proxy_index);

                emit activated(source_index);
            });
        connect(
            view, &QWidget::customContextMenuRequested,
            this, &ResultsView::context_menu);

        // NOTE: this must be called after view model is set
        // to proxy. Before that selectionModel() doesn't
        // exist
        connect(
            view->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ResultsView::selection_changed);
        connect(
            view->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &ResultsView::current_changed);
    }

    set_view_type(ResultsViewType_Detail);
}

void ResultsView::set_model(QAbstractItemModel *model) {
    proxy_model->setSourceModel(model);
}

void ResultsView::set_parent(const QModelIndex &source_index) {
    const QModelIndex proxy_index = proxy_model->mapFromSource(source_index);
    for (auto view : views.values()) {
        view->setRootIndex(proxy_index);
    }
}

void ResultsView::set_view_type(const ResultsViewType type) {
    QAbstractItemView *view = views[type];

    // NOTE: current view must be changed before
    // setCurrentWidget(), because that causes a focus
    // change
    m_current_view_type = type;

    stacked_widget->setCurrentWidget(view);

    // Clear selection since view type changed
    for (auto the_view : views.values()) {
        the_view->clearSelection();
    }
}

QAbstractItemView *ResultsView::current_view() const {
    return views[m_current_view_type];
}

ResultsViewType ResultsView::current_view_type() const {
    return m_current_view_type;
}

QTreeView *ResultsView::detail_view() const {
    return m_detail_view;
}

QList<QModelIndex> ResultsView::get_selected_indexes() const {
    const QList<QModelIndex> proxy_indexes = [this]() {
        QItemSelectionModel *selection_model = current_view()->selectionModel();

        if (current_view_type() == ResultsViewType_Detail) {
            return selection_model->selectedRows();
        } else {
            return selection_model->selectedIndexes();
        }
    }();

    // NOTE: need to map from proxy to source indexes if
    // focused view is results
    QList<QModelIndex> source_indexes;
    for (const QModelIndex &index : proxy_indexes) {
        const QModelIndex source_index = proxy_model->mapToSource(index);
        source_indexes.append(source_index);
    }

    return source_indexes;
}
