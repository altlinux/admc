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

#ifndef CONSOLE_WIDGET_P_H
#define CONSOLE_WIDGET_P_H

/**
 * Private header for ConsoleWidget.
 */

#include "console_widget/results_view.h"

class QLabel;
class QStackedWidget;
class ResultsDescription;
class ScopeModel;
class QStandardItemModel;
class QSortFilterProxyModel;
class ConsoleDragModel;
class ConsoleWidget;
class QMenu;

enum ConsoleRole {
    // Determines whether scope item was fetched
    ConsoleRole_WasFetched = Qt::UserRole + 1,

    // Id of results view that should be used for this for
    // this scope item. Doesn't apply to results items.
    ConsoleRole_ResultsId = Qt::UserRole + 2,
    
    ConsoleRole_Buddy = Qt::UserRole + 3,

    // Scope item parent of a results item. Doesn't apply
    // to scope items.
    ConsoleRole_ScopeParent = Qt::UserRole + 4,

    ConsoleRole_IsScope = Qt::UserRole + 5,

    // Determines whether scope is dynamic. 
    ConsoleRole_ScopeIsDynamic = Qt::UserRole + 6,

    // NOTE: don't go above ConsoleRole_LAST (defined in
    // public header)
    // ConsoleRole_LAST = Qt::UserRole + 20
};

class ConsoleWidgetPrivate : public QObject {

public:
    ConsoleWidget *q;

    QTreeView *scope_view;
    ScopeModel *scope_model;
    QLabel *description_bar;
    QAbstractItemView *focused_view;
    QSortFilterProxyModel *results_proxy_model;
    QStackedWidget *results_stacked_widget;
    QHash<int, ResultsDescription> results_descriptions;
    QHash<QPersistentModelIndex, QStandardItemModel *> results_models;

    QMenu *action_menu;
    QMenu *navigation_menu;
    QMenu *view_menu;

    ConsoleWidgetPrivate(ConsoleWidget *q_arg);

    QStandardItemModel *get_results_model_for_scope_item(const QModelIndex &index) const;
    void open_action_menu_as_context_menu(const QPoint pos);
    void connect_to_drag_model(ConsoleDragModel *model);
    void on_results_activated(const QModelIndex &index);

private:
    QList<QModelIndex> dropped;

    QAction *set_results_to_icons_action;
    QAction *set_results_to_list_action;
    QAction *set_results_to_detail_action;
    QAction *properties_action;
    QAction *navigate_up_action;
    QAction *navigate_back_action;
    QAction *navigate_forward_action;
    QAction *refresh_action;
    QAction *customize_columns_action;

    // NOTE: target history stores target items' id's.
    // History lists are in order of ascending time.
    // ... past.last() -> current -> future.first() ...
    QList<QPersistentModelIndex> targets_past;
    QList<QPersistentModelIndex> targets_future;

    void navigate_up();
    void navigate_back();
    void navigate_forward();
    void update_navigation_actions();
    void on_current_scope_item_changed(const QModelIndex &current, const QModelIndex &);
    void on_scope_items_about_to_be_removed(const QModelIndex &parent, int first, int last);
    void fetch_scope(const QModelIndex &index);
    void refresh();
    void customize_columns();
    void set_results_to_icons();
    void set_results_to_list();
    void set_results_to_detail();
    void on_focus_changed(QWidget *old, QWidget *now);
        void on_action_menu_show();
    void on_view_menu_show();
    void add_actions_to_action_menu(QMenu *menu);
    void on_start_drag(const QList<QModelIndex> &dropped);
    void on_can_drop(const QModelIndex &target, bool *ok);
    void on_drop(const QModelIndex &target);
    void set_results_to_type(const ResultsViewType type);
    const ResultsDescription get_current_results() const;
};

#endif /* CONSOLE_WIDGET_P_H */
