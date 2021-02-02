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

#include "object_menu.h"
#include "ad_interface.h"
#include "ad_config.h"
#include "ad_utils.h"
#include "ad_object.h"
#include "confirmation_dialog.h"
#include "select_dialog.h"
#include "rename_dialog.h"
#include "password_dialog.h"
#include "create_dialog.h"
#include "details_dialog.h"
#include "select_container_dialog.h"
#include "utils.h"
#include "find_dialog.h"
#include "filter.h"
#include "status.h"
#include "object_model.h"

#include <QMenu>
#include <QPoint>
#include <QAbstractItemView>
#include <QDebug>

// NOTE: for dialogs opened from this menu, the parent of the menu is passed NOT the menu itself, because the menu closes (and gets deleted if this is the context menu) when dialog opens.

void delete_object(const QList<QString> targets, QWidget *parent);
void move(const QList<QString> targets, QWidget *parent);
void add_to_group(const QList<QString> targets, QWidget *parent);
void enable_account(const QList<QString> targets, QWidget *parent);
void disable_account(const QList<QString> targets, QWidget *parent);

void details(const QString &target, QWidget *parent);
void rename(const QString &target, QWidget *parent);
void create(const QString &target, const QString &object_class, QWidget *parent);
void find(const QString &target, QWidget *parent);
void reset_password(const QString &target, QWidget *parent);

QString targets_display_string(const QList<QString> targets);

// Construct actions of the menu based on current target(s)
// NOTE: construct right before showing menu instead of in
// load_targets() because target's attributes might change
// in the span of time when target is selected and menu is
// opened. Menu needs most up-to-date target attributes to
// construct actions.
void add_object_actions_to_menu(QMenu *menu, QAbstractItemView *view, QWidget *parent) {
    // Get info about selected objects from view
    const QList<QString> targets =
    [=]() {
        QList<QString> out;

        const QList<QModelIndex> indexes = view->selectionModel()->selectedIndexes();

        for (const QModelIndex index : indexes) {
            // Need first column to access item data
            if (index.column() != 0) {
                continue;
            }

            const QString dn = index.data(Role_DN).toString();

            out.append(dn);
        }

        return out;  
    }();

    const QSet<QString> target_classes =
    [=]() {
        QSet<QString> out;
        
        const QList<QModelIndex> indexes = view->selectionModel()->selectedIndexes();

        for (const QModelIndex index : indexes) {
            // Need first column to access item data
            if (index.column() != 0) {
                continue;
            }

            const QString object_class = index.data(Role_ObjectClass).toString();

            out.insert(object_class);
        }

        return out;
    }();

    // These are f-ns that add menu's
    auto add_new =
    [=]() {
        QMenu *submenu_new = menu->addMenu(QObject::tr("New"));
        static const QList<QString> create_classes = {
            CLASS_USER,
            CLASS_COMPUTER,
            CLASS_OU,
            CLASS_GROUP,
        };
        for (const auto object_class : create_classes) {
            const QString action_text = ADCONFIG()->get_class_display_name(object_class);

            submenu_new->addAction(action_text,
                [=]() {
                    create(targets[0], object_class, parent);
                });
        }
    };

    auto add_find =
    [=]() {
        menu->addAction(QObject::tr("Find"),
            [=]() {
                find(targets[0], parent);
            });
    };

    auto add_add_to_group =
    [=]() {
        menu->addAction(QObject::tr("Add to group"),
            [=]() {
                add_to_group(targets, parent);
            });
    };

    auto add_reset_password =
    [=]() {
        menu->addAction(QObject::tr("Reset password"),
            [=]() {
                reset_password(targets[0], parent);
            });
    };

    auto add_disable_account =
    [=]() {
        menu->addAction(QObject::tr("Disable account"),
            [=]() {
                disable_account(targets, parent);
            });
    };

    auto add_enable_account =
    [=]() {
        menu->addAction(QObject::tr("Enable account"),
            [=]() {
                enable_account(targets, parent);
            });
    };

    auto add_move =
    [=](const bool disabled) {
        auto action = menu->addAction(QObject::tr("Move"),
            [=]() {
                move(targets, parent);
            });

        action->setDisabled(disabled);
    };

    auto add_delete =
    [=](const bool disabled) {
        auto action = menu->addAction(QObject::tr("Delete"),
            [=]() {
                delete_object(targets, parent);
            });
        action->setDisabled(disabled);
    };

    auto add_rename =
    [=](const bool disabled) {
        auto action = menu->addAction(QObject::tr("Rename"),
            [=]() {
                rename(targets[0], parent);
            });
        action->setDisabled(disabled);
    };

    // TODO: multi-object details
    auto add_details =
    [=]() {
        menu->addAction(QObject::tr("Details"),
            [=]() {
                details(targets[0], parent);
            });
    };

    const bool single_object = (targets.size() == 1);

    // Add menu's
    if (single_object) {
        const QString target = targets[0];
        const QString target_class = target_classes.values()[0];
        const AdObject object = AD()->search_object(target);

        // Get info about object that will determine which
        // actions are present/enabled
        const bool is_container =
        [=]() {
            const QList<QString> container_classes = ADCONFIG()->get_filter_containers();

            return container_classes.contains(target_class);
        }();

        const bool is_user = (target_class == CLASS_USER);

        const bool cannot_move = object.get_system_flag(SystemFlagsBit_CannotMove);
        const bool cannot_rename = object.get_system_flag(SystemFlagsBit_CannotRename);
        const bool cannot_delete = object.get_system_flag(SystemFlagsBit_CannotDelete);

        const bool account_disabled = object.get_account_option(AccountOption_Disabled);

        // Add actions

        if (is_container) {
            add_new();
            add_find();

            menu->addSeparator();
        }

        if (is_user) {
            add_add_to_group();
            add_reset_password();

            if (account_disabled) {
                add_enable_account();
            } else {
                add_disable_account();
            }

            menu->addSeparator();
        }

        add_move(cannot_move);
        add_delete(cannot_delete);
        add_rename(cannot_rename);

        menu->addSeparator();

        add_details();
    } else if (targets.size() > 1) {
        const bool all_users = (target_classes.contains(CLASS_USER) && target_classes.size() == 1);

        if (all_users) {
            add_add_to_group();
            add_enable_account();
            add_disable_account();

            menu->addSeparator();
        }

        add_move(false);
        add_delete(false);
    }
}

