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

#ifndef CONSOLE_WIDGET_P_H
#define CONSOLE_WIDGET_P_H

/**
 * Private header for ConsoleWidget.
 */

#include "console_widget/results_view.h"
#include "console_widget/results_description.h"
#include "console_widget/console_widget.h"

#include <QCoreApplication>
#include <QSet>

class QLabel;
class QStackedWidget;
class ScopeProxyModel;
class ConsoleDragModel;
class QStandardItemModel;
class ConsoleWidget;
class QMenu;
class QSplitter;
class ConsoleImpl;

enum ConsoleRole {
    // Determines whether scope item was fetched
    ConsoleRole_WasFetched = Qt::UserRole + 1,
    
    ConsoleRole_IsScope = Qt::UserRole + 3,
    
    // NOTE: don't go above ConsoleRole_Type and
    // ConsoleRole_LAST (defined in public header)

    // NOTE: these roles are "public" defined in public header
    // ConsoleRole_Type = Qt::UserRole + 19,
    // ConsoleRole_LAST = Qt::UserRole + 20
};

class ConsoleWidgetPrivate : public QObject {
    Q_OBJECT

public:
    ConsoleWidget *q;

    QTreeView *scope_view;
    ConsoleDragModel *model;
    ScopeProxyModel *scope_proxy_model;
    QWidget *description_bar;
    QLabel *description_bar_left;
    QLabel *description_bar_right;
    QAbstractItemView *focused_view;
    QStackedWidget *results_stacked_widget;
    QHash<int, ResultsDescription> results_descriptions;
    ResultsDescription default_results;
    QSplitter *splitter;
    QHash<int, ConsoleImpl *> impl_map;
    QList<QWidget *> registered_results_widget_list;
    QList<ResultsView *> registered_results_view_list;

    QAction *properties_action;
    QAction *refresh_current_scope_action;

    QAction *navigate_up_action;
    QAction *navigate_back_action;
    QAction *navigate_forward_action;

    QAction *set_results_to_icons_action;
    QAction *set_results_to_list_action;
    QAction *set_results_to_detail_action;
    QAction *customize_columns_action;
    QAction *toggle_console_tree_action;
    QAction *toggle_description_bar_action;

    QList<QPersistentModelIndex> dropped_list;
    QSet<int> dropped_type_list;

    // NOTE: target history stores target items' id's.
    // History lists are in order of ascending time.
    // ... past.last() -> current -> future.first() ...
    QList<QPersistentModelIndex> targets_past;
    QList<QPersistentModelIndex> targets_future;

    QMenu *action_menu;
    QHash<StandardAction, QAction *> standard_action_map;

    ConsoleWidgetPrivate(ConsoleWidget *q_arg);

    void open_action_menu_as_context_menu(const QPoint pos);
    void on_scope_expanded(const QModelIndex &index);
    void on_results_activated(const QModelIndex &index);
    void on_action_menu_show();
    void on_context_menu(const QPoint pos);
    void update_navigation_actions();
    void update_view_actions();
    void on_current_scope_item_changed(const QModelIndex &current, const QModelIndex &);
    void on_scope_items_about_to_be_removed(const QModelIndex &parent, int first, int last);
    void on_focus_changed(QWidget *old, QWidget *now);
    void refresh_current_scope();
    void customize_columns();
    void navigate_up();
    void navigate_back();
    void navigate_forward();
    void start_drag(const QList<QPersistentModelIndex> &dropped_list_arg);
    bool can_drop(const QModelIndex &target);
    void drop(const QModelIndex &target);
    void set_results_to_icons();
    void set_results_to_list();
    void set_results_to_detail();
    void set_results_to_type(const ResultsViewType type);
    void on_toggle_console_tree();
    void on_toggle_description_bar();
    void fetch_scope(const QModelIndex &index);
    const ResultsDescription get_current_results() const;
    ConsoleImpl *get_impl(const QModelIndex &index) const;
    ResultsDescription get_results(const QModelIndex &index) const;
    void update_description();
    void on_standard_action(const StandardAction action_enum);
    QList<QModelIndex> get_all_selected_items() const;
    QSet<int> get_selected_types() const;

signals:
    void context_menu(const QPoint pos);
};

#endif /* CONSOLE_WIDGET_P_H */
