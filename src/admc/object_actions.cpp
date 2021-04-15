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

#include "object_actions.h"

#include "globals.h"
#include "utils.h"
#include "console_types/object.h"
#include "adldap.h"
#include "properties_dialog.h"
#include "settings.h"
#include "filter_dialog.h"
#include "filter_widget/filter_widget.h"
#include "status.h"
#include "rename_dialog.h"
#include "select_container_dialog.h"
#include "create_dialog.h"
#include "select_dialog.h"
#include "find_dialog.h"
#include "password_dialog.h"
#include "console_widget/console_widget.h"
#include "console_widget/results_view.h"
#include "central_widget.h"

#include <QMenu>
#include <QModelIndex>
#include <QSet>
#include <QString>

ObjectActions::ObjectActions(QObject *parent)
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

                case ObjectAction_LAST: break;
            }
            return QString();
        }();

        actions[action_enum] = new QAction(action_text, this);
    }

    new_menu = new QMenu(tr("&New"));

    const QList<ObjectAction> new_actions = {
        ObjectAction_NewUser,
        ObjectAction_NewComputer,
        ObjectAction_NewOU,
        ObjectAction_NewGroup,
    };
    for (const ObjectAction action_enum : new_actions) {
        QAction *action = get(action_enum);
        new_menu->addAction(action);
    }
}

QAction *ObjectActions::get(const ObjectAction action) const {
    return actions[action];
}

void ObjectActions::add_to_menu(QMenu *menu) {
    // Container
    menu->addMenu(new_menu);
    menu->addAction(get(ObjectAction_Find));

    menu->addSeparator();

    // User
    menu->addAction(get(ObjectAction_AddToGroup));
    menu->addAction(get(ObjectAction_Enable));
    menu->addAction(get(ObjectAction_Disable));
    menu->addAction(get(ObjectAction_ResetPassword));

    // Other
    menu->addAction(get(ObjectAction_EditUpnSuffixes));

    menu->addSeparator();

    // General object
    menu->addAction(get(ObjectAction_Delete));
    menu->addAction(get(ObjectAction_Rename));
    menu->addAction(get(ObjectAction_Move));
}

void ObjectActions::hide_actions() {
    for (QAction *action : actions.values()) {
        action->setVisible(false);
    }
    new_menu->menuAction()->setVisible(false);
}

void ObjectActions::update_actions_visibility(const QList<QModelIndex> &selected_indexes) {
    auto show_action =
    [this](const ObjectAction action_enum) {
        QAction *action = get(action_enum);
        action->setVisible(true);
    };

    hide_actions();

    if (!indexes_are_of_type(selected_indexes, ItemType_DomainObject)) {
        return;
    }

    // Get info about selected objects from view
    const QList<QString> targets =
    [=]() {
        QList<QString> out;

        for (const QModelIndex index : selected_indexes) {
            const QString dn = index.data(ObjectRole_DN).toString();

            out.append(dn);
        }

        return out;  
    }();

    const QSet<QString> target_classes =
    [=]() {
        QSet<QString> out;
        
        for (const QModelIndex index : selected_indexes) {
            const QList<QString> object_classes = index.data(ObjectRole_ObjectClasses).toStringList();
            const QString main_object_class = object_classes.last();

            out.insert(main_object_class);
        }

        return out;
    }();

    const bool single_object = (targets.size() == 1);

    if (single_object) {
        const QModelIndex index = selected_indexes[0];
        const QString target = targets[0];
        const QString target_class = target_classes.values()[0];

        // Get info about object that will determine which
        // actions are present/enabled
        const bool is_container =
        [=]() {
            const QList<QString> container_classes = g_adconfig->get_filter_containers();

            return container_classes.contains(target_class);
        }();

        const bool is_user = (target_class == CLASS_USER);
        const bool is_domain = (target_class == CLASS_DOMAIN);

        const bool cannot_move = index.data(ObjectRole_CannotMove).toBool();
        const bool cannot_rename = index.data(ObjectRole_CannotRename).toBool();
        const bool cannot_delete = index.data(ObjectRole_CannotDelete).toBool();
        const bool account_disabled = index.data(ObjectRole_AccountDisabled).toBool();

        if (is_container) {
            new_menu->menuAction()->setVisible(true);
            for (QAction *new_action : new_menu->actions()) {
                new_action->setVisible(true);
            }

            show_action(ObjectAction_Find);
        }

        if (is_user) {
            show_action(ObjectAction_AddToGroup);
            show_action(ObjectAction_ResetPassword);

            if (account_disabled) {
                show_action(ObjectAction_Enable);
            } else {
                show_action(ObjectAction_Disable);
            }
        }

        if (is_domain) {
            get(ObjectAction_EditUpnSuffixes)->setVisible(true);
        }

        show_action(ObjectAction_Move);
        show_action(ObjectAction_Delete);
        show_action(ObjectAction_Rename);

        get(ObjectAction_Move)->setDisabled(cannot_move);
        get(ObjectAction_Delete)->setDisabled(cannot_delete);
        get(ObjectAction_Rename)->setDisabled(cannot_rename);
    } else if (targets.size() > 1) {
        const bool all_users = (target_classes.contains(CLASS_USER) && target_classes.size() == 1);

        if (all_users) {
            show_action(ObjectAction_AddToGroup);

            // NOTE: show both enable/disable for multiple
            // users because some users might be disabled,
            // some enabled and we want to provide a way to
            // disable all or enable all
            show_action(ObjectAction_Enable);
            show_action(ObjectAction_Disable);
        }

        show_action(ObjectAction_Move);
        show_action(ObjectAction_Delete);
    }
}

QList<QString> object_delete(const QList<QString> &targets, QWidget *parent) {
    if (targets.size() == 0) {
        return QList<QString>();
    }

    const bool confirmed = confirmation_dialog(QCoreApplication::translate("ObjectActions", "Are you sure you want to delete this object?"), parent);
    if (!confirmed) {
        return QList<QString>();
    }

    QList<QString> deleted_objects;

    AdInterface ad;
    if (ad_failed(ad)) {
        return QList<QString>();
    }

    show_busy_indicator();

    for (const QString &dn : targets) {
        const bool success = ad.object_delete(dn);

        if (success) {
            deleted_objects.append(dn);
        }
    }

    hide_busy_indicator();

    g_status()->display_ad_messages(ad, parent);

    return deleted_objects;
}

QList<QString> object_enable_disable(const QList<QString> &targets, const bool disabled, QWidget *parent) {
    AdInterface ad;
    if (ad_failed(ad)) {
        return QList<QString>();
    }

    show_busy_indicator();

    QList<QString> changed_objects;

    for (const QString &dn : targets) {
        const bool success = ad.user_set_account_option(dn, AccountOption_Disabled, disabled);

        if (success) {
            changed_objects.append(dn);
        }
    }

    hide_busy_indicator();

    g_status()->display_ad_messages(ad, parent);

    return changed_objects;
}

void object_add_to_group(const QList<QString> &targets, QWidget *parent) {
    auto dialog = new SelectDialog({CLASS_GROUP}, SelectDialogMultiSelection_Yes, parent);

    QObject::connect(
        dialog, &SelectDialog::accepted,
        [=]() {
            AdInterface ad;
            if (ad_failed(ad)) {
                return;
            }

            show_busy_indicator();

            const QList<QString> groups = dialog->get_selected();

            for (const QString &target : targets) {
                for (auto group : groups) {
                    ad.group_add_member(group, target);
                }
            }

            hide_busy_indicator();

            g_status()->display_ad_messages(ad, parent);
        });

    dialog->open();
}
