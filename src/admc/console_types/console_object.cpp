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

#include "console_types/console_object.h"

#include "adldap.h"
#include "central_widget.h"
#include "console_actions.h"
#include "console_types/console_policy.h"
#include "filter_dialog.h"
#include "globals.h"
#include "search_thread.h"
#include "select_object_dialog.h"
#include "settings.h"
#include "status.h"
#include "utils.h"

#include <QDebug>
#include <QMenu>
#include <QSet>
#include <QStandardItemModel>

int console_object_results_id;

enum DropType {
    DropType_Move,
    DropType_AddToGroup,
    DropType_None
};

void console_object_item_data_load(QStandardItem *item, const AdObject &object);
DropType console_object_get_drop_type(const QModelIndex &dropped, const QModelIndex &target);
void disable_drag_if_object_cant_be_moved(const QList<QStandardItem *> &items, const AdObject &object);
bool console_object_scope_and_results_add_check(ConsoleWidget *console, const QModelIndex &parent);
void console_object_drop_objects(ConsoleWidget *console, const QList<QPersistentModelIndex> &dropped_list, const QPersistentModelIndex &target);
void console_object_drop_policies(ConsoleWidget *console, const QList<QPersistentModelIndex> &dropped_list, const QPersistentModelIndex &target, PolicyResultsWidget *policy_results_widget);

void console_object_results_load(const QList<QStandardItem *> row, const AdObject &object) {
    // Load attribute columns
    for (int i = 0; i < g_adconfig->get_columns().count(); i++) {
        const QString attribute = g_adconfig->get_columns()[i];

        if (!object.contains(attribute)) {
            continue;
        }

        const QString display_value = [attribute, object]() {
            if (attribute == ATTRIBUTE_OBJECT_CLASS) {
                const QString object_class = object.get_string(attribute);

                if (object_class == CLASS_GROUP) {
                    const GroupScope scope = object.get_group_scope();
                    const QString scope_string = group_scope_string(scope);

                    const GroupType type = object.get_group_type();
                    const QString type_string = group_type_string_adjective(type);

                    return QString("%1 - %2").arg(type_string, scope_string);
                } else {
                    return g_adconfig->get_class_display_name(object_class);
                }
            } else {
                const QByteArray value = object.get_value(attribute);
                return attribute_display_value(attribute, value, g_adconfig);
            }
        }();

        row[i]->setText(display_value);
    }

    console_object_item_data_load(row[0], object);

    disable_drag_if_object_cant_be_moved(row, object);
}

void console_object_item_data_load(QStandardItem *item, const AdObject &object) {
    item->setData(true, ConsoleRole_HasProperties);

    item->setData(ItemType_Object, ConsoleRole_Type);

    const QIcon icon = get_object_icon(object);
    item->setIcon(icon);

    item->setData(object.get_dn(), ObjectRole_DN);

    const QList<QString> object_classes = object.get_strings(ATTRIBUTE_OBJECT_CLASS);
    item->setData(QVariant(object_classes), ObjectRole_ObjectClasses);

    const bool cannot_move = object.get_system_flag(SystemFlagsBit_CannotMove);
    item->setData(cannot_move, ObjectRole_CannotMove);

    const bool cannot_rename = object.get_system_flag(SystemFlagsBit_CannotRename);
    item->setData(cannot_rename, ObjectRole_CannotRename);

    const bool cannot_delete = object.get_system_flag(SystemFlagsBit_CannotDelete);
    item->setData(cannot_delete, ObjectRole_CannotDelete);

    const bool account_disabled = object.get_account_option(AccountOption_Disabled, g_adconfig);
    item->setData(account_disabled, ObjectRole_AccountDisabled);
}

QList<QString> console_object_header_labels() {
    QList<QString> out;

    for (const QString &attribute : g_adconfig->get_columns()) {
        const QString attribute_display_name = g_adconfig->get_column_display_name(attribute);

        out.append(attribute_display_name);
    }

    return out;
}

