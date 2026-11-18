/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
 * Copyright (C) 2026 Artyom V. Poptsov
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

/**
 * @file console_widget.h
 * @brief The central widget of the app through which the user can
 * browse and manipulate objects.
 */

#ifndef CONSOLE_WIDGET_H
#define CONSOLE_WIDGET_H

#include <QWidget>

#include "core/console/console.h"

class ConsoleWidgetPrivate;
class QStandardItem;
class QMenu;
class ConsoleImpl;
class ConsoleDragModel;

/**
 * @brief The central widget of the app through which the user can
 * browse and manipulate objects.
 *
 * It contains two panes: "scope" and "results".  Scope pane contains a tree of
 * items.  Each scope item has it's own "results" which are displayed in the
 * results pane when the scope item is selected.  Results can contain children
 * of the scope item.  Results may also display a custom widget.  Note that this
 * class only deals with general "items" and doesn't define how items are
 * loaded, displayed or interacted with.  That is defined by sub-classing
 * ConsoleImpl and registering implementations using register_impl() function.
 * Implementations are assigned to item types which should be defined outside of
 * this class.
 */
class ConsoleWidget final : public QWidget {
    Q_OBJECT

public:
    ConsoleWidget(QWidget *parent);

    /**
     * Sets and connects actions to internal slots
     *
     * NOTE: must be called before restore_state(), so that action state is
     * restored
    */
    void set_actions(const ConsoleWidgetActions &actions_arg);

    /**
     * NOTE: you must register all impl's before adding items
     */
    void register_impl(const int type, ConsoleImpl *impl);

    /**
     * Add an item to the console that is shown both in scope and results.
     *
     * Items returned from the function should be used to set text, icon and
     * your custom data roles.
     */
    QList<QStandardItem *> add_scope_item(const int type, const QModelIndex &parent);

    /**
     * Add an item to the console item that is shown only in results.
     *
     * Items returned from the function should be used to set text, icon and
     * your custom data roles
     */
    QList<QStandardItem *> add_results_item(const int type, const QModelIndex &parent);

    /**
     * Deletes an item and all of its columns.
     */
    void delete_item(const QModelIndex &index);

    /**
     * Sets the current scope item in the scope tree.
     */
    void set_current_scope(const QModelIndex &index);

    /**
     * Calls refresh() of the console impl for this item type.
     */
    void refresh_scope(const QModelIndex &index);

    /**
     * Get selected item(s) from the currently focused view, which could be
     * scope or results.  Only the main (first column) item is returned for each
     * selected row.  There is always at least one selected item.
     */
    QList<QModelIndex> get_selected_items(const int type) const;

    /**
     * Get a single selected item.  Use it if you are sure that there's only one
     (call it in a a dialog that uses one selected item, for example).
     */
    QModelIndex get_selected_item(const int type) const;

    /**
     * NOTE: Search is inclusive, examining the given parent and all of it's
     * descendants. Pass QModelIndex() parent to search the whole model. If no
     * type is given, then items of all types will be returned.
     */
    QList<QModelIndex> search_items(const QModelIndex &parent, int role, const QVariant &value, const QList<int> &type = QList<int>()) const;

    QList<QModelIndex> search_items(const QModelIndex &parent, const QList<int> &type) const;

    /**
     * Single index version of a search function.  Use when you expect only one
     * valid result or none. QModelIndex() is returned if no items are found.
     */
    QModelIndex search_item(const QModelIndex &parent, int role, const QVariant &value, const QList<int> &type = QList<int>()) const;

    /**
     * Single index version of a search function.  Use when you expect only one
     * valid result or none. QModelIndex() is returned if no items are found.
     */
    QModelIndex search_item(const QModelIndex &parent, const QList<int> &type) const;

    QModelIndex get_current_scope_item() const;
    int get_child_count(const QModelIndex &index) const;

    QStandardItem *get_item(const QModelIndex &index) const;
    QList<QStandardItem *> get_row(const QModelIndex &index) const;

    QVariant save_state() const;

    /**
     * NOTE: All implementations should be registered before calling this method
     * so that their state can be restored.
     */
    void restore_state(const QVariant &state);

    void delete_children(const QModelIndex &parent);

    void set_scope_view_visible(const bool visible);

    /**
     * Setups the action menu in menubar. Action menu opened by right click is
     * setup automatically.  Note that this must be called *after* all
     * register_impl() calls.
     */
    void setup_menubar_action_menu(QMenu *menu);

    /**
     * Define custom sort index for scope item.  By default scope items are
     * sorted by their item text.  Use sort indexes if you need more
     * fine-grained sort behavior.  Default sort index is "0".  Note that this
     * doesn't affect order in results pane.
     */
    void set_item_sort_index(const QModelIndex &index, const int sort_index);

    void update_current_item_results_widget();

    /**
     * Gets current scope item's result widget for given scope item index.  Can
     * return nullptr.
     */
    QWidget *get_result_widget_for_index(const QModelIndex &index);

    /**
     * Removes all items from scope tree view except top domain info item.  It
     * is used when domain changes.
     */
    void clear_scope_tree();

    void expand_item(const QModelIndex &index);

    QPersistentModelIndex domain_info_index();

    void hide_scope_and_results(bool hide);

    void save_tree_state();
    void restore_tree_state();

signals:
    /**
     * Emitted when selection in the whole console widget changes, both in scope
     * and results panes.  Can be caused by selection change in focused view or
     * change of which view is focused.
     */
    void selection_changed();
    void fsmo_master_changed(const QString &new_master_dn, const QString &string_fsmo_role);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;

    void translate_ui();
private:
    ConsoleWidgetPrivate *d;

    friend ConsoleDragModel;
};

int console_item_get_type(const QModelIndex &index);
bool console_item_get_was_fetched(const QModelIndex &index);

#endif /* CONSOLE_WIDGET_H */
