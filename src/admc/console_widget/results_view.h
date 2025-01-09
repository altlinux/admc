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

#ifndef RESULTS_VIEW_H
#define RESULTS_VIEW_H

/**
 * This view is really a container for multiple view types:
 * icons, list and details (tree view). Wraps some signals
 * and functions for child views and provides a way to
 * switch between views.
 */

#include <QWidget>

class QTreeView;
class QStackedWidget;
class QAbstractItemView;
class QAbstractItemModel;
class QSortFilterProxyModel;
class QModelIndex;

enum ResultsViewType {
    ResultsViewType_Icons,
    ResultsViewType_List,
    ResultsViewType_Detail,
};

class ResultsView final : public QWidget {
    Q_OBJECT

public:
    ResultsView(QWidget *parent);

    void set_model(QAbstractItemModel *model);
    void set_parent(const QModelIndex &source_index);
    void set_view_type(const ResultsViewType type);
    QAbstractItemView *current_view() const;
    ResultsViewType current_view_type() const;
    QTreeView *detail_view() const;

    // Returns selected indexes in current view. If current
    // view type is detail (QTreeView), then returns one
    // index per selected row at column 0
    QList<QModelIndex> get_selected_indexes() const;

    QVariant save_state() const;

    // NOTE: if state is empty, default columns are shown
    // and others are hidden
    void restore_state(const QVariant &state, const QList<int> &default_columns);

    void set_drag_drop_enabled(const bool enabled);
    void set_drag_drop_internal();

    void set_row_hidden(int row, bool hidden);

signals:
    void activated(const QModelIndex &index);
    void context_menu(const QPoint pos);
    void selection_changed();

private:
    QStackedWidget *stacked_widget;
    QHash<ResultsViewType, QAbstractItemView *> views;
    QSortFilterProxyModel *proxy_model;
    ResultsViewType m_current_view_type;
    QTreeView *m_detail_view;

    void on_item_activated(const QModelIndex &index);
};

#endif /* RESULTS_VIEW_H */