QList<int> console_object_default_columns() {
    // By default show first 3 columns: name, class and
    // description
    return {0, 1, 2};
}

QList<QString> console_object_search_attributes() {
    QList<QString> attributes;

    attributes += g_adconfig->get_columns();

    // NOTE: needed for loading group type/scope into "type"
    // column
    attributes += ATTRIBUTE_GROUP_TYPE;

    // NOTE: system flags are needed to disable
    // delete/move/rename for objects that can't do those
    // actions
    attributes += ATTRIBUTE_SYSTEM_FLAGS;

    return attributes;
}

void disable_drag_if_object_cant_be_moved(const QList<QStandardItem *> &items, const AdObject &object) {
    const bool cannot_move = object.get_system_flag(SystemFlagsBit_CannotMove);

    for (auto item : items) {
        item->setDragEnabled(!cannot_move);
    }
}

// NOTE: have to search instead of just using deleted
// index because can delete objects from query tree
void console_object_delete(ConsoleWidget *console, const QList<QString> &dn_list, const bool ignore_query_tree) {
    for (const QString &dn : dn_list) {
        // Delete in scope
        const QList<QPersistentModelIndex> scope_indexes = persistent_index_list(console->search_items(ObjectRole_DN, dn, ItemType_Object));
        for (const QPersistentModelIndex &index : scope_indexes) {
            console->delete_item(index);
        }

        // Delete in results
        const QList<QPersistentModelIndex> results_indexes = persistent_index_list(console->search_items(ObjectRole_DN, dn, ItemType_Object));
        for (const QPersistentModelIndex &index : results_indexes) {
            // NOTE: don't touch query tree indexes, they
            // stay around and just go out of date
            const bool index_is_in_query_tree = [=]() {
                const QModelIndex parent = index.parent();
                const ItemType parent_type = (ItemType) parent.data(ConsoleRole_Type).toInt();

                return (parent_type == ItemType_QueryItem);
            }();

            if (index_is_in_query_tree && ignore_query_tree) {
                continue;
            }

            console->delete_item(index);
        }
    }
}

void console_object_create(ConsoleWidget *console, AdInterface &ad, const QList<QString> &dn_list, const QModelIndex &parent) {
    if (!console_object_scope_and_results_add_check(console, parent)) {
        return;
    }

    const QList<AdObject> object_list = [&]() {
        QList<AdObject> out;

        for (const QString &dn : dn_list) {
            const AdObject object = ad.search_object(dn);
            out.append(object);
        }

        return out;
    }();

    console_object_create(console, object_list, parent);
}

void console_object_move(ConsoleWidget *console, AdInterface &ad, const QList<QString> &old_dn_list, const QList<QString> &new_dn_list, const QString &new_parent_dn) {
    // NOTE: delete old item AFTER adding new item because:
    // If old item is deleted first, then it's possible for
    // new parent to get selected (if they are next to each
    // other in scope tree). Then what happens is that due
    // to new parent being selected, it gets fetched and
    // loads new object. End result is that new object is
    // duplicated.
    const QModelIndex new_parent_index = [=]() {
        const QList<QModelIndex> results = console->search_items(ObjectRole_DN, new_parent_dn, ItemType_Object);

        if (results.size() == 1) {
            return results[0];
        } else {
            return QModelIndex();
        }
    }();

    console_object_create(console, ad, new_dn_list, new_parent_index);
    console_object_delete(console, old_dn_list, true);

    console->sort_scope();
}

void console_object_move(ConsoleWidget *console, AdInterface &ad, const QList<QString> &old_dn_list, const QString &new_parent_dn) {
    const QList<QString> new_dn_list = [&]() {
        QList<QString> out;

        for (const QString &old_dn : old_dn_list) {
            const QString new_dn = dn_move(old_dn, new_parent_dn);
            out.append(new_dn);
        }

        return out;
    }();

    console_object_move(console, ad, old_dn_list, new_dn_list, new_parent_dn);
}

