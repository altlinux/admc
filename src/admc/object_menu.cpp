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

#include <QPoint>
#include <QAbstractItemView>

// NOTE: for dialogs opened from this menu, the parent of the menu is passed NOT the menu itself, because the menu closes (and gets deleted if this is the context menu) when dialog opens.

void ObjectMenu::change_target(const QString &new_target) {
    target = new_target;
}

void ObjectMenu::setup_as_menubar_menu(QAbstractItemView *view, const int dn_column) {
    connect(
        view->selectionModel(), &QItemSelectionModel::currentChanged,
        [this, dn_column](const QModelIndex &current, const QModelIndex &) {
            const QString &dn = get_dn_from_index(current, dn_column);

            change_target(dn);
        });
}

void ObjectMenu::showEvent(QShowEvent *) {
    clear();

    if (target.isEmpty()) {
        return;
    }

    const AdObject object = AD()->search_object(target);

    if (object.is_empty()) {
        return;
    }

    addAction(tr("Details"), this, &ObjectMenu::details);

    addSeparator();

    auto move_action = addAction(tr("Move"), this, &ObjectMenu::move);
    const bool cannot_move = object.get_system_flag(SystemFlagsBit_CannotMove);
    if (cannot_move) {
        move_action->setEnabled(false);
    }

    auto rename_action = addAction(tr("Rename"), this, &ObjectMenu::rename);
    const bool cannot_rename = object.get_system_flag(SystemFlagsBit_CannotRename);
    if (cannot_rename) {
        rename_action->setEnabled(false);
    }

    auto delete_action = addAction(tr("Delete"), this, &ObjectMenu::delete_object);
    const bool cannot_delete = object.get_system_flag(SystemFlagsBit_CannotDelete);
    if (cannot_delete) {
        delete_action->setEnabled(false);
    }

    addSeparator();

    const bool is_container =
    [object]() {
        const QList<QString> container_classes = ADCONFIG()->get_filter_containers();
        for (const QString object_class : container_classes) {
            if (object.is_class(object_class)) {
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

    if (object.is_class(CLASS_USER)) {
        addAction(tr("Add to group"), this, &ObjectMenu::add_to_group);
        addAction(tr("Reset password"), this, &ObjectMenu::reset_password);

        const bool disabled = object.get_account_option(AccountOption_Disabled);
        if (disabled) {
            addAction(tr("Enable account"), this, &ObjectMenu::enable_account);
        } else {
            addAction(tr("Disable account"), this, &ObjectMenu::disable_account);
        }

        addSeparator();
    }
}

void ObjectMenu::setup_as_context_menu(QAbstractItemView *view, const int dn_column) {
    QObject::connect(
        view, &QWidget::customContextMenuRequested,
        [=](const QPoint pos) {
            const QString dn = get_dn_from_pos(pos, view, dn_column);
            if (dn.isEmpty()) {
                return;
            }

            ObjectMenu menu(view);
            menu.change_target(dn);
            exec_menu_from_view(&menu, view, pos);
        });
}

void ObjectMenu::details() const {
    DetailsDialog::open_for_target(target);
}

void ObjectMenu::delete_object() const {
    const QString name = dn_get_name(target);
    const QString text = QString(tr("Are you sure you want to delete object \"%1\"?")).arg(name);
    const bool confirmed = confirmation_dialog(text, parentWidget());

    if (confirmed) {
        AD()->object_delete(target);
    }
}

void ObjectMenu::move() const {
    const AdObject object = AD()->search_object(target);

    const QList<QString> object_classes = object.get_strings(ATTRIBUTE_OBJECT_CLASS);
    const QList<QString> possible_superiors = ADCONFIG()->get_possible_superiors(object_classes);

    const QString name = dn_get_name(object.get_dn());
    const QString title = QString(tr("Move object \"%1\"")).arg(name);
    const QList<QString> selected_objects = SelectDialog::open(possible_superiors, SelectDialogMultiSelection_Yes, title, parentWidget());

    if (selected_objects.size() == 1) {
        const QString container = selected_objects[0];

        AD()->object_move(object.get_dn(), container);
    }
}

void ObjectMenu::add_to_group() const {
    const QList<QString> classes = {CLASS_GROUP};
    const QString name = dn_get_name(target);
    const QString title = QString(tr("Add object \"%1\" to group")).arg(name);
    const QList<QString> selected_objects = SelectDialog::open(classes, SelectDialogMultiSelection_Yes, title, parentWidget());

    if (selected_objects.size() > 0) {
        for (auto group : selected_objects) {
            AD()->group_add_member(group, target);
        }
    }
}

void ObjectMenu::rename() const {
    auto dialog = new RenameDialog(target, parentWidget());
    dialog->open();
}

// TODO: only open this for container-likes?
void ObjectMenu::create(const QString &object_class) const {
    const auto create_dialog = new CreateDialog(target, object_class, parentWidget());
    create_dialog->open();
}

void ObjectMenu::reset_password() const {
    const auto password_dialog = new PasswordDialog(target, parentWidget());
    password_dialog->open();
}

void ObjectMenu::enable_account() const {
    AD()->user_set_account_option(target, AccountOption_Disabled, false);
}

void ObjectMenu::disable_account() const {
    AD()->user_set_account_option(target, AccountOption_Disabled, true);
}

void ObjectMenu::find() const {
    auto find_dialog = new FindDialog(target, parentWidget());
    find_dialog->open();
}
