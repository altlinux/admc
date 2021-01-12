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

#include <QPoint>
#include <QAbstractItemView>
#include <QDebug>

// NOTE: for dialogs opened from this menu, the parent of the menu is passed NOT the menu itself, because the menu closes (and gets deleted if this is the context menu) when dialog opens.

ObjectMenu::ObjectMenu(QWidget *parent)
: QMenu(parent)
{
    // Start off disabled until menu gets a target
    setDisabled(true);
}

void ObjectMenu::setup_as_menubar_menu(QAbstractItemView *view, const int dn_column) {
    connect(
        view->selectionModel(), &QItemSelectionModel::selectionChanged,
        [=](const QItemSelection &, const QItemSelection &) {
            load_targets(view, dn_column);
        });
}

void ObjectMenu::setup_as_context_menu(QAbstractItemView *view, const int dn_column) {
    // NOTE: creating this on heap instead of stack in the slot so that menu instance and it's members are accesible in dialog "accepted" slots
    auto menu = new ObjectMenu(view);

    QObject::connect(
        view, &QWidget::customContextMenuRequested,
        [=](const QPoint pos) {
            menu->load_targets(view, dn_column);

            if (!menu->targets.isEmpty()) {
                exec_menu_from_view(menu, view, pos);
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

    setDisabled(targets.isEmpty());
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

    auto add_new =
    [this]() {
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
    };

    auto add_find =
    [this]() {
        addAction(tr("Find"), this, &ObjectMenu::find);
    };

    auto add_add_to_group =
    [this]() {
        addAction(tr("Add to group"), this, &ObjectMenu::add_to_group);
    };

    auto add_reset_password =
    [this]() {
        addAction(tr("Reset password"), this, &ObjectMenu::reset_password);
    };

    auto add_disable_account =
    [this]() {
        addAction(tr("Disable account"), this, &ObjectMenu::disable_account);
    };

    auto add_enable_account =
    [this]() {
        addAction(tr("Enable account"), this, &ObjectMenu::enable_account);
    };

    auto add_move =
    [this](const bool disabled) {
        auto action = addAction(tr("Move"), this, &ObjectMenu::move);
        action->setDisabled(disabled);
    };

    auto add_delete =
    [this](const bool disabled) {
        auto action = addAction(tr("Delete"), this, &ObjectMenu::delete_object);
        action->setDisabled(disabled);
    };

    auto add_rename =
    [this](const bool disabled) {
        auto action = addAction(tr("Rename"), this, &ObjectMenu::rename);
        action->setDisabled(disabled);
    };

    // TODO: multi-object details
    auto add_details =
    [this]() {
        addAction(tr("Details"), this, &ObjectMenu::details);
    };

    const bool single_object = (targets.size() == 1);

    if (single_object) {
        const AdObject object = AD()->search_object(targets[0]);
        const QString object_class = target_classes[0];

        // Get info about object that will determine which
        // actions are present/enabled
        const bool is_container =
        [this, object_class]() {
            const QList<QString> container_classes = ADCONFIG()->get_filter_containers();

            return container_classes.contains(object_class);
        }();

        const bool is_user = (object_class == CLASS_USER);

        const bool cannot_move = object.get_system_flag(SystemFlagsBit_CannotMove);
        const bool cannot_rename = object.get_system_flag(SystemFlagsBit_CannotRename);
        const bool cannot_delete = object.get_system_flag(SystemFlagsBit_CannotDelete);

        const bool account_disabled = object.get_account_option(AccountOption_Disabled);

        // Add actions

        if (is_container) {
            add_new();
            add_find();

            addSeparator();
        }

        if (is_user) {
            add_add_to_group();
            add_reset_password();

            if (account_disabled) {
                add_enable_account();
            } else {
                add_disable_account();
            }

            addSeparator();
        }

        add_move(cannot_move);
        add_delete(cannot_delete);
        add_rename(cannot_rename);

        addSeparator();

        add_details();
    } else {
        const bool all_users = (target_classes.contains(CLASS_USER) && target_classes.size() == 1);

        if (all_users) {
            add_add_to_group();
            add_enable_account();
            add_disable_account();

            addSeparator();
        }

        add_move(false);
        add_delete(false);
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
    auto dialog = new SelectContainerDialog(parentWidget());

    const QString title = QString(tr("Move %1")).arg(targets_display_string());
    dialog->setWindowTitle(title);

    connect(
        dialog, &SelectContainerDialog::accepted,
        [this, dialog]() {
            const QString selected = dialog->get_selected();
            STATUS()->start_error_log();

            for (const QString target : targets) {
                AD()->object_move(target, selected);
            }

            STATUS()->end_error_log(parentWidget());
        });

    dialog->open();
}

// TODO: aduc also includes "built-in security principals" which equates to groups that are located in builtin container. Those objects are otherwise completely identical to other group objects, same class and everything. Adding this would be convenient but also a massive PITA because that would mean making select classes widget somehow have mixed options for classes and whether parent object is the Builtin
void ObjectMenu::add_to_group() const {
    auto dialog = new SelectDialog({CLASS_GROUP}, SelectDialogMultiSelection_Yes, parentWidget());

    const QString title = QString(tr("Add %1 to group")).arg(targets_display_string());
    dialog->setWindowTitle(title);

    connect(
        dialog, &SelectDialog::accepted,
        [this, dialog]() {
            const QList<QString> selected = dialog->get_selected();

            STATUS()->start_error_log();

            for (const QString target : targets) {
                for (auto group : selected) {
                    AD()->group_add_member(group, target);
                }
            }

            STATUS()->end_error_log(parentWidget());
        });
}

void ObjectMenu::rename() const {
    if (targets.size() == 1) {
        auto dialog = new RenameDialog(targets[0], parentWidget());
        dialog->open();
    }
}

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
        auto find_dialog = new FindDialog(filter_classes, targets[0], parentWidget());
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
