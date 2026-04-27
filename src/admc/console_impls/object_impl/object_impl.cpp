/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#include "console_impls/object_impl/object_impl.h"

#include "adldap.h"
#include "attribute_dialogs/list_attribute_dialog.h"
#include "console_filter_dialog.h"
#include "console_impls/find_object_impl.h"
#include "console_impls/item_type.h"
#include "console_impls/policy_ou_impl.h"
#include "console_impls/policy_root_impl.h"
#include "console_impls/query_folder_impl.h"
#include "console_impls/query_item_impl.h"
#include "console_widget/results_view.h"
#include "globals.h"
#include "password_dialog.h"
#include "select_dialogs/select_container_dialog.h"
#include "select_dialogs/select_object_dialog.h"
#include "find_widgets/find_object_dialog.h"
#include "settings.h"
#include "status.h"
#include "utils.h"
#include "tabs/general_user_tab.h"
#include "tabs/general_group_tab.h"
#include "managers/icon_manager.h"
#include "results_widgets/pso_results_widget/pso_results_widget.h"
#include "results_widgets/subnet_results_widget/subnet_results_widget.h"

#include <QDebug>
#include <QMenu>
#include <QSet>
#include <QStandardItemModel>
#include <QStackedWidget>
#include <QMessageBox>

#include <algorithm>
#include "drag_n_drop.h"


ObjectImpl::ObjectImpl(ConsoleWidget *console_arg)
: ConsoleImpl(console_arg) {
    console_list = {
        console,
    };

    setup_widgets();

    setup_filters();

    setup_actions();
}

void ObjectImpl::set_buddy_console(ConsoleWidget *buddy_console) {
    console_list = {
        console,
        buddy_console,
    };
}

// Load children of this item in scope tree
// and load results linked to this scope item
void ObjectImpl::fetch(const QModelIndex &index) {
    const QString base = index.data(ObjectRole_DN).toString();

    const SearchScope scope = SearchScope_Children;

    //
    // Search object's children
    //
    const QString filter = [=]() {
        QString out;

        // NOTE: OR user filter with containers filter so
        // that container objects are always shown, even if
        // they are filtered out by user filter
        if (object_filter_enabled) {
            out = filter_OR({is_container_filter(), out});
            out = filter_OR({object_filter, out});
        }

        const QString fetched_obj_class = index.data(ObjectRole_ObjectClasses).toStringList().last();
        // Disable advances features option for site-related and pso objects
        if (!g_adconfig->get_site_related_classes().contains(fetched_obj_class) &&
                fetched_obj_class != CLASS_PSO_CONTAINER) {
            out = advanced_features_filter(out);
        }

        return out;
    }();

    const QList<QString> attributes = ConsoleObjectTreeOperations::console_object_search_attributes();

    // NOTE: do an extra search before real search for
    // objects that should be visible in dev mode
    const bool dev_mode = settings_get_variant(SETTING_feature_dev_mode).toBool();
    if (dev_mode) {
        AdInterface ad;
        if (ad_connected(ad, console)) {
            QHash<QString, AdObject> results;
            dev_mode_search_results(results, ad, base);

            ConsoleObjectTreeOperations::add_objects_to_console(console, results.values(), index);
        }
    }

    ConsoleObjectTreeOperations::console_object_search(console, index, base, scope, filter, attributes);
}

bool ObjectImpl::can_drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type) {
    UNUSED_ARG(target_type);

    const bool dropped_are_all_objects = (dropped_type_list == QSet<int>({ItemType_Object}));

    if (dropped_are_all_objects) {
        if (dropped_list.size() == 1) {
            const QPersistentModelIndex dropped = dropped_list[0];

            const ObjectDragDrop::DropType drop_type = ObjectDragDrop::console_object_get_drop_type(dropped, target);
            const bool can_drop = (drop_type != ObjectDragDrop::DropType_None);

            return can_drop;
        } else {
            // NOTE: always allow dropping when dragging
            // multiple objects. This way, whatever objects
            // can drop will be dropped and if others fail
            // to drop it's not a big deal.
            return true;
        }
    } else {
        return false;
    }
}