// Check parent index before adding objects to console
bool console_object_scope_and_results_add_check(ConsoleWidget *console, const QModelIndex &parent) {
    if (!parent.isValid()) {
        return false;
    }

    // NOTE: don't add if parent wasn't fetched yet. If that
    // is the case then the object will be added naturally
    // when parent is fetched.
    const bool parent_was_fetched = console->item_was_fetched(parent);
    if (!parent_was_fetched) {
        return false;
    }

    return true;
}

void console_object_create(ConsoleWidget *console, const QList<AdObject> &object_list, const QModelIndex &parent) {
    if (!console_object_scope_and_results_add_check(console, parent)) {
        return;
    }

    for (const AdObject &object : object_list) {
        const bool should_be_in_scope = [&]() {
            // NOTE: "containers" referenced here don't mean
            // objects with "container" object class.
            // Instead it means all the objects that can
            // have children(some of which are not
            // "container" class).
            const bool is_container = [=]() {
                const QList<QString> filter_containers = g_adconfig->get_filter_containers();
                const QString object_class = object.get_string(ATTRIBUTE_OBJECT_CLASS);

                return filter_containers.contains(object_class);
            }();

            const bool show_non_containers_ON = g_settings->get_bool(BoolSetting_ShowNonContainersInConsoleTree);

            return (is_container || show_non_containers_ON);
        }();

        const QList<QStandardItem *> row = [&]() {
            if (should_be_in_scope) {
                return console->add_scope_item(console_object_results_id, ScopeNodeType_Dynamic, parent);
            } else {
                return console->add_results_item(parent);
            }
        }();

        console_object_results_load(row, object);
    }
}

void console_object_search(ConsoleWidget *console, const QModelIndex &index, const QString &base, const SearchScope scope, const QString &filter, const QList<QString> &attributes) {
    // Save original icon and switch to a different icon
    // that will indicate that this item is being fetched.
    QStandardItem *item = console->get_scope_item(index);
    const QIcon original_icon = item->icon();
    item->setIcon(QIcon::fromTheme("system-search"));

    // NOTE: need to set this role to disable actions during
    // fetch
    console->set_item_fetching(item->index(), true);
    item->setDragEnabled(false);

    auto search_thread = new SearchThread(base, scope, filter, attributes);

    const QPersistentModelIndex persistent_index = index;

    // NOTE: need to pass console as receiver object to
    // connect() even though we're using lambda as a slot.
    // This is to be able to define queuedconnection type,
    // because there's no connect() version with no receiver
    // which has a connection type argument.
    QObject::connect(
        search_thread, &SearchThread::results_ready,
        console,
        [console, search_thread, persistent_index](const QHash<QString, AdObject> &results) {
            // NOTE: fetched index might become invalid for
            // many reasons, parent getting moved, deleted,
            // item at the index itself might get modified.
            // Since this slot runs in the main thread, it's
            // not possible for any catastrophic conflict to
            // happen, so it's enough to just stop the
            // search.
            if (!persistent_index.isValid()) {
                search_thread->stop();

                return;
            }

            console_object_create(console, results.values(), persistent_index);
            console->sort_scope();
        },
        Qt::QueuedConnection);
    QObject::connect(
        search_thread, &SearchThread::finished,
        console,
        [console, persistent_index, original_icon]() {
            if (!persistent_index.isValid()) {
                return;
            }

            QStandardItem *scope_item = console->get_scope_item(persistent_index);
            scope_item->setIcon(original_icon);

            console->set_item_fetching(scope_item->index(), false);
            scope_item->setDragEnabled(true);
        },
        Qt::QueuedConnection);

    search_thread->start();
}

