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

#ifndef RESULTS_VIEW_H
#define RESULTS_VIEW_H

/**
 * This view is really a container for multiple view types:
 * icons, list and details (tree view). Wraps some signals
 * and functions for child views and provides a way to
 * switch between views.
 */

#include <QWidget>
#include <QHash>

class QTreeView;
class QListView;
class QCheckBox;
class ResultsDescription;
class QStackedWidget;
class QAbstractItemView;
class QAbstractItemModel;

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
    void set_view_type(const ResultsViewType type);
    QAbstractItemView *current_view() const;
    QTreeView *detail_view() const;

signals:
    void activated(const QModelIndex &index);
    void context_menu(const QPoint pos);

private:
    QStackedWidget *stacked_widget;
    QHash<ResultsViewType, QAbstractItemView *> views;
    QAbstractItemView *m_current_view;
    QTreeView *m_detail_view;
};

#endif /* RESULTS_VIEW_H */