void ObjectImpl::drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type) {
    UNUSED_ARG(target_type);
    UNUSED_ARG(dropped_type_list);

    const QString target_dn = target.data(ObjectRole_DN).toString();

    AdInterface ad;
    if (ad_failed(ad, console)) {
        return;
    }

    show_busy_indicator();

    for (const QPersistentModelIndex &dropped : dropped_list) {
        const QString dropped_dn = dropped.data(ObjectRole_DN).toString();
        const ObjectDragDrop::DropType drop_type = ObjectDragDrop::console_object_get_drop_type(dropped, target);

        switch (drop_type) {
            case ObjectDragDrop::DropType_Move: {
                const bool move_success = ad.object_move(dropped_dn,
                    target_dn);

                if (move_success) {
                    move(ad, QList<QString>({dropped_dn}), target_dn);
                }

                break;
            }
            case ObjectDragDrop::DropType_AddToGroup: {
                ad.group_add_member(target_dn, dropped_dn);

                break;
            }
            case ObjectDragDrop::DropType_None: {
                break;
            }
        }
    }

    hide_busy_indicator();

    g_status->display_ad_messages(ad, console);
}

QString ObjectImpl::get_description(const QModelIndex &index) const {
    QString out;

    const QString object_count_text = ConsoleObjectTreeOperations::console_object_count_string(console, index);

    out += object_count_text;

    if (object_filter_enabled) {
        out += tr(" [Filtering enabled]");
    }

    return out;
}

void ObjectImpl::activate(const QModelIndex &index) {
    properties({index});
}

QList<QAction *> ObjectImpl::get_all_custom_actions() const {
    QList<QAction *> out = {
        new_action,
        find_action,
        add_to_group_action,
        enable_action,
        disable_action,
        reset_password_action,
        reset_account_action,
        edit_upn_suffixes_action,
        move_action,
        create_pso_action,
        create_subnet_action,
        create_site_action,
        create_site_link_action,
        create_site_link_bridge_action
    };

    return out;
}

QSet<QAction *> ObjectImpl::get_custom_actions(const QModelIndex &index, const bool single_selection) const {
    QSet<QAction *> out;

    const QString object_class = index.data(ObjectRole_ObjectClasses).toStringList().last();

    const bool is_container = [=]() {
        const QList<QString> container_classes = g_adconfig->get_filter_containers();

        return container_classes.contains(object_class);
    }();

    const bool is_user = (object_class == CLASS_USER);
    const bool is_group = (object_class == CLASS_GROUP);
    const bool is_domain = (object_class == CLASS_DOMAIN);
    const bool is_computer = (object_class == CLASS_COMPUTER);
    const bool is_pso_container = (object_class == CLASS_PSO_CONTAINER);
    const bool is_sites_container = (object_class == CLASS_SITES_CONTAINER);
    const bool is_subnet_container = (object_class == CLASS_SUBNET_CONTAINER);
    const bool is_site_links_container = (object_class == CLASS_INTER_SITE_TRANSPORT);


    const bool account_disabled = index.data(ObjectRole_AccountDisabled).toBool();

    if (single_selection) {
        // Single selection only
        if (is_container) {
            out.insert(new_action);

            if (find_action_enabled) {
                out.insert(find_action);
            }
        }

        if (is_user) {
            out.insert(reset_password_action);
        }

        if (is_user || is_computer) {
            if (account_disabled) {
                out.insert(enable_action);
            } else {
                out.insert(disable_action);
            }
        }

        if (is_computer) {
            out.insert(reset_account_action);
        }

        if (is_domain) {
            out.insert(edit_upn_suffixes_action);
        }

        if (is_pso_container) {
            out.insert(create_pso_action);
        }

        if (is_sites_container) {
            out.insert(create_site_action);
        }

        if (is_subnet_container) {
            out.insert(create_subnet_action);
        }

        if (is_site_links_container) {
            out.insert(create_site_link_action);
            out.insert(create_site_link_bridge_action);
        }

    } else {
        // Multi selection only
        if (is_user) {
            out.insert(enable_action);
            out.insert(disable_action);
        }

        if (is_computer) {
            out.insert(reset_account_action);
        }
    }

    // Single OR multi selection
    if (is_user || is_group) {
        out.insert(add_to_group_action);
    }

    out.insert(move_action);

    // NOTE: have to manually call setVisible here
    // because "New" actions are contained inside "New"
    // sub-menu, so console widget can't manage them
    for (const QString &action_object_class : standard_create_action_map.keys()) {
        QAction *action = standard_create_action_map[action_object_class];

        const bool is_visible = can_create_class_at_parent(action_object_class, object_class);
        action->setVisible(is_visible);
    }

    return out;
}

