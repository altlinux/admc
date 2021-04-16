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

#ifndef CONSOLE_ACTIONS_H
#define CONSOLE_ACTIONS_H

/**
 * Helper storage class for managing console actions.
 */

#include <QObject>
#include <QHash>

class QMenu;
class QAction;
class QModelIndex;
template <typename T> class QList;

enum ObjectAction {
    ObjectAction_NewUser,
    ObjectAction_NewComputer,
    ObjectAction_NewOU,
    ObjectAction_NewGroup,
    ObjectAction_Find,
    ObjectAction_AddToGroup,
    ObjectAction_Enable,
    ObjectAction_Disable,
    ObjectAction_ResetPassword,
    ObjectAction_Rename,
    ObjectAction_Delete,
    ObjectAction_Move,
    ObjectAction_EditUpnSuffixes,

    ObjectAction_PolicyCreate,
    ObjectAction_PolicyAddLink,
    ObjectAction_PolicyRename,
    ObjectAction_PolicyDelete,

    ObjectAction_QueryCreateFolder,
    ObjectAction_QueryCreateItem,
    ObjectAction_QueryEditFolder,
    ObjectAction_QueryDeleteItemOrFolder,
    ObjectAction_QueryMoveItemOrFolder,

    ObjectAction_LAST,
};

class ConsoleActions final : public QObject {

public:
    ConsoleActions(QObject *parent);

    QAction *get(const ObjectAction action) const;
    void show(const ObjectAction action);
    void add_to_menu(QMenu *menu);
    void update_actions_visibility(const QList<QModelIndex> &indexes);

private:
    QHash<ObjectAction, QAction *> actions;
    QMenu *new_menu;
    QList<ObjectAction> new_actions;
};

#endif /* CONSOLE_ACTIONS_H */