// Load children of this item in scope tree
// and load results linked to this scope item
void console_object_fetch(ConsoleWidget *console, FilterDialog *filter_dialog, const QModelIndex &index) {
    const QString base = index.data(ObjectRole_DN).toString();

    const SearchScope scope = SearchScope_Children;

    //
    // Search object's children
    //
    const QString filter = [=]() {
        QString out;

        out = is_container_filter();

        // NOTE: OR user filter with containers filter so
        // that container objects are always shown, even if
        // they are filtered out by user filter
        const QString user_filter = filter_dialog->get_filter();
        out = filter_OR({user_filter, out});

        advanced_features_filter(out);
        dev_mode_filter(out);

        return out;
    }();

    const QList<QString> attributes = console_object_search_attributes();

    // NOTE: do an extra search before real search for
    // objects that should be visible in dev mode
    const bool dev_mode = g_settings->get_bool(BoolSetting_DevMode);
    if (dev_mode) {
        AdInterface ad;
        if (ad_connected(ad)) {
            QHash<QString, AdObject> results;
            dev_mode_search_results(results, ad, base);

            console_object_create(console, results.values(), index);
            console->sort_scope();
        }
    }

    console_object_search(console, index, base, scope, filter, attributes);
}

QStandardItem *console_object_tree_init(ConsoleWidget *console, AdInterface &ad) {
    // Create tree head
    QStandardItem *top_item = console->add_top_item(console_object_results_id, ScopeNodeType_Dynamic);

    const QString top_dn = g_adconfig->domain_head();
    const AdObject top_object = ad.search_object(top_dn);
    console_object_item_data_load(top_item, top_object);
    console_object_load_domain_head_text(top_item);

    return top_item;
}

void console_object_can_drop(const QList<QPersistentModelIndex> &dropped_list, const QPersistentModelIndex &target, const QSet<ItemType> &dropped_types, bool *ok) {
    const bool dropped_are_all_objects = (dropped_types.size() == 1 && dropped_types.contains(ItemType_Object));
    const bool dropped_are_policies = (dropped_types == QSet<ItemType>({ItemType_Policy}));

    if (dropped_are_all_objects) {
        // NOTE: always allow dropping when dragging multiple
        // objects. This way, whatever objects can drop will be
        // dropped and if others fail to drop it's not a big
        // deal.
        if (dropped_list.size() != 1) {
            *ok = true;
        } else {
            const QPersistentModelIndex dropped = dropped_list[0];

            const DropType drop_type = console_object_get_drop_type(dropped, target);
            const bool can_drop = (drop_type != DropType_None);

            *ok = can_drop;
        }
    } else if (dropped_are_policies) {
        const bool target_is_ou = console_object_is_ou(target);

        *ok = target_is_ou;
    }
}

void console_object_drop(ConsoleWidget *console, const QList<QPersistentModelIndex> &dropped_list, const QSet<ItemType> &dropped_types, const QPersistentModelIndex &target, PolicyResultsWidget *policy_results_widget) {
    const bool dropped_are_all_objects = (dropped_types.size() == 1 && dropped_types.contains(ItemType_Object));
    const bool dropped_are_policies = (dropped_types == QSet<ItemType>({ItemType_Policy}));

    if (dropped_are_all_objects) {
        console_object_drop_objects(console, dropped_list, target);
    } else if (dropped_are_policies) {
        console_object_drop_policies(console, dropped_list, target, policy_results_widget);
    }
}