QSet<QAction *> ObjectImpl::get_disabled_custom_actions(const QModelIndex &index, const bool single_selection) const {
    UNUSED_ARG(single_selection);

    QSet<QAction *> out;

    const QString object_class = index.data(ObjectRole_ObjectClasses).toStringList().last();
    const bool cannot_move = index.data(ObjectRole_CannotMove).toBool();
    const QList<QString> not_movable_obj_classes = {
        CLASS_PSO,
        CLASS_SITE,
        CLASS_SUBNET,
        CLASS_SITE_LINK,
        CLASS_SITE_LINK_BRIDGE,
        CLASS_SITES_CONTAINER,
        CLASS_INTER_SITE_TRANSPORT,
        CLASS_INTER_SITE_TRANSPORT_CONTAINER,
        CLASS_SUBNET_CONTAINER
    };

    if (cannot_move || not_movable_obj_classes.contains(object_class)) {
        out.insert(move_action);
    }

    return out;
}

QSet<StandardAction> ObjectImpl::get_standard_actions(const QModelIndex &index, const bool single_selection) const {
    QSet<StandardAction> out;

    out.insert(StandardAction_Properties);

    // NOTE: only add refresh action if item was fetched,
    // this filters out all the objects like users that
    // should never get refresh action
    const bool can_refresh = console_item_get_was_fetched(index);
    if (can_refresh && single_selection && refresh_action_enabled) {
        out.insert(StandardAction_Refresh);
    }

    const bool can_rename = [&]() {
        const QList<QString> renamable_class_list = {
            CLASS_USER,
            CLASS_GROUP,
            CLASS_OU,
            CLASS_SITE
        };
        const QString object_class = index.data(ObjectRole_ObjectClasses).toStringList().last();
        const bool can_rename_out = (single_selection && renamable_class_list.contains(object_class));

        return can_rename_out;
    }();

    if (can_rename) {
        out.insert(StandardAction_Rename);
    }

    out.insert(StandardAction_Delete);

    return out;
}

QSet<StandardAction> ObjectImpl::get_disabled_standard_actions(const QModelIndex &index, const bool single_selection) const {
    UNUSED_ARG(single_selection);

    QSet<StandardAction> out;

    const bool cannot_rename = index.data(ObjectRole_CannotRename).toBool();
    const bool cannot_delete = index.data(ObjectRole_CannotDelete).toBool();

    if (cannot_rename) {
        out.insert(StandardAction_Rename);
    }

    if (cannot_delete) {
        out.insert(StandardAction_Delete);
    }

    return out;
}

void ObjectImpl::rename(const QList<QModelIndex> &index_list) {
    const QModelIndex index = index_list[0];
    const QString object_class = index.data(ObjectRole_ObjectClasses).toStringList().last();

    ConsoleObjectTreeOperations::console_object_rename(console_list, index_list, ObjectRole_DN, object_class);
}

void ObjectImpl::properties(const QList<QModelIndex> &index_list) {
    const QList<QString> class_list = [&]() {
        QSet<QString> out;

        for (const QModelIndex &index : index_list) {
            const QList<QString> this_class_list = index.data(ObjectRole_ObjectClasses).toStringList();
            const QString main_class = this_class_list.last();
            out.insert(main_class);
        }

        return QList<QString>(out.begin(), out.end());
    }();

    ConsoleObjectTreeOperations::console_object_properties(console_list, index_list, ObjectRole_DN, class_list);
}

