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

#include "console_widget/results_view.h"

#include <QTreeView>
#include <QListView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QStackedWidget>

ResultsView::ResultsView(QWidget *parent)
: QWidget(parent)
{
    // Setup child views
    m_detail_view = new QTreeView();
    QHeaderView *detail_header = m_detail_view->header();
    detail_header->setSectionsMovable(true);
    // TODO: these sorting f-ns are  only available for
    // tree, what about others? maybe have to sort the model
    m_detail_view->sortByColumn(0, Qt::AscendingOrder);
    m_detail_view->setSortingEnabled(true);

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

    // Perform common setup on child views
    for (auto view : views.values()) {
        view->setEditTriggers(QAbstractItemView::NoEditTriggers);
        view->setContextMenuPolicy(Qt::CustomContextMenu);
        view->setSelectionMode(QAbstractItemView::ExtendedSelection);
        view->setDragDropMode(QAbstractItemView::DragDrop);
        view->setDragDropOverwriteMode(true);
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
            this, &ResultsView::activated);
        connect(
            view, &QWidget::customContextMenuRequested,
            this, &ResultsView::context_menu);
    }

    set_view_type(ResultsViewType_Detail);
}

void ResultsView::set_model(QAbstractItemModel *model) {
    for (auto view : views.values()) {
        view->setModel(model);


        // NOTE: selection model doesn't exist until view's
        // model is set, so have to connect here
        connect(
            view->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ResultsView::selection_changed);
        connect(
            view->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &ResultsView::current_changed);
    }
}

void ResultsView::set_view_type(const ResultsViewType type) {
    QAbstractItemView *view = views[type];

    // NOTE: m_current_view must be changed before
    // setCurrentWidget(), because that causes a focus
    // change
    m_current_view = view;

    stacked_widget->setCurrentWidget(view);

    // Clear selection since view type changed
    for (auto the_view : views.values()) {
        the_view->clearSelection();
    }
}

QAbstractItemView *ResultsView::current_view() const {
    return m_current_view;
}

QTreeView *ResultsView::detail_view() const {
    return m_detail_view;
}
