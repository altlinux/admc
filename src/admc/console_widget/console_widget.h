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
 * The central widget of the app through which the user can
 * browse and manipulate objects. Note that this is just a
 * framework for the actual console and it doesn't contain
 * any logic relating to any problem domains. This widget
 * should be wrapped by a parent widget and the parent
 * widget should be the one to provide domain logic by
 * loading data into console widget and determining how it
 * should be displayed. Contains two panes: "scope" and
 * "results". Scope pane contains a tree of items. Each
 * scope item has it's own "results" which are displayed in
 * the results pane when the scope item is selected. Results
 * can contain items that represent children of the scope
 * item in scope tree. Results can also contain items that
 * do not have an equivalent in the scope tree and are
 * associated to the scope item in some other way. For
 * example - a search query, where the scope item represents
 * the query and results represents the results of the
 * search. The way results are displayed can be customized
 * by registering certain types of results views and
 * assigning them to scope items. The user widget of the
 * console widget is responsible for loading scope items and
 * results, creating results views and other things.
 */

#include <QWidget>

class ConsoleWidgetPrivate;
class ResultsView;
class QStandardItem;
class QMenu;
class QAbstractItemView;

enum ConsoleRolePublic {
    ConsoleRole_HasProperties = Qt::UserRole + 18,
    
    // Use this role to set and get item types
    ConsoleRole_Type = Qt::UserRole + 19,

    // NOTE: when implementing custom roles, make sure they do
    // not conflict with console roles, like this:
    //
    // enum YourRole {
    //     YourRole_First = ConsoleRole_LAST + 1,
    //     YourRole_Second = ConsoleRole_LAST + 2,
    //     ... 
    // };
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
    // together. If a scope item is deleted, it’s buddy in
    // results is also deleted and vice versa. When a buddy
    // in results is activated (double-click or
    // select+enter), scope’s current item is changed to
    // it’s scope buddy. If this is the first scope item
    // added to console, then it is set as current scope by
    // default. If you want another scope item at startup,
    // use set_current_scope(). Items returned from these
    // f-ns should be used to set text, icon and your custom
    // data roles. Note that after adding and setting up the
    // first scope item you should call set_current_scope()
    // on it.
    //
    // Arguments:
    // 
    // "results_id" - id of the results description that
    // should've been previously registered by calling
    // register_results().
    //
    // "scope_type" - scope items can be static or dynamic.
    // Static scope items should be loaded once and never
    // change after that. Dynamic scope items should be
    // loaded when item_fetched() signal is emitted for that
    // scope item. Note that dynamic scope items can be
    // fetched again via the refresh_scope() f-n or
    // "Refresh" action of the item menu.
    //
    // "parent" - index of the scope item which will be the
    // parent of created item. Note that results are also
    // "parented" by a scope parent, even though they are
    // not in the scope tree. Pass empty QModelIndex as
    // parent to add a scope item as top level item.
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

    // NOTE: if no type is given, then items of all types
    // will be returned
    QList<QModelIndex> search_scope_by_role(int role, const QVariant &value, const int type = -1) const;
    QList<QModelIndex> search_results_by_role(int role, const QVariant &value, const int type = -1) const;

    QModelIndex get_current_scope_item() const;
    int get_current_results_count() const;

    QStandardItem *get_scope_item(const QModelIndex &scope_index) const;
    QList<QStandardItem *> get_results_row(const QModelIndex &results_index) const;

    QModelIndex get_buddy(const QModelIndex &index) const;
    QModelIndex get_scope_parent(const QModelIndex &index) const;
    bool is_scope_item(const QModelIndex &index) const;
    bool item_was_fetched(const QModelIndex &index) const;
    
    // If index is already scope, returns index, otherwise
    // returns scope buddy, if it exists
    QModelIndex convert_to_scope_index(const QModelIndex &index) const;

    void add_actions_to_action_menu(QMenu *menu);
    void add_actions_to_navigation_menu(QMenu *menu);
    void add_actions_to_view_menu(QMenu *menu);

    // These getters are only for showing/hiding these widgets
    QWidget *get_scope_view() const;
    QWidget *get_description_bar() const;

signals:
    // Emitted when a dynamic scope item is expanded or
    // selected for the first time. User of this widget
    // should connect to this signal and load item's
    // children in the slot using add_item().
    void item_fetched(const QModelIndex &index);

    // Emitted when current scope item changes.
    void current_scope_item_changed(const QModelIndex &index);

    // Emitted while items are dragged to determine whether
    // they can be dropped on target. Set "ok" to true if
    // items can be dropped, false if can't be dropped.
    void items_can_drop(const QList<QModelIndex> &dropped, const QModelIndex &target, bool *ok);

    // Emitted when items are dropped onto target. Modify
    // scope and results in the slot.
    void items_dropped(const QList<QModelIndex> &dropped, const QModelIndex &target);

    // Emitted when a properties dialog is requested via the
    // action menu for a scope or results item.
    void properties_requested();

    // Emitted when item count changes in one of results,
    // due to items getting added or deleted. Note that this
    // can be emitted for a non-current results as well.
    // Useful if you want to display results count in
    // description bar.
    void results_count_changed();

    // Emitted when selected items changes. Note that this
    // is also emitted when focus changes between scope and
    // results panes.
    void selection_changed();

    // Emitted when a context menu is requested from a scope
    // or results view
    void context_menu(const QPoint pos);

private:
    ConsoleWidgetPrivate *d;
};

// Returns true if all given indexes are of given type.
// Returns false for empty lists.
bool indexes_are_of_type(const QList<QModelIndex> &indexes, const int type);

#endif /* CONSOLE_WIDGET_H */