void ObjectImpl::refresh(const QList<QModelIndex> &index_list) {
    if (index_list.size() != 1) {
        return;
    }

    const QModelIndex index = index_list[0];

    console->delete_children(index);
    fetch(index);

    update_results_widget(index);
}

void ObjectImpl::delete_action(const QList<QModelIndex> &index_list) {
    ConsoleObjectTreeOperations::console_object_delete(console_list, index_list, ObjectRole_DN);
}

void ObjectImpl::selected_as_scope(const QModelIndex &index)
{
    AdInterface ad;
    if (ad_failed(ad, console)) {
        return;
    }

    const QString dn = index.data(ObjectRole_DN).toString();
    const AdObject object = ad.search_object(dn);

    if (object.is_class(CLASS_GROUP)) {
        stacked_widget->setCurrentWidget(group_results_widget);
        group_results_widget->update(ad, object);
    }
    else if (object.is_class(CLASS_CONTACT) ||
             object.is_class(CLASS_USER) ||
             object.is_class(CLASS_INET_ORG_PERSON)) {
        stacked_widget->setCurrentWidget(user_results_widget);
        user_results_widget->update(ad, object);
    }
    else if (object.is_class(CLASS_PSO)) {
        stacked_widget->setCurrentWidget(pso_results_widget);
        pso_results_widget->update(object);
    }
    else if (object.is_class(CLASS_SUBNET)) {
        stacked_widget->setCurrentWidget(subnet_results_widget);
        subnet_results_widget->update(object);
    }
    else {
        stacked_widget->setCurrentWidget(view());
    }
}

void ObjectImpl::update_results_widget(const QModelIndex &index) const {
    const QStringList index_data_classes = index.data(ObjectRole_ObjectClasses).toStringList();

    if (!(index_data_classes.contains(CLASS_GROUP) || index_data_classes.contains(CLASS_CONTACT) ||
          index_data_classes.contains(CLASS_USER) || index_data_classes.contains(CLASS_INET_ORG_PERSON) ||
          index_data_classes.contains(CLASS_PSO) || index_data_classes.contains(CLASS_SUBNET))) {
            return;
    }

    AdInterface ad;
    if (ad_failed(ad, console)) {
        return;
    }

    const QString dn = index.data(ObjectRole_DN).toString();
    const AdObject object = ad.search_object(dn);

    if (object.is_class(CLASS_GROUP)) {
        group_results_widget->update(ad, object);
        return;
    }

    if (object.is_class(CLASS_CONTACT) ||
             object.is_class(CLASS_USER) ||
             object.is_class(CLASS_INET_ORG_PERSON)) {
        user_results_widget->update(ad, object);
        return;
    }

    if (object.is_class(CLASS_PSO)) {
        pso_results_widget->update(object);
        return;
    }

    if (object.is_class(CLASS_SUBNET)) {
        subnet_results_widget->update(object);
        return;
    }
}

void ObjectImpl::set_find_action_enabled(const bool enabled) {
    find_action_enabled = enabled;
}

void ObjectImpl::set_refresh_action_enabled(const bool enabled) {
    refresh_action_enabled = enabled;
}

void ObjectImpl::set_toolbar_actions(QAction *toolbar_create_user_arg, QAction *toolbar_create_group_arg, QAction *toolbar_create_ou_arg) {
    toolbar_create_user = toolbar_create_user_arg;
    toolbar_create_group = toolbar_create_group_arg;
    toolbar_create_ou = toolbar_create_ou_arg;

    connect(
        toolbar_create_user, &QAction::triggered,
        this, &ObjectImpl::on_new_user);
    connect(
        toolbar_create_group, &QAction::triggered,
        this, &ObjectImpl::on_new_group);
    connect(
        toolbar_create_ou, &QAction::triggered,
        this, &ObjectImpl::on_new_ou);
}

