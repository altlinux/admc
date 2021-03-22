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
#include "ad/ad_interface.h"
#include "ad/ad_config.h"
#include "ad/ad_utils.h"
#include "ad/ad_object.h"
#include "select_dialog.h"
#include "rename_dialog.h"
#include "password_dialog.h"
#include "create_dialog.h"
#include "properties_dialog.h"
#include "select_container_dialog.h"
#include "utils.h"
#include "find_dialog.h"
#include "ad/ad_filter.h"
#include "status.h"
#include "object_model.h"

#include <QMenu>
#include <QPoint>
#include <QAbstractItemView>
#include <QDebug>
#include <QCoreApplication>

// NOTE: for dialogs opened from this menu, the parent of
// the menu is passed NOT the menu itself, because the menu
// closes (and gets deleted if this is the context menu)
// when dialog opens.

QAction *add_object_actions_to_menu(QMenu *menu, const QList<QModelIndex> &selected_indexes, QWidget *parent, const bool include_find_action, const ObjectMenuData &data) {
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

    // These are f-ns that add menu's
    auto add_new =
    [=]() {
        if (data.new_menu != nullptr) {
            menu->addMenu(data.new_menu);

            return;
        }

        QMenu *submenu_new = menu->addMenu(QCoreApplication::translate("object_menu", "New"));
        static const QList<QString> create_classes = {
            CLASS_USER,
            CLASS_COMPUTER,
            CLASS_OU,
            CLASS_GROUP,
        };
        for (const auto &object_class : create_classes) {
            const QString action_text = ADCONFIG()->get_class_display_name(object_class);

            submenu_new->addAction(action_text,
                [=]() {
                    create(targets[0], object_class, parent);
                });
        }
    };

    auto add_find =
    [=]() {
        menu->addAction(QCoreApplication::translate("object_menu", "Find"),
            [=]() {
                find(targets[0], parent);
            });
    };

    auto add_add_to_group =
    [=]() {
        menu->addAction(QCoreApplication::translate("object_menu", "Add to group"),
            [=]() {
                add_to_group(targets, parent);
            });
    };

    auto add_reset_password =
    [=]() {
        menu->addAction(QCoreApplication::translate("object_menu", "Reset password"),
            [=]() {
                reset_password(targets[0], parent);
            });
    };

    auto add_disable_account =
    [=]() {
        menu->addAction(QCoreApplication::translate("object_menu", "Disable account"),
            [=]() {
                disable_account(targets, parent);
            });
    };

    auto add_enable_account =
    [=]() {
        menu->addAction(QCoreApplication::translate("object_menu", "Enable account"),
            [=]() {
                enable_account(targets, parent);
            });
    };

    auto add_move =
    [=](const bool disabled) {
        auto action =
        [=]() {
            if (data.move != nullptr) {
                menu->addAction(data.move);
                
                return data.move;
            } else {
                return menu->addAction(QCoreApplication::translate("object_menu", "Move"),
                    [=]() {
                        move(targets, parent);
                    });
            }
        }();

        action->setDisabled(disabled);
    };

    auto add_delete =
    [=](const bool disabled) {
        auto action =
        [=]() {
            if (data.delete_object != nullptr) {
                menu->addAction(data.delete_object);

                return data.delete_object;
            } else {
                return menu->addAction(QCoreApplication::translate("object_menu", "Delete"),
                    [=]() {
                        delete_object(targets, parent);
                    });
            }
        }();

        action->setDisabled(disabled);
    };

    auto add_rename =
    [=](const bool disabled) {
        auto action =
        [=]() {
            if (data.rename != nullptr) {
                menu->addAction(data.rename);

                return data.rename;
            } else {
                return menu->addAction(QCoreApplication::translate("object_menu", "Rename"),
                    [=]() {
                        rename(targets[0], parent);
                    });
            }
        }();
        action->setDisabled(disabled);
    };

    // TODO: multi-object properties
    auto add_properties =
    [=]() {
        if (!data.properties_already_added) {
            menu->addAction(PropertiesDialog::display_name(),
                [=]() {
                    properties(targets[0], parent);
                });
        }
    };

    const bool single_object = (targets.size() == 1);

    menu->addSeparator();

    // Add menu's
    if (single_object) {
        // TODO: handle error
        AdInterface ad;
        if (ad_failed(ad)) {
            return nullptr;
        }

        const QString target = targets[0];
        const QString target_class = target_classes.values()[0];
        const AdObject object = ad.search_object(target);

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

            if (include_find_action) {
                add_find();
            }

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

        QAction *properties_separator = menu->addSeparator();

        add_properties();

        return properties_separator;
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

    return nullptr;
}

void properties(const QString &target, QWidget *parent) {
    // TODO: multi-object properties
    PropertiesDialog::open_for_target(target);
}

void delete_object(const QList<QString> targets, QWidget *parent) {
    const QString text = QString(QCoreApplication::translate("object_menu", "Are you sure you want to delete this object?"));
    const bool confirmed = confirmation_dialog(text, parent);

    if (confirmed) {
        AdInterface ad;

        if (ad_connected(ad)) {
            for (const QString &target : targets) {
                ad.object_delete(target);
            }

            STATUS()->display_ad_messages(ad, parent);
        }
    }
}

void move(const QList<QString> targets, QWidget *parent) {
    auto dialog = new SelectContainerDialog(parent);

    QObject::connect(
        dialog, &SelectContainerDialog::accepted,
        [=]() {
            const QString selected = dialog->get_selected();

            AdInterface ad;
            if (ad_connected(ad)) {
                for (const QString &target : targets) {
                    ad.object_move(target, selected);
                }

                STATUS()->display_ad_messages(ad, parent);
            }
        });

    dialog->open();
}

// TODO: aduc also includes "built-in security principals" which equates to groups that are located in builtin container. Those objects are otherwise completely identical to other group objects, same class and everything. Adding this would be convenient but also a massive PITA because that would mean making select classes widget somehow have mixed options for classes and whether parent object is the Builtin
void add_to_group(const QList<QString> targets, QWidget *parent) {
    auto dialog = new SelectDialog({CLASS_GROUP}, SelectDialogMultiSelection_Yes, parent);

    QObject::connect(
        dialog, &SelectDialog::accepted,
        [=]() {
            const QList<QString> selected = dialog->get_selected();

            AdInterface ad;
            if (ad_connected(ad)) {
                for (const QString &target : targets) {
                    for (auto group : selected) {
                        ad.group_add_member(group, target);
                    }
                }

                STATUS()->display_ad_messages(ad, parent);
            }
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
    AdInterface ad;
    if (ad_connected(ad)) {
        for (const QString &target : targets) {
            ad.user_set_account_option(target, AccountOption_Disabled, false);
        }

        STATUS()->display_ad_messages(ad, parent);
    }
}

void disable_account(const QList<QString> targets, QWidget *parent) {
    AdInterface ad;
    if (ad_connected(ad)) {
        for (const QString &target : targets) {
            ad.user_set_account_option(target, AccountOption_Disabled, true);
        }

        STATUS()->display_ad_messages(ad, parent);
    }
}

void find(const QString &target, QWidget *parent) {
    auto find_dialog = new FindDialog(filter_classes, target, parent);
    find_dialog->open();
}

void move_object(const QList<QString> targets, QWidget *parent) {
    auto dialog = new SelectContainerDialog(parent);

    QObject::connect(
        dialog, &SelectContainerDialog::accepted,
        [=]() {
            const QString selected = dialog->get_selected();

            AdInterface ad;
            if (ad_connected(ad)) {
                for (const QString &target : targets) {
                    ad.object_move(target, selected);
                }

                STATUS()->display_ad_messages(ad, parent);
            }
        });

    dialog->open();
}