// Determine what kind of drop type is dropping this object
// onto target. If drop type is none, then can't drop this
// object on this target.
DropType console_object_get_drop_type(const QModelIndex &dropped, const QModelIndex &target) {
    const bool dropped_is_target = [&]() {
        const QString dropped_dn = dropped.data(ObjectRole_DN).toString();
        const QString target_dn = target.data(ObjectRole_DN).toString();

        return (dropped_dn == target_dn);
    }();

    const QList<QString> dropped_classes = dropped.data(ObjectRole_ObjectClasses).toStringList();
    const QList<QString> target_classes = target.data(ObjectRole_ObjectClasses).toStringList();

    const bool dropped_is_user = dropped_classes.contains(CLASS_USER);
    const bool dropped_is_group = dropped_classes.contains(CLASS_GROUP);
    const bool target_is_group = target_classes.contains(CLASS_GROUP);
    const bool target_is_fetching = console_get_item_fetching(target);

    if (dropped_is_target || target_is_fetching) {
        return DropType_None;
    } else if (dropped_is_user && target_is_group) {
        return DropType_AddToGroup;
    } else if (dropped_is_group && target_is_group) {
        return DropType_AddToGroup;
    } else {
        const QList<QString> dropped_superiors = g_adconfig->get_possible_superiors(dropped_classes);

        const bool target_is_valid_superior = [&]() {
            for (const auto &object_class : dropped_superiors) {
                if (target_classes.contains(object_class)) {
                    return true;
                }
            }

            return false;
        }();

        if (target_is_valid_superior) {
            return DropType_Move;
        } else {
            return DropType_None;
        }
    }
}

void console_object_actions_add_to_menu(ConsoleActions *actions, QMenu *menu) {
    // Container
    menu->addAction(actions->get(ConsoleAction_Find));

    menu->addSeparator();

    // User
    menu->addAction(actions->get(ConsoleAction_AddToGroup));
    menu->addAction(actions->get(ConsoleAction_Enable));
    menu->addAction(actions->get(ConsoleAction_Disable));
    menu->addAction(actions->get(ConsoleAction_ResetPassword));

    // Other
    menu->addAction(actions->get(ConsoleAction_EditUpnSuffixes));
    menu->addAction(actions->get(ConsoleAction_ChangeDC));

    menu->addSeparator();

    // General object
    menu->addAction(actions->get(ConsoleAction_Delete));
    menu->addAction(actions->get(ConsoleAction_Rename));
    menu->addAction(actions->get(ConsoleAction_Move));
}

void console_object_actions_get_state(const QModelIndex &index, const bool single_selection, QSet<ConsoleAction> *visible_actions, QSet<ConsoleAction> *disabled_actions) {
    const ItemType type = (ItemType) index.data(ConsoleRole_Type).toInt();
    if (type != ItemType_Object) {
        return;
    }

    const QString object_class = index.data(ObjectRole_ObjectClasses).toStringList().last();

    const bool is_container = [=]() {
        const QList<QString> container_classes = g_adconfig->get_filter_containers();

        return container_classes.contains(object_class);
    }();

    const bool is_user = (object_class == CLASS_USER);
    const bool is_domain = (object_class == CLASS_DOMAIN);

    const bool cannot_move = index.data(ObjectRole_CannotMove).toBool();
    const bool cannot_rename = index.data(ObjectRole_CannotRename).toBool();
    const bool cannot_delete = index.data(ObjectRole_CannotDelete).toBool();
    const bool account_disabled = index.data(ObjectRole_AccountDisabled).toBool();

    QSet<ConsoleAction> my_visible_actions;

    if (single_selection) {
        // Single selection only
        if (is_container) {
            my_visible_actions.insert(ConsoleAction_NewUser);
            my_visible_actions.insert(ConsoleAction_NewComputer);
            my_visible_actions.insert(ConsoleAction_NewOU);
            my_visible_actions.insert(ConsoleAction_NewGroup);

            my_visible_actions.insert(ConsoleAction_Find);
        }

        if (is_user) {
            my_visible_actions.insert(ConsoleAction_ResetPassword);

            if (account_disabled) {
                my_visible_actions.insert(ConsoleAction_Enable);
            } else {
                my_visible_actions.insert(ConsoleAction_Disable);
            }
        }

        if (is_domain) {
            my_visible_actions.insert(ConsoleAction_EditUpnSuffixes);
            my_visible_actions.insert(ConsoleAction_ChangeDC);
        }

        my_visible_actions.insert(ConsoleAction_Rename);

        if (cannot_move) {
            disabled_actions->insert(ConsoleAction_Move);
        }
        if (cannot_delete) {
            disabled_actions->insert(ConsoleAction_Delete);
        }
        if (cannot_rename) {
            disabled_actions->insert(ConsoleAction_Rename);
        }
    } else {
        // Multi selection only
        if (is_user) {
            my_visible_actions.insert(ConsoleAction_Enable);
            my_visible_actions.insert(ConsoleAction_Disable);
        }
    }

    // Single OR multi selection
    if (is_user) {
        my_visible_actions.insert(ConsoleAction_AddToGroup);
    }

    my_visible_actions.insert(ConsoleAction_Move);
    my_visible_actions.insert(ConsoleAction_Delete);

    visible_actions->unite(my_visible_actions);

    const bool is_fetching = console_get_item_fetching(index);
    if (is_fetching) {
        disabled_actions->unite(my_visible_actions);
    }
}