QList<QString> ObjectImpl::column_labels() const {
    return ConsoleObjectTreeOperations::object_impl_column_labels();
}

QList<int> ObjectImpl::default_columns() const {
    return ConsoleObjectTreeOperations::object_impl_default_columns();
}

void ObjectImpl::refresh_tree() {
    const QModelIndex object_tree_root = ConsoleObjectTreeOperations::get_domain_object_tree_root(console);
    if (!object_tree_root.isValid()) {
        return;
    }

    show_busy_indicator();

    console->refresh_scope(object_tree_root);

    hide_busy_indicator();
}

void ObjectImpl::open_console_filter_dialog() {
    auto dialog = new ConsoleFilterDialog(console);

    const QVariant dialog_state = settings_get_variant(SETTING_console_filter_dialog_state);
    dialog->restore_state(dialog_state);

    dialog->open();

    connect(
        dialog, &QDialog::accepted,
        this,
        [this, dialog]() {
            object_filter = dialog->get_filter();
            object_filter_enabled = dialog->get_filter_enabled();

            settings_set_variant(SETTING_object_filter, object_filter);
            settings_set_variant(SETTING_object_filter_enabled, object_filter_enabled);

            settings_set_variant(SETTING_console_filter_dialog_state, dialog->save_state());

            refresh_tree();
        });
}

void ObjectImpl::on_new_user() {
    new_object(CLASS_USER);
}

void ObjectImpl::on_new_computer() {
    new_object(CLASS_COMPUTER);
}

void ObjectImpl::on_new_ou() {
    new_object(CLASS_OU);
}

void ObjectImpl::on_new_group() {
    new_object(CLASS_GROUP);
}

void ObjectImpl::on_new_shared_folder() {
    new_object(CLASS_SHARED_FOLDER);
}


void ObjectImpl::on_move() {
    AdInterface ad;
    if (ad_failed(ad, console)) {
        return;
    }

    const QList<QString> dn_list = get_selected_dn_list_object();

    auto dialog = new SelectContainerDialog(ad, console, dn_list);
    dialog->open();

    connect(
        dialog, &QDialog::accepted,
        this,
        [this, dialog, dn_list]() {
            AdInterface ad2;
            if (ad_failed(ad2, console)) {
                return;
            }

            show_busy_indicator();

            const QString new_parent_dn = dialog->get_selected();

            // First move in AD
            const QList<QString> moved_objects = [&]() {
                QList<QString> out;

                for (const QString &dn : dn_list) {
                    const bool success = ad2.object_move(dn, new_parent_dn);

                    if (success) {
                        out.append(dn);
                    }
                }

                return out;
            }();

            g_status->display_ad_messages(ad2, nullptr);

            // Then move in console
            move(ad2, moved_objects, new_parent_dn);

            hide_busy_indicator();
        });
}

void ObjectImpl::on_enable() {
    set_disabled(false);
}

void ObjectImpl::on_disable() {
    set_disabled(true);
}

void ObjectImpl::on_add_to_group() {
    auto dialog = new SelectObjectDialog({CLASS_GROUP}, SelectObjectDialogMultiSelection_Yes, console);
    dialog->setWindowTitle(tr("Add to Group"));
    dialog->open();

    connect(
        dialog, &SelectObjectDialog::accepted,
        this,
        [this, dialog]() {
            AdInterface ad;
            if (ad_failed(ad, console)) {
                return;
            }

            show_busy_indicator();

            const QList<QString> target_list = get_selected_dn_list_object();

            const QList<QString> groups = dialog->get_selected();

            for (const QString &target : target_list) {
                for (auto group : groups) {
                    ad.group_add_member(group, target);
                }
            }

            hide_busy_indicator();

            g_status->display_ad_messages(ad, console);
        });
}

void ObjectImpl::on_find() {
    const QList<QString> dn_list = get_selected_dn_list_object();

    const QString dn = dn_list[0];

    auto find_dialog = new FindObjectDialog(console, dn, console);
    find_dialog->open();
}

