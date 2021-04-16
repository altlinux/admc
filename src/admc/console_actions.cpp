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

#include "console_actions.h"

#include "console_types/object.h"
#include "console_types/policy.h"
#include "console_types/query.h"

#include <QMenu>
#include <QModelIndex>
#include <QSet>

ConsoleActions::ConsoleActions(QObject *parent)
: QObject(parent)
{
    for (int action_i = ObjectAction_NewUser; action_i < ObjectAction_LAST; action_i++) {
        const ObjectAction action_enum = (ObjectAction) action_i;

        const QString action_text =
        [action_enum]() {
            switch(action_enum) {
                case ObjectAction_NewUser: return tr("&User");
                case ObjectAction_NewComputer: return tr("&Computer");
                case ObjectAction_NewOU: return tr("&Organization");
                case ObjectAction_NewGroup: return tr("&Group");
                case ObjectAction_Find: return tr("&Find");
                case ObjectAction_AddToGroup: return tr("&Add to group");
                case ObjectAction_Enable: return tr("&Enable account");
                case ObjectAction_Disable: return tr("D&isable account");
                case ObjectAction_ResetPassword: return tr("Reset &Password");
                case ObjectAction_Rename: return tr("&Rename");
                case ObjectAction_Delete: return tr("&Delete");
                case ObjectAction_Move: return tr("&Move");
                case ObjectAction_EditUpnSuffixes: return tr("Edit &Upn Suffixes");

                case ObjectAction_PolicyCreate: return tr("&Policy");
                case ObjectAction_PolicyAddLink: return tr("&Add link");
                case ObjectAction_PolicyRename: return tr("&Rename");
                case ObjectAction_PolicyDelete: return tr("&Delete");

                case ObjectAction_QueryCreateFolder: return tr("&Folder");
                case ObjectAction_QueryCreateItem: return tr("&Query");
                case ObjectAction_QueryEditFolder: return tr("&Edit");
                case ObjectAction_QueryDeleteItemOrFolder: return tr("&Delete");

                case ObjectAction_LAST: break;
            }
            return QString();
        }();

        actions[action_enum] = new QAction(action_text, this);
    }

    new_menu = new QMenu(tr("&New"));

    new_actions = {
        ObjectAction_NewUser,
        ObjectAction_NewComputer,
        ObjectAction_NewOU,
        ObjectAction_NewGroup,

        ObjectAction_PolicyCreate,

        ObjectAction_QueryCreateFolder,
        ObjectAction_QueryCreateItem,
    };
    for (const ObjectAction action_enum : new_actions) {
        QAction *action = get(action_enum);
        new_menu->addAction(action);
    }
}

QAction *ConsoleActions::get(const ObjectAction action) const {
    return actions[action];
}

void ConsoleActions::show(const ObjectAction action_enum) {
    QAction *action = get(action_enum);
    action->setVisible(true);
}

void ConsoleActions::add_to_menu(QMenu *menu) {
    menu->addMenu(new_menu);

    menu->addSeparator();

    object_add_actions_to_menu(this, menu);
    policy_add_actions_to_menu(this, menu);
    query_add_actions_to_menu(this, menu);
}

void ConsoleActions::update_actions_visibility(const QList<QModelIndex> &indexes) {
    // Hide all actions first
    for (QAction *action : actions.values()) {
        action->setVisible(false);
    }
    new_menu->menuAction()->setVisible(false);

    // Show actions which need to be shown
    object_show_hide_actions(this, indexes);
    policy_show_hide_actions(this, indexes);
    query_show_hide_actions(this, indexes);

    // Show "New" menu if any new actions are visible
    const bool any_new_action_visible =
    [&]() {
        for (const ObjectAction action_enum : new_actions) {
            QAction *action = get(action_enum);

            if (action->isVisible()) {
                return true;
            }
        }

        return false;
    }();

    new_menu->menuAction()->setVisible(any_new_action_visible);
}
