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

#ifndef CONSOLE_IMPL_H
#define CONSOLE_IMPL_H

/**
 * Base class for implementing item logic specific to
 * different item types. You should implement this class for
 * each type of item that you use. Implemented functions
 * will be called by console.
 */

#include <QObject>

#include "console_widget/console_widget.h"

class ConsoleWidget;
class ResultsView;

class ConsoleImpl : public QObject {
    Q_OBJECT

public:
    ConsoleImpl(ConsoleWidget *console_arg);

    // Called when a scope item of this type. Fetching
    // happens when using the user selects or expands an
    // item for the first time. Typically this is where you
    // would load children of an item, if they need to be
    // loaded dynamically. If children of your item type are
    // static, you don't need to implement this.
    virtual void fetch(const QModelIndex &index);

    // Called when items are dragged on top of an item of
    // this type to determine whether dropping is allowed.
    // Note that dragged items may be of any type and even
    // contain mixed types.
    virtual bool can_drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type);

    // Called when items are dropped onto item of this type.
    // In the implementation, move items or perform other
    // operations as necessary.
    virtual void drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type);

    // Called when description bar text needs to be updated
    // and currently selected scope item is of this type.
    // Return whatever text should be displayed.
    virtual QString get_description(const QModelIndex &index) const;

    // Called when an item of this type is activated, by
    // being double clicked or pressed enter on. Implement
    // appropriate response if needed.
    virtual void activate(const QModelIndex &index);

    // Called when an item of this type is selected in the
    // scope pane.
    virtual void selected_as_scope(const QModelIndex &index);

    // Return all custom actions that are available for this
    // type. This will be used to determine the order of
    // actions in the menu.
    virtual QList<QAction *> get_all_custom_actions() const;

    // Return a set of custom actions that should be
    // disabled
    virtual QSet<QAction *> get_disabled_custom_actions(const QModelIndex &index, const bool single_selection) const;

    // Return all custom actions that are available for this
    // item. This should be a subset of all possible
    // actions.
    virtual QSet<QAction *> get_custom_actions(const QModelIndex &index, const bool single_selection) const;

    // Return a set of standard actions that should be
    // displayed for this item
    virtual QSet<StandardAction> get_standard_actions(const QModelIndex &index, const bool single_selection) const;

    // Return a set of standard actions that should be
    // disabled
    virtual QSet<StandardAction> get_disabled_standard_actions(const QModelIndex &index, const bool single_selection) const;

    // NOTE: It is possible to select indexes of mixed type.
    // For this reason, action f-ns will be called with
    // index lists that only contain indexes of the same
    // type as this impl. Use this list, NOT the
    // list from console's get_selected_items().

    // Implementations of standard actions. Note that you
    // don't have to implement all of these, only the ones
    // that you return from get_standard_actions().
    virtual void copy(const QList<QModelIndex> &index_list);
    virtual void cut(const QList<QModelIndex> &index_list);
    virtual void rename(const QList<QModelIndex> &index_list);
    virtual void delete_action(const QList<QModelIndex> &index_list);
    virtual void paste(const QList<QModelIndex> &index_list);
    virtual void print(const QList<QModelIndex> &index_list);
    virtual void refresh(const QList<QModelIndex> &index_list);
    virtual void properties(const QList<QModelIndex> &index_list);

    // Override to provide custom header labels and default
    // columns. You need to do this if you are defining a
    // results view.
    virtual QList<QString> column_labels() const;
    virtual QList<int> default_columns() const;

    virtual void update_results_widget(const QModelIndex &index) const;

    QVariant save_state() const;
    void restore_state(const QVariant &state);

    QWidget *widget() const;
    ResultsView *view() const;

protected:
    ConsoleWidget *console;

    // Here are the options for results widget and view:
    // 1. Widget only - provide your own custom widget using
    //    set_results_widget().
    // 2. View only - provide a view using
    //    set_results_view().
    // 3. View and widget - provide both using setters.
    // Default behavior is a blank widget.
    void set_results_view(ResultsView *view);
    void set_results_widget(QWidget *widget);

private:
    ResultsView *results_view;
    QWidget *results_widget;
};

#endif /* CONSOLE_IMPL_H */
