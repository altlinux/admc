/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2026 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

#ifndef CORE_CONSOLE_H
#define CORE_CONSOLE_H

#include <Qt>

class QAction;

enum ConsoleRole {
    // Determines whether scope item was fetched
    ConsoleRole_WasFetched = Qt::UserRole + 1,
    ConsoleRole_SortIndex = Qt::UserRole + 2,

    ConsoleRole_IsScope = Qt::UserRole + 3,
    ConsoleRole_IsHidden

    // NOTE: don't go above ConsoleRole_Type and
    // ConsoleRole_LAST (defined in public header)

    // NOTE: these roles are "public" defined below:
    // ConsoleRole_Type = Qt::UserRole + 19,
    // ConsoleRole_LAST = Qt::UserRole + 20
};

enum ConsoleRolePublic {
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

// TODO: Why is it called "My Console Role"?  That's weird, we have to rename it
// to something more meaningful.
enum MyConsoleRole {
    MyConsoleRole_SearchThreadId = ConsoleRole_LAST + 1,
    MyConsoleRole_LAST,
};

// TODO: Find a way to unite policy, policy ou and object subtree
// roles. It'll help to not include all impl headers every time
// to get mentioned roles.
enum ObjectRole {
    ObjectRole_DN = MyConsoleRole_LAST + 1,
    ObjectRole_ObjectClasses,
    ObjectRole_ObjectCategory,
    ObjectRole_CannotMove,
    ObjectRole_CannotRename,
    ObjectRole_CannotDelete,
    ObjectRole_AccountDisabled,
    ObjectRole_Fetching,
    ObjectRole_SearchId,

    ObjectRole_LAST,
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
};

class ConsoleWidgetActions final {
public:
    QAction *navigate_up;
    QAction *navigate_back;
    QAction *navigate_forward;
    QAction *refresh;
    QAction *customize_columns;
    QAction *view_icons;
    QAction *view_list;
    QAction *view_detail;
    QAction *toggle_console_tree;
    QAction *toggle_description_bar;
};

#endif  /* ifndef CORE_CONSOLE_H */
