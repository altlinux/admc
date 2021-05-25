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

#include "console_types/console_object.h"
#include "console_types/console_policy.h"
#include "console_types/console_query.h"

#include <QMenu>
#include <QModelIndex>
#include <QSet>

ConsoleActions::ConsoleActions(QObject *parent)
: QObject(parent)
{
    for (int action_i = ConsoleAction_NewUser; action_i < ConsoleAction_LAST; action_i++) {
        const ConsoleAction action_enum = (ConsoleAction) action_i;

        const QString action_text =
        [action_enum]() {
            switch(action_enum) {
                case ConsoleAction_NewUser: return tr("&User");
                case ConsoleAction_NewComputer: return tr("&Computer");
                case ConsoleAction_NewOU: return tr("&Organization");
                case ConsoleAction_NewGroup: return tr("&Group");
                case ConsoleAction_Find: return tr("&Find");
                case ConsoleAction_AddToGroup: return tr("&Add to group");
                case ConsoleAction_Enable: return tr("&Enable account");
                case ConsoleAction_Disable: return tr("D&isable account");
                case ConsoleAction_ResetPassword: return tr("Reset &Password");
                case ConsoleAction_Rename: return tr("&Rename");
                case ConsoleAction_Delete: return tr("&Delete");
                case ConsoleAction_Move: return tr("&Move");
                case ConsoleAction_EditUpnSuffixes: return tr("Edit &Upn Suffixes");
                case ConsoleAction_ChangeDC: return tr("Change domain controller");

                case ConsoleAction_PolicyCreate: return tr("&Policy");
                case ConsoleAction_PolicyAddLink: return tr("&Add link");
                case ConsoleAction_PolicyRename: return tr("&Rename");
                case ConsoleAction_PolicyDelete: return tr("&Delete");

                case ConsoleAction_QueryCreateFolder: return tr("&Folder");
                case ConsoleAction_QueryCreateItem: return tr("&Query");
                case ConsoleAction_QueryEditFolder: return tr("&Edit");
                case ConsoleAction_QueryEditItem: return tr("&Edit");
                case ConsoleAction_QueryDeleteItemOrFolder: return tr("&Delete");
                case ConsoleAction_QueryCutItemOrFolder: return tr("Cut");
                case ConsoleAction_QueryCopyItemOrFolder: return tr("&Copy");
                case ConsoleAction_QueryPasteItemOrFolder: return tr("&Paste");
                case ConsoleAction_QueryExport: return tr("&Export query");
                case ConsoleAction_QueryImport: return tr("&Import query");

                case ConsoleAction_LAST: break;
            }
            return QString();
        }();

        actions[action_enum] = new QAction(action_text, this);
    }

    new_menu = new QMenu(tr("&New"));

    new_actions = {
        ConsoleAction_NewUser,
        ConsoleAction_NewComputer,
        ConsoleAction_NewOU,
        ConsoleAction_NewGroup,

        ConsoleAction_PolicyCreate,

        ConsoleAction_QueryCreateFolder,
        ConsoleAction_QueryCreateItem,
    };
    for (const ConsoleAction action_enum : new_actions) {
        QAction *action = get(action_enum);
        new_menu->addAction(action);
    }
}

QAction *ConsoleActions::get(const ConsoleAction action) const {
    return actions[action];
}

void ConsoleActions::add_to_menu(QMenu *menu) {
    menu->addMenu(new_menu);

    menu->addSeparator();

    console_object_actions_add_to_menu(this, menu);
    console_policy_actions_add_to_menu(this, menu);
    console_query_actions_add_to_menu(this, menu);
}

void ConsoleActions::update_actions_visibility(const QList<QModelIndex> &indexes) {
    // Figure out action state for current selection. For
    // single selections this is trivial. For multi
    // selections have to get the intersection of visiblity
    // states so that only actions that can apply to all
    // selected items are visible.
    QSet<ConsoleAction> visible_actions;
    QSet<ConsoleAction> disabled_actions;

    const bool single_selection = (indexes.size() == 1);

    for (int i = 0; i < indexes.size(); i++) {
        const QModelIndex index = indexes[i];
        QSet<ConsoleAction> this_visible_actions;

        console_object_actions_get_state(index, single_selection, &this_visible_actions, &disabled_actions);
        console_query_actions_get_state(index, single_selection, &this_visible_actions, &disabled_actions);
        console_policy_actions_get_state(index, single_selection, &this_visible_actions, &disabled_actions);

        if (i == 0) {
            // NOTE: for first index, add the whole set
            // instead of intersecting, otherwise total set
            // would just stay empty
            visible_actions = this_visible_actions;
        } else {
            visible_actions.intersect(this_visible_actions);
        }
    }

    for (const ConsoleAction action_enum : actions.keys()) {
        QAction *action = actions[action_enum];

        const bool is_visible = visible_actions.contains(action_enum);
        action->setVisible(is_visible);

        const bool is_disabled = disabled_actions.contains(action_enum);
        action->setDisabled(is_disabled);
    }

    // Show "New" menu if any new actions are visible
    const bool any_new_action_visible =
    [&]() {
        for (const ConsoleAction action_enum : new_actions) {
            QAction *action = actions[action_enum];

            if (action->isVisible()) {
                return true;
            }
        }

        return false;
    }();

    new_menu->menuAction()->setVisible(any_new_action_visible);
}
