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

enum ConsoleAction {
    ConsoleAction_NewUser,
    ConsoleAction_NewComputer,
    ConsoleAction_NewOU,
    ConsoleAction_NewGroup,
    ConsoleAction_Find,
    ConsoleAction_AddToGroup,
    ConsoleAction_Enable,
    ConsoleAction_Disable,
    ConsoleAction_ResetPassword,
    ConsoleAction_ResetComputerAccount,
    ConsoleAction_Rename,
    ConsoleAction_Delete,
    ConsoleAction_Move,
    ConsoleAction_EditUpnSuffixes,
    ConsoleAction_ChangeDC,

    ConsoleAction_PolicyCreate,
    ConsoleAction_PolicyAddLink,
    ConsoleAction_PolicyRename,
    ConsoleAction_PolicyDelete,

    ConsoleAction_QueryCreateFolder,
    ConsoleAction_QueryCreateItem,
    ConsoleAction_QueryEditFolder,
    ConsoleAction_QueryEditItem,
    ConsoleAction_QueryDeleteItemOrFolder,
    ConsoleAction_QueryCutItemOrFolder,
    ConsoleAction_QueryCopyItemOrFolder,
    ConsoleAction_QueryPasteItemOrFolder,
    ConsoleAction_QueryExport,
    ConsoleAction_QueryImport,

    ConsoleAction_LAST,
};

class ConsoleActions final : public QObject {
    Q_OBJECT

public:
    ConsoleActions(QObject *parent);

    QAction *get(const ConsoleAction action) const;
    void add_to_menu(QMenu *menu);
    void update_actions_visibility(const QList<QModelIndex> &indexes);

private:
    QHash<ConsoleAction, QAction *> actions;
    QMenu *new_menu;
    QList<ConsoleAction> new_actions;
};

#endif /* CONSOLE_ACTIONS_H */