void ObjectImpl::on_reset_password() {
    AdInterface ad;
    if (ad_failed(ad, console)) {
        return;
    }

    const QString dn = get_selected_target_dn_object();

    auto dialog = new PasswordDialog(ad, dn, console);
    dialog->open();
}

void ObjectImpl::on_edit_upn_suffixes() {
    AdInterface ad;
    if (ad_failed(ad, console)) {
        return;
    }

    // Open attribute dialog for upn suffixes attribute of
    // partitions object
    const QString partitions_dn = g_adconfig->partitions_dn();
    const AdObject partitions_object = ad.search_object(partitions_dn);
    const QList<QByteArray> current_values = partitions_object.get_values(ATTRIBUTE_UPN_SUFFIXES);

    g_status->display_ad_messages(ad, console);

    const QString attribute = ATTRIBUTE_UPN_SUFFIXES;
    const bool read_only = false;
    auto dialog = new ListAttributeDialog(current_values, attribute, read_only, console);
    dialog->setWindowTitle(tr("Edit UPN Suffixes"));
    dialog->open();

    connect(
        dialog, &QDialog::accepted,
        this,
        [this, dialog, partitions_dn]() {
            AdInterface ad_inner;
            if (ad_failed(ad_inner, console)) {
                return;
            }

            const QList<QByteArray> new_values = dialog->get_value_list();

            ad_inner.attribute_replace_values(partitions_dn, ATTRIBUTE_UPN_SUFFIXES, new_values);
            g_status->display_ad_messages(ad_inner, console);
        });
}

void ObjectImpl::on_reset_account() {
    const bool confirmed = confirmation_dialog(tr("Are you sure you want to reset this account?"), console);
    if (!confirmed) {
        return;
    }

    AdInterface ad;
    if (ad_failed(ad, console)) {
        return;
    }

    show_busy_indicator();

    const QList<QString> target_list = get_selected_dn_list_object();

    for (const QString &target : target_list) {
        ad.computer_reset_account(target);
    }

    hide_busy_indicator();

    g_status->display_ad_messages(ad, console);
}

void ObjectImpl::new_object(const QString &object_class) {
    const QString parent_dn = get_selected_target_dn_object();

    ConsoleObjectTreeOperations::console_object_create({console}, object_class, parent_dn);
}



void ObjectImpl::set_disabled(const bool disabled) {
    AdInterface ad;
    if (ad_failed(ad, console)) {
        return;
    }

    show_busy_indicator();

    const QList<QString> changed_objects = [&]() {
        QList<QString> out;

        const QList<QString> dn_list = get_selected_dn_list_object();

        for (const QString &dn : dn_list) {
            const bool success = ad.user_set_account_option(dn, AccountOption_Disabled, disabled);

            if (success) {
                out.append(dn);
            }
        }

        return out;
    }();

    auto apply_changes = [&changed_objects, &disabled](ConsoleWidget *target_console) {
        auto apply_changes_to_branch = [&](const QModelIndex &root_index) {
            if (!root_index.isValid()) {
                return;
            }

            for (const QString &dn : changed_objects) {
                const QList<QModelIndex> index_list = target_console->search_items(root_index, ObjectRole_DN, dn, {ItemType_Object});

                for (const QModelIndex &index : index_list) {
                    QStandardItem *item = target_console->get_item(index);
                    item->setData(disabled, ObjectRole_AccountDisabled);
                    const QString category= dn_get_name(item->data(ObjectRole_ObjectCategory).toString());
                    QIcon icon;
                    if (category == OBJECT_CATEGORY_PERSON) {
                        icon = disabled ? g_icon_manager->item_icon(ItemIcon_Person_Blocked) :
                                            g_icon_manager->item_icon(ItemIcon_Person);
                    }
                    else if (category == OBJECT_CATEGORY_COMPUTER) {
                        icon = disabled ? g_icon_manager->item_icon(ItemIcon_Computer_Blocked) :
                                            g_icon_manager->item_icon(ItemIcon_Computer);
                    }
                    item->setIcon(icon);
                }
            }
        };

        const QModelIndex object_root = ConsoleObjectTreeOperations::get_domain_object_tree_root(target_console);
        const QModelIndex find_object_root = get_find_object_root(target_console);
        const QModelIndex query_root = get_query_tree_root(target_console);

        apply_changes_to_branch(object_root);
        apply_changes_to_branch(find_object_root);
        apply_changes_to_branch(query_root);
    };

    for (ConsoleWidget *target_console : console_list) {
        apply_changes(target_console);
    }

    hide_busy_indicator();

    g_status->display_ad_messages(ad, console);
}

