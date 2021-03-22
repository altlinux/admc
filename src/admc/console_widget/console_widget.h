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

#ifndef CONSOLE_WIDGET_H
#define CONSOLE_WIDGET_H

/**
 * The central widget of the app through which user can
 * browse and manipulate objects. Contains two panes:
 * "scope" and "results". Scope pane contains a tree of
 * items. Scope items are loaded dynamically as the tree is
 * traversed by the user. When a scope item is first created
 * it is in "unfetched" state. When that scope item is
 * expanded or selected, it is "fetched". Fetching loads
 * scope item's children in scope tree as well as the
 * associated results. Each scope item has it's own
 * "results" which are displayed in the results pane when
 * the scope item is selected. Results can contain items
 * that represent children of the scope item in scope tree.
 * Results can also contain items that do not have an
 * equivalent in the scope tree and are associate to the
 * scope item in some other way, for example - results of a
 * search query, where the scope item represents the query.
 * Each scope item can have it's own results view with a
 * specific set of columns. Implements a navigation system
 * where a history of selected scope items is stored and you
 * can navigate through it using navigation actions. The
 * user widget of the console widget is responsible for
 * loading scope items and results, creating results views
 * and other things.
 */

#include <QWidget>

class ConsoleWidgetPrivate;
class ResultsView;
class QStandardItem;
class QMenu;
class QAbstractItemView;

// NOTE: when implementing custom roles, make sure they do
// not conflict with console roles, like this:
//
// enum YourRole {
//     YourRole_First = ConsoleRole_LAST + 1,
//     YourRole_Second = ConsoleRole_LAST + 2,
//     ... 
// };
enum ConsoleRoleLast {
    ConsoleRole_LAST = Qt::UserRole + 20,
};

enum ScopeNodeType {
    ScopeNodeType_Static,
    ScopeNodeType_Dynamic,
};

class ConsoleWidget final : public QWidget {
Q_OBJECT

public:        
    ConsoleWidget(QWidget *parent = nullptr);

    // These f-ns are for adding items to console. There are
    // two types of items "scope" and "results". Scope is
    // composed of a single item, while results is a row of
    // multiple items. Scope and results items can be
    // "buddies". This is for cases where a results item
    // also represents a scope item. Buddies are deleted
    // together. If a scope item is deleted, it’s results
    // buddy is also deleted and vice versa. When a buddy in
    // results is activated (double-click or select+enter),
    // scope’s current item is changed to it’s scope buddy.
    //
    // Arguments:
    // 
    // "results_id" - id of the results description that
    // should've been previously registered by calling
    // register_results().
    //
    // "scope_type" - scope items can be static or dynamic.
    // Static scope items should be loaded at startup and
    // never change after that. Dynamic scope items should
    // be loaded when item_fetched() signal is emitted for
    // that scope item. Note that dynamic scope items can be
    // fetched again via the refresh_scope() f-n or
    // "Refresh" action of the item menu.
    //
    // "parent" - index of the scope item which will be the
    // parent of created item. Note that results are also
    // "parented" by a scope parent, even though they are
    // not in the scope tree. Pass empty QModelIndex as
    // parent to add a scope item as top level item.
    //
    // Items returned from these f-ns should be used to set
    // text, icon and your custom data roles.

    //Pass empty QModelIndex as
    // "parent" to add scope item as top level item. Results
    // id is the id of the results view that must have been
    // received from a previous register_results()
    // call. Scope items can be dynamic, see comment about
    // ConsoleRole_ScopeIsDynamic for more info.
    // Add new row of items to results of given scope item.
    // Use the return row to fill it with your data.
    //
    // If this is the first scope item added to console,
    // then it is set as current scope by default. If you
    // want another scope item at startup, use
    // set_current_scope().
    QStandardItem *add_scope_item(const int results_id, const ScopeNodeType scope_type, const QModelIndex &parent);
    QList<QStandardItem *> add_results_row(const QModelIndex &scope_parent);
    void add_buddy_scope_and_results(const int results_id, const ScopeNodeType scope_type, const QModelIndex &scope_parent, QStandardItem **scope_arg, QList<QStandardItem *> *results_arg);

