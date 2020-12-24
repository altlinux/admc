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
#include "utils.h"
#include "find_dialog.h"
#include "status.h"
#include "object_model.h"

#include <QPoint>
#include <QAbstractItemView>
#include <QDebug>

// NOTE: for dialogs opened from this menu, the parent of the menu is passed NOT the menu itself, because the menu closes (and gets deleted if this is the context menu) when dialog opens.

void ObjectMenu::setup_as_menubar_menu(QAbstractItemView *view, const int dn_column) {
    connect(
        view->selectionModel(), &QItemSelectionModel::selectionChanged,
        [=](const QItemSelection &, const QItemSelection &) {
            load_targets(view, dn_column);
        });
}

void ObjectMenu::setup_as_context_menu(QAbstractItemView *view, const int dn_column) {
    QObject::connect(
        view, &QWidget::customContextMenuRequested,
        [=](const QPoint pos) {
            ObjectMenu menu(view);
            menu.load_targets(view, dn_column);

            if (!menu.targets.isEmpty()) {
                exec_menu_from_view(&menu, view, pos);
            }
        });
}

// Load targets(and their classes), which are the selected
// objects of given view. Note that menu actions are not
// made at this point.
void ObjectMenu::load_targets(QAbstractItemView *view, const int dn_column) {
    QSet<QString> selected_dns;
    QSet<QString> classes;

    const QList<QModelIndex> indexes = view->selectionModel()->selectedIndexes();

    for (const QModelIndex index : indexes) {
        const QString &dn = get_dn_from_index(index, dn_column);
        const QString object_class = index.siblingAtColumn(0).data(ObjectModel::RawObjectClass).toString();

        selected_dns.insert(dn);
        classes.insert(object_class);
    }

    targets = selected_dns.toList();
    target_classes = classes.toList();
}

// Construct actions of the menu based on current target(s)
// NOTE: construct right before showing menu instead of in
// load_targets() because target's attributes might change
// in the span of time when target is selected and menu is
// opened. Menu needs most up-to-date target attributes to
// construct actions.
void ObjectMenu::showEvent(QShowEvent *) {
    clear();

    if (targets.isEmpty()) {
        return;
    }

    const bool one_object = (targets.size() == 1);

    // Searching object to get some attributes used for menu
    // construction
    const AdObject the_object = AD()->search_object(targets[0]);

    // NOTE: the logic for order of actions is that they are
    // arranged in order of specialization: at the bottom
    // you have actions that are common and top is for more
    // specialized actions. This is so that user can get
    // used to their locations.

    //
    // Container actions - New, Find
    //
    if (one_object) {
        const bool is_container =
        [this]() {
            const QList<QString> container_classes = ADCONFIG()->get_filter_containers();
            for (const QString object_class : container_classes) {
                if (target_classes.contains(object_class)) {
                    return true;
                }
            }
            return false;
        }();

        if (is_container) {
            QMenu *submenu_new = addMenu(tr("New"));
            static const QList<QString> create_classes = {
                CLASS_USER,
                CLASS_COMPUTER,
                CLASS_OU,
                CLASS_GROUP,
            };
            for (const auto object_class : create_classes) {
                const QString action_text = ADCONFIG()->get_class_display_name(object_class);

                submenu_new->addAction(action_text,
                    [this, object_class]() {
                        create(object_class);
                    });
            }

            addAction(tr("Find"), this, &ObjectMenu::find);

            addSeparator();
        }
    }

    //
    // User actions - Add to group, Reset password,
    // enable/disable account
    //
    const bool all_targets_are_users = (target_classes.contains(CLASS_USER) && target_classes.size() == 1);
    if (all_targets_are_users) {
        addAction(tr("Add to group"), this, &ObjectMenu::add_to_group);

        if (one_object) {
            addAction(tr("Reset password"), this, &ObjectMenu::reset_password);
        }

        auto add_disable_action =
        [this]() {
            addAction(tr("Disable account"), this, &ObjectMenu::disable_account);
        };
        auto add_enable_action =
        [this]() {
            addAction(tr("Enable account"), this, &ObjectMenu::enable_account);
        };

        // If one object, show only one applicable action.
        // If multiple, show both since so that one or the
        // other can be applied to all
        if (one_object) {
            const bool disabled = the_object.get_account_option(AccountOption_Disabled);
            if (disabled) {
                add_enable_action();
            } else {
                add_disable_action();
            }
        } else {
            add_enable_action();
            add_disable_action();
        }

        addSeparator();
    }

    //
    // Move, Delete, Rename
    //
    auto move_action = addAction(tr("Move"), this, &ObjectMenu::move);

    auto delete_action = addAction(tr("Delete"), this, &ObjectMenu::delete_object);

    if (one_object) {
        // NOTE: rename action only available for single object
        auto rename_action = addAction(tr("Rename"), this, &ObjectMenu::rename);

        // Disable move/rename/delete depending on target
        // object's system flags
        const QString target = targets[0];

        const bool cannot_move = the_object.get_system_flag(SystemFlagsBit_CannotMove);
        if (cannot_move) {
            move_action->setEnabled(false);
        }

        const bool cannot_rename = the_object.get_system_flag(SystemFlagsBit_CannotRename);
        if (cannot_rename) {
            rename_action->setEnabled(false);
        }

        const bool cannot_delete = the_object.get_system_flag(SystemFlagsBit_CannotDelete);
        if (cannot_delete) {
            delete_action->setEnabled(false);
        }
    }

    addSeparator();

    // TODO: multi-object details
    if (one_object) {
        addSeparator();

        addAction(tr("Details"), this, &ObjectMenu::details);
    }
}