// NOTE: this is a helper f-n for move_and_rename() that
// generates the new_dn_list for you, assuming that you just
// want to move objects to new parent without renaming
void ObjectImpl::move(AdInterface &ad, const QList<QString> &old_dn_list, const QString &new_parent_dn) {
    const QHash<QString, QString> old_to_new_dn_map = [&]() {
        QHash<QString, QString> out;

        for (const QString &old_dn : old_dn_list) {
            const QString new_dn = dn_move(old_dn, new_parent_dn);
            out[old_dn] = new_dn;
        }

        return out;
    }();

    ConsoleObjectTreeOperations::console_object_move_and_rename(console_list, ad, old_to_new_dn_map, new_parent_dn);
}

void ObjectImpl::update_toolbar_actions() {
    const QHash<QString, QAction *> toolbar_action_map = {
        {CLASS_USER, toolbar_create_user},
        {CLASS_GROUP, toolbar_create_group},
        {CLASS_OU, toolbar_create_ou},
    };

    // Disable all actions by default
    for (const QString &action_object_class : toolbar_action_map.keys()) {
        QAction *action = toolbar_action_map[action_object_class];

        if (action == nullptr) {
            continue;
        }

        action->setEnabled(false);
    }

    // Then enable them depending on current selection
    const QList<QModelIndex> target_list = console->get_selected_items(ItemType_Object);

    const bool single_selection = (target_list.size() == 1);
    if (!single_selection) {
        return;
    }

    const QModelIndex target = target_list[0];
    const QVariant target_data = target.data(ObjectRole_ObjectClasses);
    if (!target_data.canConvert<QStringList>() || target_data.toStringList().isEmpty()) {
        return;
    }
    const QString object_class = target_data.toStringList().last();

    for (const QString &action_object_class : toolbar_action_map.keys()) {
        QAction *action = toolbar_action_map[action_object_class];

        if (action == nullptr) {
            continue;
        }

        const bool is_enabled = can_create_class_at_parent(action_object_class, object_class);
        action->setEnabled(is_enabled);
    }
}

QList<QString> ObjectImpl::get_selected_dn_list_object() {
    return get_selected_dn_list(console, ItemType_Object, ObjectRole_DN);
}

QString ObjectImpl::get_selected_target_dn_object() {
    return get_selected_target_dn(console, ItemType_Object, ObjectRole_DN);
}

bool ObjectImpl::can_create_class_at_parent(const QString &create_class, const QString &parent_class) const {
    // NOTE: to get full list of possible
    // superiors, need to use the all of the parent
    // classes too, not just the leaf class
    const QList<QString> action_object_class_list = g_adconfig->get_inherit_chain(create_class);
    const QList<QString> possible_superiors = g_adconfig->get_possible_superiors(QList<QString>(action_object_class_list));
    const bool out = possible_superiors.contains(parent_class);

    return out;
}

void ObjectImpl::setup_widgets() {
    stacked_widget = new QStackedWidget(console);
    set_results_view(new ResultsView(console));
    group_results_widget = new GeneralGroupTab();
    user_results_widget = new GeneralUserTab();
    pso_results_widget = new PSOResultsWidget();
    subnet_results_widget = new SubnetResultsWidget();
    stacked_widget->addWidget(group_results_widget);
    stacked_widget->addWidget(user_results_widget);
    stacked_widget->addWidget(pso_results_widget);
    stacked_widget->addWidget(subnet_results_widget);
    stacked_widget->addWidget(view());
    set_results_widget(stacked_widget);
}