void details(const QString &target, QWidget *parent) {
    // TODO: multi-object details
    DetailsDialog::open_for_target(target);
}

void delete_object(const QList<QString> targets, QWidget *parent) {
    const QString text = QString(QObject::tr("Are you sure you want to delete %1?")).arg(targets_display_string(targets));
    const bool confirmed = confirmation_dialog(text, parent);

    if (confirmed) {
        STATUS()->start_error_log();

        for (const QString target : targets) {
            AD()->object_delete(target);
        }

        STATUS()->end_error_log(parent);
    }
}

void move(const QList<QString> targets, QWidget *parent) {
    auto dialog = new SelectContainerDialog(parent);

    const QString title = QString(QObject::tr("Move %1")).arg(targets_display_string(targets));
    dialog->setWindowTitle(title);

    QObject::connect(
        dialog, &SelectContainerDialog::accepted,
        [=]() {
            const QString selected = dialog->get_selected();
            STATUS()->start_error_log();

            for (const QString target : targets) {
                AD()->object_move(target, selected);
            }

            STATUS()->end_error_log(parent);
        });

    dialog->open();
}

// TODO: aduc also includes "built-in security principals" which equates to groups that are located in builtin container. Those objects are otherwise completely identical to other group objects, same class and everything. Adding this would be convenient but also a massive PITA because that would mean making select classes widget somehow have mixed options for classes and whether parent object is the Builtin
void add_to_group(const QList<QString> targets, QWidget *parent) {
    auto dialog = new SelectDialog({CLASS_GROUP}, SelectDialogMultiSelection_Yes, parent);

    const QString title = QString(QObject::tr("Add %1 to group")).arg(targets_display_string(targets));
    dialog->setWindowTitle(title);

    QObject::connect(
        dialog, &SelectDialog::accepted,
        [=]() {
            const QList<QString> selected = dialog->get_selected();

            STATUS()->start_error_log();

            for (const QString target : targets) {
                for (auto group : selected) {
                    AD()->group_add_member(group, target);
                }
            }

            STATUS()->end_error_log(parent);
        });

    dialog->open();
}

void rename(const QString &target, QWidget *parent) {
    auto dialog = new RenameDialog(target, parent);
    dialog->open();
}

void create(const QString &target, const QString &object_class, QWidget *parent) {
    const auto create_dialog = new CreateDialog(target, object_class, parent);
    create_dialog->open();
}

void reset_password(const QString &target, QWidget *parent) {
    const auto password_dialog = new PasswordDialog(target, parent);
    password_dialog->open();
}

void enable_account(const QList<QString> targets, QWidget *parent) {
    STATUS()->start_error_log();
    
    for (const QString target : targets) {
        AD()->user_set_account_option(target, AccountOption_Disabled, false);
    }

    STATUS()->end_error_log(parent);
}

void disable_account(const QList<QString> targets, QWidget *parent) {
    STATUS()->start_error_log();
    
    for (const QString target : targets) {
        AD()->user_set_account_option(target, AccountOption_Disabled, true);
    }

    STATUS()->end_error_log(parent);
}

void find(const QString &target, QWidget *parent) {
    auto find_dialog = new FindDialog(filter_classes, target, parent);
    find_dialog->open();
}

QString targets_display_string(const QList<QString> targets) {
    if (targets.size() == 1) {
        const QString dn = targets[0];
        const QString name = dn_get_name(dn);
        return QString(QObject::tr("object \"%1\"")).arg(name);
    } else {
        return QObject::tr("multiple objects");
    }
}

void move_object(const QList<QString> targets, QWidget *parent) {
    auto dialog = new SelectContainerDialog(parent);

    const QString title = QString(QObject::tr("Move %1")).arg(targets_display_string(targets));
    dialog->setWindowTitle(title);

    QObject::connect(
        dialog, &SelectContainerDialog::accepted,
        [=]() {
            const QString selected = dialog->get_selected();
            STATUS()->start_error_log();

            for (const QString target : targets) {
                AD()->object_move(target, selected);
            }

            STATUS()->end_error_log(parent);
        });

    dialog->open();
}
