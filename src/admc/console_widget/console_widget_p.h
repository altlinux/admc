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

#ifndef CONSOLE_WIDGET_P_H
#define CONSOLE_WIDGET_P_H

/**
 * Private header for ConsoleWidget.
 */

#include "console_widget/console_widget.h"
#include "console_widget/results_view.h"

#include <QCoreApplication>
#include <QSet>
#include <QPersistentModelIndex>

class QLabel;
class QStackedWidget;
class ScopeProxyModel;
class ConsoleDragModel;
class QStandardItemModel;
class ConsoleWidget;
class QSplitter;
class ConsoleImpl;

enum ConsoleRole {
    // Determines whether scope item was fetched
    ConsoleRole_WasFetched = Qt::UserRole + 1,
    ConsoleRole_SortIndex = Qt::UserRole + 2,

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
    QSplitter *splitter;
    QHash<int, ConsoleImpl *> impl_map;
    QWidget *default_results_widget;
    ConsoleImpl *default_impl;

    ConsoleWidgetActions actions;

    QList<QPersistentModelIndex> dropped_list;
    QSet<int> dropped_type_list;

    // NOTE: target history stores target items' id's.
    // History lists are in order of ascending time.
    // ... past.last() -> current -> future.first() ...
    QList<QPersistentModelIndex> targets_past;
    QList<QPersistentModelIndex> targets_future;

    QHash<StandardAction, QAction *> standard_action_map;

    QPersistentModelIndex domain_info_index;


    ConsoleWidgetPrivate(ConsoleWidget *q_arg);

    void update_navigation_actions();
    void update_view_actions();
    void start_drag(const QList<QPersistentModelIndex> &dropped_list_arg);
    bool can_drop(const QModelIndex &target);
    void drop(const QModelIndex &target);
    void set_results_to_type(const ResultsViewType type);
    void fetch_scope(const QModelIndex &index);
    ConsoleImpl *get_current_scope_impl() const;
    ConsoleImpl *get_impl(const QModelIndex &index) const;
    void update_description();
    QList<QModelIndex> get_all_selected_items() const;
    QList<QAction *> get_custom_action_list() const;
    void open_context_menu(const QPoint &global_pos);
    void add_actions(QMenu *menu);
    bool update_actions();

public slots:
    void on_current_scope_item_changed(const QModelIndex &current, const QModelIndex &);
    void on_scope_items_about_to_be_removed(const QModelIndex &parent, int first, int last);
    void on_focus_changed(QWidget *old, QWidget *now);
    void on_refresh();
    void on_customize_columns();
    void on_navigate_up();
    void on_navigate_back();
    void on_navigate_forward();
    void on_view_icons();
    void on_view_list();
    void on_view_detail();
    void on_toggle_console_tree();
    void on_toggle_description_bar();
    void on_standard_action(const StandardAction action_enum);
    void on_results_context_menu(const QPoint &pos);
    void on_scope_context_menu(const QPoint &pos);
    void on_scope_expanded(const QModelIndex &index);
    void on_results_activated(const QModelIndex &index);
};

#endif /* CONSOLE_WIDGET_P_H */