void ObjectImpl::setup_filters() {
    object_filter = settings_get_variant(SETTING_object_filter).toString();
    object_filter_enabled = settings_get_variant(SETTING_object_filter_enabled).toBool();
}

void ObjectImpl::setup_actions() {
    find_action_enabled = true;
    refresh_action_enabled = true;

    toolbar_create_user = nullptr;
    toolbar_create_group = nullptr;
    toolbar_create_ou = nullptr;

    standard_create_action_map[CLASS_USER] = new QAction(tr("User"), this);
    standard_create_action_map[CLASS_COMPUTER] = new QAction(tr("Computer"), this);
    standard_create_action_map[CLASS_OU] = new QAction(tr("OU"), this);
    standard_create_action_map[CLASS_GROUP] = new QAction(tr("Group"), this);
    standard_create_action_map[CLASS_SHARED_FOLDER] = new QAction(tr("Shared Folder"), this);
    standard_create_action_map[CLASS_INET_ORG_PERSON] = new QAction(tr("inetOrgPerson"), this);
    standard_create_action_map[CLASS_CONTACT] = new QAction(tr("Contact"), this);
    find_action = new QAction(tr("Find..."), this);
    move_action = new QAction(tr("Move..."), this);
    add_to_group_action = new QAction(tr("Add to group..."), this);
    enable_action = new QAction(tr("Enable"), this);
    disable_action = new QAction(tr("Disable"), this);
    reset_password_action = new QAction(tr("Reset password"), this);
    reset_account_action = new QAction(tr("Reset account"), this);
    edit_upn_suffixes_action = new QAction(tr("Edit UPN suffixes"), this);

    auto new_menu = new QMenu(tr("New"), console);
    new_action = new_menu->menuAction();

    const QList<QString> new_action_keys_sorted = [&]() {
        QList<QString> out = standard_create_action_map.keys();
        std::sort(out.begin(), out.end());

        return out;
    }();
    for (const QString &key : new_action_keys_sorted) {
        QAction *action = standard_create_action_map[key];
        new_menu->addAction(action);
    }

    create_pso_action = new QAction(tr("Create password setting object"), this);
    create_subnet_action = new QAction(tr("Create subnet"), this);
    create_site_action = new QAction(tr("Create site"), this);
    create_site_link_action = new QAction(tr("Create site link"), this);
    create_site_link_bridge_action = new QAction(tr("Create site link bridge"), this);

    QHash<QString, QAction *> all_create_action_map {standard_create_action_map};
    all_create_action_map[CLASS_PSO] = create_pso_action;
    all_create_action_map[CLASS_SUBNET] = create_subnet_action;
    all_create_action_map[CLASS_SITE] = create_site_action;
    all_create_action_map[CLASS_SITE_LINK] = create_site_link_action;
    all_create_action_map[CLASS_SITE_LINK_BRIDGE] = create_site_link_bridge_action;

    for (const QString &obj_class : all_create_action_map.keys()) {
        connect(
            all_create_action_map[obj_class], &QAction::triggered,
                    this, [this, obj_class]() {
                                new_object(obj_class); });
    }

    connect(
        move_action, &QAction::triggered,
        this, &ObjectImpl::on_move);
    connect(
        add_to_group_action, &QAction::triggered,
        this, &ObjectImpl::on_add_to_group);
    connect(
        enable_action, &QAction::triggered,
        this, &ObjectImpl::on_enable);
    connect(
        disable_action, &QAction::triggered,
        this, &ObjectImpl::on_disable);
    connect(
        reset_password_action, &QAction::triggered,
        this, &ObjectImpl::on_reset_password);
    connect(
        reset_account_action, &QAction::triggered,
        this, &ObjectImpl::on_reset_account);
    connect(
        find_action, &QAction::triggered,
        this, &ObjectImpl::on_find);
    connect(
        edit_upn_suffixes_action, &QAction::triggered,
        this, &ObjectImpl::on_edit_upn_suffixes);
    connect(
        console, &ConsoleWidget::selection_changed,
        this, &ObjectImpl::update_toolbar_actions);
}
