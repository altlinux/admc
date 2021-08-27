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

#ifndef CONSOLE_IMPL_H
#define CONSOLE_IMPL_H

/**
 * Base class for implementing item logic specific to
 * different item types. You should implement this class for
 * each type of item that you use. Imlemented functions will
 * be called by console.
 */

#include <QObject>

#include "console_widget/console_widget.h"

class ConsoleWidget;

class ConsoleImpl : public QObject {
    Q_OBJECT

public:
    ConsoleImpl(ConsoleWidget *console_arg);

    // Called when an item of this type is fetched. Only
    // called on items that are dynamic. You should load
    // children in the implementation.
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

    virtual void activate(const QModelIndex &index);

    // TODO: comment
    virtual QSet<StandardAction> get_visible_standard_actions(const QModelIndex &index) const;

    // NOTE: It is possible to select indexes of mixed type.
    // For this reason, action f-ns will be called with
    // index lists that only contain indexes of the same
    // type as this impl. Use this list, NOT the
    // list from console's get_selected_items().

    // TODO: comment
    virtual void copy(const QList<QModelIndex> &index_list);

    // TODO: comment
    virtual void cut(const QList<QModelIndex> &index_list);
    
    // TODO: comment
    virtual void rename(const QList<QModelIndex> &index_list);
    
    // TODO: comment
    virtual void delete_action(const QList<QModelIndex> &index_list);
    
    // TODO: comment
    virtual void paste(const QList<QModelIndex> &index_list);
    
    // TODO: comment
    virtual void print(const QList<QModelIndex> &index_list);
    
    // TODO: comment
    virtual void refresh(const QList<QModelIndex> &index_list);
    
    // TODO: comment
    virtual void properties(const QList<QModelIndex> &index_list);

protected:
    ConsoleWidget *console;
};

#endif /* CONSOLE_IMPL_H */