QList<QString> object_operation_delete(const QList<QString> &targets, QWidget *parent) {
    if (targets.size() == 0) {
        return QList<QString>();
    }

    const bool confirmed = confirmation_dialog(QCoreApplication::translate("ConsoleActions", "Are you sure you want to delete this object?"), parent);
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

QList<QString> object_operation_set_disabled(const QList<QString> &targets, const bool disabled, QWidget *parent) {
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

void object_operation_add_to_group(const QList<QString> &targets, QWidget *parent) {
    auto dialog = new SelectObjectDialog({CLASS_GROUP}, SelectObjectDialogMultiSelection_Yes, parent);

    QObject::connect(
        dialog, &SelectObjectDialog::accepted,
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

bool console_object_is_ou(const QModelIndex &index) {
    const QList<QString> classes = index.data(ObjectRole_ObjectClasses).toStringList();
    const bool is_ou = classes.contains(CLASS_OU);

    return is_ou;
}

void console_object_drop_objects(ConsoleWidget *console, const QList<QPersistentModelIndex> &dropped_list, const QPersistentModelIndex &target) {
    const QString target_dn = target.data(ObjectRole_DN).toString();

    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    show_busy_indicator();

    for (const QPersistentModelIndex &dropped : dropped_list) {
        const QString dropped_dn = dropped.data(ObjectRole_DN).toString();
        const DropType drop_type = console_object_get_drop_type(dropped, target);

        switch (drop_type) {
            case DropType_Move: {
                const bool move_success = ad.object_move(dropped_dn,
                    target_dn);

                if (move_success) {
                    console_object_move(console, ad, QList<QString>({dropped_dn}), target_dn);
                }

                break;
            }
            case DropType_AddToGroup: {
                ad.group_add_member(target_dn, dropped_dn);

                break;
            }
            case DropType_None: {
                break;
            }
        }
    }

    console->sort_scope();

    hide_busy_indicator();

    g_status()->display_ad_messages(ad, console);
}

void console_object_drop_policies(ConsoleWidget *console, const QList<QPersistentModelIndex> &dropped_list, const QPersistentModelIndex &target, PolicyResultsWidget *policy_results_widget) {
    const QList<QString> policy_list = [&]() {
        QList<QString> out;

        for (const QPersistentModelIndex &index : dropped_list) {
            const QString dn = index.data(PolicyRole_DN).toString();
            out.append(dn);
        }

        return out;
    }();

    const QString target_dn = target.data(ObjectRole_DN).toString();
    const QList<QString> ou_list = {target_dn};

    console_policy_add_link(console, policy_list, ou_list, policy_results_widget);
}

void console_object_load_domain_head_text(QStandardItem *item) {
    const QString domain_head = g_adconfig->domain().toLower();
    const QString dc = AdInterface::get_dc();
    const QString domain_text = QString("%1 [%2]").arg(domain_head, dc);

    item->setText(domain_text);
}
