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
 * can contain children of the scope item. Results may also
 * display a custom widget. The way results are displayed
 * can be customized by registering certain types of results
 * views and assigning them to scope items. The user widget
 * of the console widget is responsible for loading items,
 * creating results views and other things.
 */

#include <QWidget>

class ConsoleWidgetPrivate;
class ResultsView;
class QStandardItem;
class QMenu;
class QAbstractItemView;
class QStandardItemModel;
class QToolBar;
class ConsoleImpl;
class ConsoleDragModel;

enum ConsoleRolePublic {
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

enum StandardAction {
    StandardAction_Copy,
    StandardAction_Cut,
    StandardAction_Rename,
    StandardAction_Delete,
    StandardAction_Paste,
    StandardAction_Print,
    StandardAction_Refresh,
    StandardAction_Properties,
    
    StandardAction_COUNT,
};

class ConsoleWidget final : public QWidget {
    Q_OBJECT

public:
    ConsoleWidget(QWidget *parent = nullptr);

    // Register results to be used later for scope items.
    // Must be done BEFORE adding items. Results can be just
    // a widget, a tree view or a widget that contains a
    // tree view. Returns the unique id assigned to this
    // results view, which should be used when creating
    // scope items. Note that if results is just a widget,
    // then you can't add or get results rows. Note that
    // call order is important for correct state restoration
    // so register your results in the same order every
    // time.
    void register_results(const int type, QWidget *widget);
    void register_results(const int type, ResultsView *view, const QList<QString> &column_labels, const QList<int> &default_columns);
    void register_results(const int type, QWidget *widget, ResultsView *view, const QList<QString> &column_labels, const QList<int> &default_columns);

    // NOTE: you must register types before adding items
    void register_impl(const int type, ConsoleImpl *impl);

    // These f-ns are for adding items to console. Items
    // returned from these f-ns should be used to set text,
    // icon and your custom data roles. add_scope_item()
    // adds an item that is shown both in scope and results.
    // add_results_item() adds an item that is shown only in
    // results.
    //
    // Arguments:
    //
    // "scope_type" - scope items can be static or dynamic.
    // Static scope items should be loaded once and never
    // change after that. Dynamic scope items will trigger a
    // fetch() call on their assigned ConsoleImpl. Note that
    // dynamic scope items can be fetched again via the
    // refresh_scope() f-n or "Refresh" action of the item
    // menu.
    QList<QStandardItem *> add_scope_item(const int type, const ScopeNodeType scope_type, const QModelIndex &parent);
    QList<QStandardItem *> add_results_item(const int type, const QModelIndex &parent);

    // Deletes an item and all of it's columns
    void delete_item(const QModelIndex &index);

    // Sets current scope item in the scope tree
    void set_current_scope(const QModelIndex &index);

    // Clears children of this scope item, then fetches them
    // again.
    void refresh_scope(const QModelIndex &index);

    // Gets selected item(s) from currently focused view,
    // which could be scope or results. Only the main (first
    // column) item is returned for each selected row.

    // NOTE: there is always at least one selected item. If
    // results is currently focused but has no selection,
    // selected items from scope are returned instead.
    QList<QModelIndex> get_selected_items() const;

    // Get a single selected item. Use if you are sure that
    // there's only one (dialog that uses one target item
    // for example). Returns first item in list if there are
    // multiple items selected.
    QModelIndex get_selected_item() const;

    // NOTE: Search is inclusive, examining the given parent
    // and all of it's descendants. Pass QModelIndex()
    // parent to search the whole model. If no type is
    // given, then items of all types will be returned.
    QList<QModelIndex> search_items(const QModelIndex &parent, int role, const QVariant &value, const int type = -1) const;

    QModelIndex get_current_scope_item() const;
    int get_child_count(const QModelIndex &index) const;

    QStandardItem *get_item(const QModelIndex &index) const;
    QList<QStandardItem *> get_row(const QModelIndex &index) const;

    void add_actions(QMenu *action_menu, QMenu *view_menu, QMenu *preferences_menu, QToolBar *toolbar);

    QVariant save_state() const;
    // NOTE: all results should be registered before this is
    // called so that their state can be restored
    void restore_state(const QVariant &state);

    QAction *get_refresh_action() const;

signals:
    // Emitted when current scope item changes.
    void current_scope_item_changed(const QModelIndex &index);

    // Emitted when a results item is double-clicked or
    // pressed ENTER on. Connect this signal to an
    // appropriate default action.
    void results_item_activated(const QModelIndex);

    // Emitted when actions need to updated due to selection
    // changing.
    void actions_changed();

private:
    ConsoleWidgetPrivate *d;

    friend ConsoleDragModel;
};

int console_item_get_type(const QModelIndex &index);
bool console_item_get_was_fetched(const QModelIndex &index);

#endif /* CONSOLE_WIDGET_H */