    // Sets whether a given item should have "Properties"
    // action in it's menu, which opens the Properties
    // dialog.
    void set_has_properties(const QModelIndex &index, const bool has_properties);

    // Deletes a scope/results item. If this item has a
    // buddy, that buddy is also deleted.
    void delete_item(const QModelIndex &index);

    // Sets current scope item in the scope tree
    void set_current_scope(const QModelIndex &index);

    // Clears children of this scope item, then emits
    // item_fetched() signal so that the scope item can be
    // reloaded.
    void refresh_scope(const QModelIndex &index);

    // Register results to be used later for scope items.
    // Results can be just a widget, a tree view or a widget
    // that contains a tree view. Returns the unique id
    // assigned to this results view, which should be used
    // when creating scope items. Note that if results is
    // just a widget, then you can't add or get results
    // rows.
    int register_results(QWidget *widget);
    int register_results(ResultsView *view, const QList<QString> &column_labels, const QList<int> &default_columns);
    int register_results(QWidget *widget, ResultsView *view, const QList<QString> &column_labels, const QList<int> &default_columns);

    // Sorts scope items by name. Note that this can affect
    // performance negatively if overused. For example, when
    // adding multiple scope items, try to call this once
    // after all items are added. If you were to call this
    // after each item is added the whole process would be
    // slowed down.
    void sort_scope();

    void set_description_bar_text(const QString &text);

    // Gets selected item(s) from currently focused view,
    // which could be scope or results.
    QList<QModelIndex> get_selected_items() const;

    QList<QModelIndex> search_scope_by_role(int role, const QVariant &value) const;

    QModelIndex get_current_scope_item() const;
    int get_current_results_count() const;

    QStandardItem *get_scope_item(const QModelIndex &scope_index) const;
    QList<QStandardItem *> get_results_row(const QModelIndex &results_index) const;

    QModelIndex get_buddy(const QModelIndex &index) const;
    bool is_scope_item(const QModelIndex &index) const;

    // These getters are only for showing/hiding these widgets
    QWidget *get_scope_view() const;
    QWidget *get_description_bar() const;

    // Insert these into the menubar of your app
    QMenu *get_action_menu() const;
    QMenu *get_navigation_menu() const;
    QMenu *get_view_menu() const;

signals:
    // Emitted when a scope item is expanded or selected for
    // the first time. User of this widget should connect to
    // this signal and load item's children in the slot
    // using add_item().
    void item_fetched(const QModelIndex &index);

    void current_scope_item_changed(const QModelIndex &index);

    // Emitted when action menu is about to open. Action
    // menu can be opened both from menubar or as a context
    // menu. Connect to this signal and add actions to menu
    // in the slot. Use get_selected_items() to get the
    // items for which this menu should add actions.
    void action_menu_about_to_open(QMenu *menu);

    void view_menu_about_to_open(QMenu *menu);

    // Emitted while items are dragged to determine whether
    // they can be dropped on target. Set "ok" to true if
    // items can be dropped, false if can't be dropped.
    void items_can_drop(const QList<QModelIndex> &dropped, const QModelIndex &target, bool *ok);

    // Emitted when items are dropped onto target. Modify
    // scope and results in the slot.
    void items_dropped(const QList<QModelIndex> &dropped, const QModelIndex &target);

    void properties_requested();

    // Emitted when item count changes in one of results,
    // due to items getting added or deleted. Note that this
    // can be emitted for a non-current results as well.
    // Useful if you want to display results count in
    // description bar.
    void results_count_changed();

private:
    ConsoleWidgetPrivate *d;
};

#endif /* CONSOLE_WIDGET_H */
