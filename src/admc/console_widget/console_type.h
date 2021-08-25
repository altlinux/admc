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

#ifndef CONSOLE_TYPE_H
#define CONSOLE_TYPE_H

/**
 * Base class for implementing item logic specific to
 * different item types. You should implement this class for
 * each type of item that you use. Imlemented functions will
 * be called by console.
 */

#include <QObject>

class ConsoleWidget;

class ConsoleType : public QObject {
    Q_OBJECT

public:
    ConsoleType(ConsoleWidget *console_arg);

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

protected:
    ConsoleWidget *console;
};

#endif /* CONSOLE_TYPE_H */
