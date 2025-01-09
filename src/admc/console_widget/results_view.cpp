/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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
        view->setDragDropOverwriteMode(true);

        // NOTE: a proxy is model is inserted between
        // results views and results models for more
        // efficient sorting. If results views and models
        // are connected directly, deletion of results
        // models becomes extremely slow.
        view->setModel(proxy_model);

        connect(
            view->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ResultsView::selection_changed);
    }

    set_drag_drop_enabled(true);

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
            this, &ResultsView::on_item_activated);
        connect(
            view, &QWidget::customContextMenuRequested,
            this, &ResultsView::context_menu);
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

QVariant ResultsView::save_state() const {
    QHash<QString, QVariant> state;

    state["header"] = detail_view()->header()->saveState();
    state["view_type"] = m_current_view_type;

    return QVariant(state);
}

void ResultsView::restore_state(const QVariant &state_variant, const QList<int> &default_columns) {
    QHeaderView *header = detail_view()->header();

    if (state_variant.isValid()) {
        const QHash<QString, QVariant> state = state_variant.toHash();

        const QByteArray header_state = state["header"].toByteArray();
        header->restoreState(header_state);

        const ResultsViewType view_type = (ResultsViewType) state["view_type"].toInt();
        set_view_type(view_type);
    } else {
        // Set default state in absence of saved state
        for (int i = 0; i < header->count(); i++) {
            const bool hidden = !default_columns.contains(i);
            header->setSectionHidden(i, hidden);
        }

        // NOTE: it is important to set default sort
        // state here by modifying header of detail
        // view, NOT the detail view itself. If we
        // modify detail view only, then by default
        // header will be in unsorted state, that state
        // will get saved and overwrite sorted state
        // the next time the app is run. Also, setting
        // sort state for header is enough, because it
        // propagates to detail view itself and
        // icons/list views, because they share
        // model/proxy
        header->setSortIndicator(0, Qt::AscendingOrder);
    }
}

void ResultsView::on_item_activated(const QModelIndex &proxy_index) {
    const QModelIndex &source_index = proxy_model->mapToSource(proxy_index);

    emit activated(source_index);
}

void ResultsView::set_drag_drop_enabled(const bool enabled) {
    const QAbstractItemView::DragDropMode mode = [&]() {
        if (enabled) {
            return QAbstractItemView::DragDrop;
        } else {
            return QAbstractItemView::NoDragDrop;
        }
    }();

    for (auto view : views.values()) {
        view->setDragDropMode(mode);
    }
}

void ResultsView::set_drag_drop_internal() {
    for (auto view : views.values()) {
        view->setDragDropMode(QAbstractItemView::InternalMove);
    }
}

void ResultsView::set_row_hidden(int row, bool hidden)
{
    m_detail_view->setRowHidden(row, QModelIndex(), hidden);
}