void ObjectMenu::details() const {
    // TODO: multi-object details
    if (targets.size() == 1) {
        DetailsDialog::open_for_target(targets[0]);
    }
}

void ObjectMenu::delete_object() const {
    const QString text = QString(tr("Are you sure you want to delete %1?")).arg(targets_display_string());
    const bool confirmed = confirmation_dialog(text, parentWidget());

    if (confirmed) {
        STATUS()->start_error_log();

        for (const QString target : targets) {
            AD()->object_delete(target);
        }

        STATUS()->end_error_log(parentWidget());
    }
}

void ObjectMenu::move() const {
    // NOTE: object classes have "possible superiors" in schema which technically means that certain objects have certain sets of possible move targets. Going the simple route and just showing all objects that show up in container tree instead.
    const QList<QString> move_targets = ADCONFIG()->get_filter_containers();

    const QString title = QString(tr("Move %1")).arg(targets_display_string());
    const QList<QString> selected_objects = SelectDialog::open(move_targets, SelectDialogMultiSelection_No, title, parentWidget());

    if (selected_objects.size() == 1) {
        const QString container = selected_objects[0];

        STATUS()->start_error_log();

        for (const QString target : targets) {
            AD()->object_move(target, container);
        }

        STATUS()->end_error_log(parentWidget());
    }
}

void ObjectMenu::add_to_group() const {
    const QList<QString> classes = {CLASS_GROUP};
    const QString title = QString(tr("Add %1 to group")).arg(targets_display_string());
    const QList<QString> selected_objects = SelectDialog::open(classes, SelectDialogMultiSelection_Yes, title, parentWidget());

    if (selected_objects.size() > 0) {
        STATUS()->start_error_log();
        
        for (const QString target : targets) {
            for (auto group : selected_objects) {
                AD()->group_add_member(group, target);
            }
        }

        STATUS()->end_error_log(parentWidget());
    }
}

void ObjectMenu::rename() const {
    if (targets.size() == 1) {
        auto dialog = new RenameDialog(targets[0], parentWidget());
        dialog->open();
    }
}

// TODO: only open this for container-likes?
void ObjectMenu::create(const QString &object_class) const {
    if (targets.size() == 1) {
        const auto create_dialog = new CreateDialog(targets[0], object_class, parentWidget());
        create_dialog->open();
    }
}

void ObjectMenu::reset_password() const {
    if (targets.size() == 1) {
        const auto password_dialog = new PasswordDialog(targets[0], parentWidget());
        password_dialog->open();
    }
}

void ObjectMenu::enable_account() const {
    STATUS()->start_error_log();
    
    for (const QString target : targets) {
        AD()->user_set_account_option(target, AccountOption_Disabled, false);
    }

    STATUS()->end_error_log(parentWidget());
}

void ObjectMenu::disable_account() const {
    STATUS()->start_error_log();
    
    for (const QString target : targets) {
        AD()->user_set_account_option(target, AccountOption_Disabled, true);
    }

    STATUS()->end_error_log(parentWidget());
}

void ObjectMenu::find() const {
    if (targets.size() == 1) {
        auto find_dialog = new FindDialog(targets[0], parentWidget());
        find_dialog->open();
    }
}

QString ObjectMenu::targets_display_string() const {
    if (targets.size() == 1) {
        const QString dn = targets[0];
        const QString name = dn_get_name(dn);
        return QString(tr("object \"%1\"")).arg(name);
    } else {
        return tr("multiple objects");
    }
}
