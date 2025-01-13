/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2022 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
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

#include "console_impls/object_impl.h"

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
#include "create_dialogs/create_computer_dialog.h"
#include "create_dialogs/create_contact_dialog.h"
#include "create_dialogs/create_group_dialog.h"
#include "create_dialogs/create_ou_dialog.h"
#include "create_dialogs/create_shared_folder_dialog.h"
#include "create_dialogs/create_user_dialog.h"
#include "find_widgets/find_object_dialog.h"
#include "globals.h"
#include "password_dialog.h"
#include "properties_widgets/properties_dialog.h"
#include "properties_widgets/properties_multi_dialog.h"
#include "rename_dialogs/rename_group_dialog.h"
#include "rename_dialogs/rename_object_dialog.h"
#include "rename_dialogs/rename_other_dialog.h"
#include "rename_dialogs/rename_user_dialog.h"
#include "search_thread.h"
#include "select_dialogs/select_container_dialog.h"
#include "select_dialogs/select_object_dialog.h"
#include "settings.h"
#include "status.h"
#include "utils.h"
#include "tabs/general_user_tab.h"
#include "tabs/general_group_tab.h"
#include "managers/icon_manager.h"
#include "create_dialogs/create_pso_dialog.h"
#include "results_widgets/pso_results_widget/pso_results_widget.h"

#include <QDebug>
#include <QMenu>
#include <QSet>
#include <QStandardItemModel>
#include <QStackedWidget>
#include <QMessageBox>

#include <algorithm>

enum DropType {
    DropType_Move,
    DropType_AddToGroup,
    DropType_None
};

DropType console_object_get_drop_type(const QModelIndex &dropped, const QModelIndex &target);
QList<QString> get_selected_dn_list_object(ConsoleWidget *console);
QString get_selected_target_dn_object(ConsoleWidget *console);
void console_object_delete_dn_list(ConsoleWidget *console, const QList<QString> &dn_list, const QModelIndex &tree_root, const int type, const int dn_role);
bool can_create_class_at_parent(const QString &create_class, const QString &parent_class);
void console_object_move_and_rename(const QList<ConsoleWidget *> &console_list, AdInterface &ad, const QHash<QString, QString> &old_to_new_dn_map_arg, const QString &new_parent_dn);

ObjectImpl::ObjectImpl(ConsoleWidget *console_arg)
: ConsoleImpl(console_arg) {
    console_list = {
        console,
    };

    stacked_widget = new QStackedWidget(console_arg);
    set_results_view(new ResultsView(console_arg));
    group_results_widget = new GeneralGroupTab();
    user_results_widget = new GeneralUserTab();
    pso_results_widget = new PSOResultsWidget();
    stacked_widget->addWidget(group_results_widget);
    stacked_widget->addWidget(user_results_widget);
    stacked_widget->addWidget(pso_results_widget);
    stacked_widget->addWidget(view());
    set_results_widget(stacked_widget);

    find_action_enabled = true;
    refresh_action_enabled = true;

    toolbar_create_user = nullptr;
    toolbar_create_group = nullptr;
    toolbar_create_ou = nullptr;

    object_filter = settings_get_variant(SETTING_object_filter).toString();
    object_filter_enabled = settings_get_variant(SETTING_object_filter_enabled).toBool();

    new_action_map[CLASS_USER] = new QAction(tr("User"), this);
    new_action_map[CLASS_COMPUTER] = new QAction(tr("Computer"), this);
    new_action_map[CLASS_OU] = new QAction(tr("OU"), this);
    new_action_map[CLASS_GROUP] = new QAction(tr("Group"), this);
    new_action_map[CLASS_SHARED_FOLDER] = new QAction(tr("Shared Folder"), this);
    new_action_map[CLASS_INET_ORG_PERSON] = new QAction(tr("inetOrgPerson"), this);
    new_action_map[CLASS_CONTACT] = new QAction(tr("Contact"), this);
    find_action = new QAction(tr("Find..."), this);
    move_action = new QAction(tr("Move..."), this);
    add_to_group_action = new QAction(tr("Add to group..."), this);
    enable_action = new QAction(tr("Enable"), this);
    disable_action = new QAction(tr("Disable"), this);
    reset_password_action = new QAction(tr("Reset password"), this);
    reset_account_action = new QAction(tr("Reset account"), this);
    edit_upn_suffixes_action = new QAction(tr("Edit UPN suffixes"), this);
    create_pso_action = new QAction(tr("Create password setting object"), this);

    auto new_menu = new QMenu(tr("New"), console_arg);
    new_action = new_menu->menuAction();

    const QList<QString> new_action_keys_sorted = [&]() {
        QList<QString> out = new_action_map.keys();
        std::sort(out.begin(), out.end());

        return out;
    }();
    for (const QString &key : new_action_keys_sorted) {
        QAction *action = new_action_map[key];
        new_menu->addAction(action);
    }

    connect(
        new_action_map[CLASS_USER], &QAction::triggered,
        this, &ObjectImpl::on_new_user);
    connect(
        new_action_map[CLASS_COMPUTER], &QAction::triggered,
        this, &ObjectImpl::on_new_computer);
    connect(
        new_action_map[CLASS_OU], &QAction::triggered,
        this, &ObjectImpl::on_new_ou);
    connect(
        new_action_map[CLASS_GROUP], &QAction::triggered,
        this, &ObjectImpl::on_new_group);
    connect(
        new_action_map[CLASS_SHARED_FOLDER], &QAction::triggered,
        this, &ObjectImpl::on_new_shared_folder);
    connect(
        new_action_map[CLASS_INET_ORG_PERSON], &QAction::triggered,
        this, &ObjectImpl::on_new_inet_org_person);
    connect(
        new_action_map[CLASS_CONTACT], &QAction::triggered,
        this, &ObjectImpl::on_new_contact);
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
    connect(
        create_pso_action, &QAction::triggered,
        this, &ObjectImpl::on_create_pso);
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

        out = advanced_features_filter(out);

        return out;
    }();

    const QList<QString> attributes = console_object_search_attributes();

    // NOTE: do an extra search before real search for
    // objects that should be visible in dev mode
    const bool dev_mode = settings_get_variant(SETTING_feature_dev_mode).toBool();
    if (dev_mode) {
        AdInterface ad;
        if (ad_connected(ad, console)) {
            QHash<QString, AdObject> results;
            dev_mode_search_results(results, ad, base);

            object_impl_add_objects_to_console(console, results.values(), index);
        }
    }

    console_object_search(console, index, base, scope, filter, attributes);
}

bool ObjectImpl::can_drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type) {
    UNUSED_ARG(target_type);

    const bool dropped_are_all_objects = (dropped_type_list == QSet<int>({ItemType_Object}));

    if (dropped_are_all_objects) {
        if (dropped_list.size() == 1) {
            const QPersistentModelIndex dropped = dropped_list[0];

            const DropType drop_type = console_object_get_drop_type(dropped, target);
            const bool can_drop = (drop_type != DropType_None);

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
        const DropType drop_type = console_object_get_drop_type(dropped, target);

        switch (drop_type) {
            case DropType_Move: {
                const bool move_success = ad.object_move(dropped_dn,
                    target_dn);

                if (move_success) {
                    move(ad, QList<QString>({dropped_dn}), target_dn);
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

    hide_busy_indicator();

    g_status->display_ad_messages(ad, console);
}

QString ObjectImpl::get_description(const QModelIndex &index) const {
    QString out;

    const QString object_count_text = console_object_count_string(console, index);

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
    for (const QString &action_object_class : new_action_map.keys()) {
        QAction *action = new_action_map[action_object_class];

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

    if (cannot_move || object_class == CLASS_PSO) {
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

    console_object_rename(console_list, index_list, ObjectRole_DN, object_class);
}

void console_object_rename(const QList<ConsoleWidget *> &console_list, const QList<QModelIndex> &index_list, const int dn_role, const QString &object_class) {
    AdInterface ad;
    if (ad_failed(ad, console_list[0])) {
        return;
    }

    const QString old_dn = [&]() {
        if (!index_list.isEmpty()) {
            const QModelIndex index = index_list[0];
            const QString out = index.data(dn_role).toString();

            return out;
        } else {
            return QString();
        }
    }();

    RenameObjectDialog *dialog = [&]() -> RenameObjectDialog * {
        const bool is_user = (object_class == CLASS_USER);
        const bool is_group = (object_class == CLASS_GROUP);

        if (is_user) {
            return new RenameUserDialog(ad, old_dn, console_list[0]);
        } else if (is_group) {
            return new RenameGroupDialog(ad, old_dn, console_list[0]);
        } else {
            return new RenameOtherDialog(ad, old_dn, console_list[0]);
        }
    }();

    dialog->open();

    QObject::connect(
        dialog, &QDialog::accepted,
        console_list[0],
        [console_list, dialog, old_dn]() {
            AdInterface ad_inner;
            if (ad_failed(ad_inner, console_list[0])) {
                return;
            }

            const QString new_dn = dialog->get_new_dn();
            const QString parent_dn = dn_get_parent(old_dn);

            console_object_move_and_rename(console_list, ad_inner, {{old_dn, new_dn}}, parent_dn);
        });
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

    console_object_properties(console_list, index_list, ObjectRole_DN, class_list);
}

void console_object_properties(const QList<ConsoleWidget *> &console_list, const QList<QModelIndex> &index_list, const int dn_role, const QList<QString> &class_list) {
    AdInterface ad;
    if (ad_failed(ad, console_list[0])) {
        return;
    }

    const QList<QString> dn_list = index_list_to_dn_list(index_list, dn_role);

    auto on_object_properties_applied = [console_list, dn_list]() {
        AdInterface ad2;
        if (ad_failed(ad2, console_list[0])) {
            return;
        }

        const QList<AdObject> object_list = [&]() {
            QList<AdObject> out;

            for (const QString &dn : dn_list) {
                const AdObject object = ad2.search_object(dn);

                // TODO: band-aid for the situations
                // where properties dialog interacts
                // with deleted objects. Bad stuff can
                // still happen if properties is opened
                // while object exists, then object is
                // deleted and properties is applied.
                // Remove this or improve it when you
                // tackle this issue head-on.
                if (object.is_empty()) {
                    continue;
                }

                out.append(object);
            }

            return out;
        }();

        auto apply_changes = [&object_list](ConsoleWidget *target_console) {
            auto apply_changes_to_branch = [&](const QModelIndex &root_index, const int item_type, const int update_dn_role) {
                if (!root_index.isValid()) {
                    return;
                }

                for (const AdObject &object : object_list) {
                    const QString dn = object.get_dn();
                    const QModelIndex object_index = target_console->search_item(root_index, update_dn_role, dn, {item_type});

                    if (object_index.isValid()) {
                        const QList<QStandardItem *> object_row = target_console->get_row(object_index);
                        console_object_load(object_row, object);
                    }
                }
            };

            const QModelIndex object_root = get_object_tree_root(target_console);
            const QModelIndex query_root = get_query_tree_root(target_console);
            const QModelIndex policy_root = get_policy_tree_root(target_console);
            const QModelIndex find_object_root = get_find_object_root(target_console);

            apply_changes_to_branch(object_root, ItemType_Object, ObjectRole_DN);
            apply_changes_to_branch(query_root, ItemType_Object, ObjectRole_DN);
            apply_changes_to_branch(find_object_root, ItemType_Object, ObjectRole_DN);

            // Apply to policy branch
            if (policy_root.isValid()) {
                for (const AdObject &object : object_list) {
                    const QString dn = object.get_dn();
                    const QModelIndex object_index = target_console->search_item(policy_root, PolicyOURole_DN, dn, {ItemType_PolicyOU});

                    if (object_index.isValid()) {
                        const QList<QStandardItem *> object_row = target_console->get_row(object_index);

                        policy_ou_impl_load_row(object_row, object);
                    }
                }
            }
        };

        for (ConsoleWidget *console : console_list) {
            apply_changes(console);
        }

        g_status->display_ad_messages(ad2, console_list[0]);
    };

    if (dn_list.size() == 1) {
        const QString dn = dn_list[0];

        bool dialog_is_new;
        PropertiesDialog *dialog = PropertiesDialog::open_for_target(ad, dn, &dialog_is_new, console_list[0]);

        if (dialog_is_new) {
            QObject::connect(
                dialog, &PropertiesDialog::applied,
                console_list[0], on_object_properties_applied);

            QObject::connect(
                dialog, &PropertiesDialog::applied,
                [console_list]()
                    {console_list[0]->update_current_item_results_widget();});
        }
    } else if (dn_list.size() > 1) {
        auto dialog = new PropertiesMultiDialog(ad, dn_list, class_list);
        dialog->open();

        QObject::connect(
            dialog, &PropertiesMultiDialog::applied,
            console_list[0], on_object_properties_applied);
    }
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
    console_object_delete(console_list, index_list, ObjectRole_DN);
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
    else {
        stacked_widget->setCurrentWidget(view());
    }
}

void ObjectImpl::update_results_widget(const QModelIndex &index) const
{
    const QStringList index_data_classes = index.data(ObjectRole_ObjectClasses).toStringList();
    if (!(index_data_classes.contains(CLASS_GROUP) || index_data_classes.contains(CLASS_CONTACT) ||
          index_data_classes.contains(CLASS_USER) || index_data_classes.contains(CLASS_INET_ORG_PERSON) ||
          index_data_classes.contains(CLASS_PSO))) {
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
}

void console_object_delete(const QList<ConsoleWidget *> &console_list, const QList<QModelIndex> &index_list, const int dn_role) {
    const bool confirmed = console_object_deletion_dialog(console_list[0], index_list);
    if (!confirmed) {
        return;
    }

    AdInterface ad;
    if (ad_failed(ad, console_list[0])) {
        return;
    }

    show_busy_indicator();

    const QList<QString> deleted_list = [&]() {
        QList<QString> out;

        const QList<QString> target_list = index_list_to_dn_list(index_list, dn_role);

        for (const QString &target : target_list) {
            const bool success = ad.object_delete(target);

            if (success) {
                out.append(target);
            }
        }

        return out;
    }();

    auto apply_changes = [&deleted_list](ConsoleWidget *target_console) {
        const QList<QModelIndex> root_list = {
            get_object_tree_root(target_console),
            get_query_tree_root(target_console),
            get_find_object_root(target_console),
        };

        for (const QModelIndex &root : root_list) {
            if (root.isValid()) {
                console_object_delete_dn_list(target_console, deleted_list, root, ItemType_Object, ObjectRole_DN);
            }
        }

        const QModelIndex policy_root = get_policy_tree_root(target_console);
        if (policy_root.isValid()) {
            console_object_delete_dn_list(target_console, deleted_list, policy_root, ItemType_PolicyOU, PolicyOURole_DN);
        }
    };

    for (ConsoleWidget *console : console_list) {
        apply_changes(console);
    }

    hide_busy_indicator();

    g_status->display_ad_messages(ad, console_list[0]);
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
    return object_impl_column_labels();
}

QList<int> ObjectImpl::default_columns() const {
    return object_impl_default_columns();
}

void ObjectImpl::refresh_tree() {
    const QModelIndex object_tree_root = get_object_tree_root(console);
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

void ObjectImpl::on_new_inet_org_person() {
    new_object(CLASS_INET_ORG_PERSON);
}

void ObjectImpl::on_new_contact() {
    new_object(CLASS_CONTACT);
}

void ObjectImpl::on_create_pso() {
    new_object(CLASS_PSO);
}

void ObjectImpl::on_move() {
    AdInterface ad;
    if (ad_failed(ad, console)) {
        return;
    }

    auto dialog = new SelectContainerDialog(ad, console);
    dialog->open();

    connect(
        dialog, &QDialog::accepted,
        this,
        [this, dialog]() {
            AdInterface ad2;
            if (ad_failed(ad2, console)) {
                return;
            }

            const QList<QString> dn_list = get_selected_dn_list_object(console);

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

            const QList<QString> target_list = get_selected_dn_list_object(console);

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
    const QList<QString> dn_list = get_selected_dn_list_object(console);

    const QString dn = dn_list[0];

    auto find_dialog = new FindObjectDialog(console, dn, console);
    find_dialog->open();
}

void ObjectImpl::on_reset_password() {
    AdInterface ad;
    if (ad_failed(ad, console)) {
        return;
    }

    const QString dn = get_selected_target_dn_object(console);

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

    const QList<QString> target_list = get_selected_dn_list_object(console);

    for (const QString &target : target_list) {
        ad.computer_reset_account(target);
    }

    hide_busy_indicator();

    g_status->display_ad_messages(ad, console);
}

void ObjectImpl::new_object(const QString &object_class) {
    const QString parent_dn = get_selected_target_dn_object(console);

    console_object_create({console}, object_class, parent_dn);
}

void console_object_create(const QList<ConsoleWidget *> &console_list, const QString &object_class, const QString &parent_dn) {
    AdInterface ad;
    if (ad_failed(ad, console_list[0])) {
        return;
    }

    // NOTE: creating dialogs here instead of directly
    // in "on_new_x" slots looks backwards but it's
    // necessary to avoid even more code duplication
    // due to having to pass "ad" and "parent_dn" args
    // to dialog ctors
    CreateObjectDialog *dialog = [&]() -> CreateObjectDialog * {
        const bool is_user = (object_class == CLASS_USER);
        const bool is_group = (object_class == CLASS_GROUP);
        const bool is_computer = (object_class == CLASS_COMPUTER);
        const bool is_ou = (object_class == CLASS_OU);
        const bool is_shared_folder = (object_class == CLASS_SHARED_FOLDER);
        const bool is_inet_org_person = (object_class == CLASS_INET_ORG_PERSON);
        const bool is_contact = (object_class == CLASS_CONTACT);
        const bool is_pso = (object_class == CLASS_PSO);

        if (is_user) {
            return new CreateUserDialog(ad, parent_dn, CLASS_USER, console_list[0]);
        } else if (is_group) {
            return new CreateGroupDialog(parent_dn, console_list[0]);
        } else if (is_computer) {
            return new CreateComputerDialog(parent_dn, console_list[0]);
        } else if (is_ou) {
            return new CreateOUDialog(parent_dn, console_list[0]);
        } else if (is_shared_folder) {
            return new CreateSharedFolderDialog(parent_dn, console_list[0]);
        } else if (is_inet_org_person) {
            return new CreateUserDialog(ad, parent_dn, CLASS_INET_ORG_PERSON, console_list[0]);
        } else if (is_contact) {
            return new CreateContactDialog(parent_dn, console_list[0]);
        } else if (is_pso) {
            return new CreatePSODialog(parent_dn, console_list[0]);
        } else {
            return nullptr;
        }
    }();

    if (dialog == nullptr) {
        return;
    }

    dialog->open();

    QObject::connect(
        dialog, &QDialog::accepted,
        console_list[0],
        [console_list, dialog, parent_dn, object_class]() {
            AdInterface ad_inner;
            if (ad_failed(ad_inner, console_list[0])) {
                return;
            }

            show_busy_indicator();

            const QString created_dn = dialog->get_created_dn();

            // NOTE: we don't just use currently selected index as
            // parent to add new object onto, because you can create
            // an object in a query tree or find dialog. Therefore
            // need to search for index of parent object in domain
            // tree.
            auto apply_changes = [&](ConsoleWidget *target_console) {
                const QModelIndex object_root = get_object_tree_root(target_console);
                if (object_root.isValid()) {
                    const QModelIndex parent_object = target_console->search_item(object_root, ObjectRole_DN, parent_dn, {ItemType_Object});

                    if (parent_object.isValid()) {
                        object_impl_add_objects_to_console_from_dns(target_console, ad_inner, {created_dn}, parent_object);
                    }
                }

                // NOTE: changes are not applied to
                // find tree because creation of
                // objects shouldn't affect find
                // results. If it does affect them by
                // creating a new object that fits
                // search criteria, then find dialog
                // can just be considered out of date
                // and should be updated

                // Apply changes to policy tree
                const QModelIndex policy_root = get_policy_tree_root(target_console);
                if (policy_root.isValid() && object_class == CLASS_OU) {
                    const QModelIndex parent_policy = target_console->search_item(policy_root, PolicyOURole_DN, parent_dn, {ItemType_PolicyOU});

                    if (parent_policy.isValid()) {
                        policy_ou_impl_add_objects_from_dns(target_console, ad_inner, {created_dn}, parent_policy);
                    }
                }
            };

            for (ConsoleWidget *console : console_list) {
                apply_changes(console);
            }

            hide_busy_indicator();
        });
}

void ObjectImpl::set_disabled(const bool disabled) {
    AdInterface ad;
    if (ad_failed(ad, console)) {
        return;
    }

    show_busy_indicator();

    const QList<QString> changed_objects = [&]() {
        QList<QString> out;

        const QList<QString> dn_list = get_selected_dn_list_object(console);

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
                        icon = disabled ? g_icon_manager->get_icon_for_type(ItemIconType_Person_Blocked) :
                                            g_icon_manager->get_icon_for_type(ItemIconType_Person_Clean);
                    }
                    else if (category == OBJECT_CATEGORY_COMPUTER) {
                        icon = disabled ? g_icon_manager->get_icon_for_type(ItemIconType_Computer_Blocked) :
                                            g_icon_manager->get_icon_for_type(ItemIconType_Computer_Clean);
                    }
                    item->setIcon(icon);
                }
            }
        };

        const QModelIndex object_root = get_object_tree_root(target_console);
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

void console_object_move_and_rename(const QList<ConsoleWidget *> &console_list, AdInterface &ad, const QHash<QString, QString> &old_to_new_dn_map_arg, const QString &new_parent_dn) {
    // NOTE: sometimes, some objects that are supposed
    // to be moved don't actually need to be. For
    // example, if an object were to be moved to an
    // object that is already it's current parent. If
    // we were to move them that would cause gui
    // glitches because that kind of move is not
    // considered in the update logic. Instead, we skip
    // these kinds of objects.
    const QHash<QString, QString> &old_to_new_dn_map = [&]() {
        QHash<QString, QString> out;

        for (const QString &old_dn : old_to_new_dn_map_arg.keys()) {
            const QString new_dn = old_to_new_dn_map_arg[old_dn];
            const bool dn_changed = (new_dn != old_dn);

            if (dn_changed) {
                out[old_dn] = new_dn;
            }
        }

        return out;
    }();

    const QList<QString> old_dn_list = old_to_new_dn_map.keys();
    const QList<QString> new_dn_list = old_to_new_dn_map.values();

    // NOTE: search for objects once here to reuse them
    // multiple times later
    const QHash<QString, AdObject> object_map = [&]() {
        QHash<QString, AdObject> out;

        for (const QString &dn : new_dn_list) {
            const AdObject object = ad.search_object(dn);
            out[dn] = object;
        }

        return out;
    }();

    auto apply_changes = [&old_to_new_dn_map, &old_dn_list, &new_parent_dn, &object_map](ConsoleWidget *target_console) {
        // For object tree, we add items representing
        // updated objects and delete old items. In the case
        // of move, this moves the items to their new
        // parent. In the case of rename, this updates
        // item's information about the object by recreating
        // it.

        // NOTE: delete old item AFTER adding new item
        // because: If old item is deleted first, then it's
        // possible for new parent to get selected (if they
        // are next to each other in scope tree). Then what
        // happens is that due to new parent being selected,
        // it gets fetched and loads new object. End result
        // is that new object is duplicated.
        const QModelIndex object_root = get_object_tree_root(target_console);
        if (object_root.isValid()) {
            const QModelIndex parent_object = target_console->search_item(object_root, ObjectRole_DN, new_parent_dn, {ItemType_Object});

            if (parent_object.isValid()) {
                object_impl_add_objects_to_console(target_console, object_map.values(), parent_object);

                console_object_delete_dn_list(target_console, old_dn_list, object_root, ItemType_Object, ObjectRole_DN);
            }
        }

        // For query tree, we don't move items or recreate
        // items but instead update items. Move doesn't make
        // sense because query tree doesn't represent the
        // object tree. Note that changing name or DN of
        // objects may mean that the parent query may become
        // invalid. For example, if query searches for all
        // objects with names starting with "foo" and we
        // rename an object in that query so that it doesn't
        // start with "foo" anymore. Since it is too
        // complicated to determine match criteria on the
        // client, and refreshing the query is too wasteful,
        // we instead mark queries with modified objects as
        // "out of date". It is then up to the user to
        // refresh the query if needed.
        const QModelIndex query_root = get_query_tree_root(target_console);
        if (query_root.isValid()) {
            // Find indexes of modified objects in query
            // tree
            const QHash<QString, QModelIndex> index_map = [&]() {
                QHash<QString, QModelIndex> out;

                for (const QString &old_dn : old_dn_list) {
                    const QList<QModelIndex> results = target_console->search_items(query_root, ObjectRole_DN, old_dn, {ItemType_Object});

                    for (const QModelIndex &index : results) {
                        out[old_dn] = index;
                    }
                }

                return out;
            }();

            for (const QString &old_dn : index_map.keys()) {
                const QModelIndex index = index_map[old_dn];

                // Mark parent query as "out of date". Icon
                // and tooltip will be restored after
                // refresh
                const QModelIndex query_index = index.parent();
                QStandardItem *item = target_console->get_item(query_index);
                item->setIcon(g_icon_manager->get_indicator_icon(g_icon_manager->warning_indicator));
                item->setToolTip(QCoreApplication::translate("ObjectImpl", "Query may be out of date"));

                // Update item row
                const QList<QStandardItem *> row = target_console->get_row(index);
                const QString new_dn = old_to_new_dn_map[old_dn];
                const AdObject object = object_map[new_dn];
                console_object_load(row, object);
            }
        }

        // TODO: decrease code duplication
        // For find tree, we only reload the rows to
        // update name, and attributes (DN for example)
        const QModelIndex find_object_root = get_query_tree_root(target_console);
        if (find_object_root.isValid()) {
            // Find indexes of modified objects in find
            // tree
            const QHash<QString, QModelIndex> index_map = [&]() {
                QHash<QString, QModelIndex> out;

                for (const QString &old_dn : old_dn_list) {
                    const QList<QModelIndex> results = target_console->search_items(find_object_root, ObjectRole_DN, old_dn, {ItemType_Object});

                    for (const QModelIndex &index : results) {
                        out[old_dn] = index;
                    }
                }

                return out;
            }();

            for (const QString &old_dn : index_map.keys()) {
                const QModelIndex index = index_map[old_dn];

                // Update item row
                const QList<QStandardItem *> row = target_console->get_row(index);
                const QString new_dn = old_to_new_dn_map[old_dn];
                const AdObject object = object_map[new_dn];
                console_object_load(row, object);
            }
        }

        // Apply changes to policy tree
        const QModelIndex policy_root = get_policy_tree_root(target_console);
        if (policy_root.isValid()) {
            const QModelIndex new_parent_index = target_console->search_item(policy_root, PolicyOURole_DN, new_parent_dn, {ItemType_PolicyOU});

            if (new_parent_index.isValid()) {
                policy_ou_impl_add_objects_to_console(target_console, object_map.values(), new_parent_index);

                console_object_delete_dn_list(target_console, old_dn_list, policy_root, ItemType_PolicyOU, PolicyOURole_DN);
            }
        }
    };

    for (ConsoleWidget *console : console_list) {
        apply_changes(console);
    }
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

    console_object_move_and_rename(console_list, ad, old_to_new_dn_map, new_parent_dn);
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

void object_impl_add_objects_to_console(ConsoleWidget *console, const QList<AdObject> &object_list, const QModelIndex &parent) {
    if (!parent.isValid()) {
        return;
    }

    // NOTE: don't add if parent wasn't fetched yet. If that
    // is the case then the object will be added naturally
    // when parent is fetched.
    const bool parent_was_fetched = console_item_get_was_fetched(parent);
    if (!parent_was_fetched) {
        return;
    }

    for (const AdObject &object : object_list) {
        if (object.is_empty())
            continue;
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

            const bool show_non_containers_ON = settings_get_variant(SETTING_show_non_containers_in_console_tree).toBool();

            return (is_container || show_non_containers_ON);
        }();

        const QList<QStandardItem *> row = [&]() {
            if (should_be_in_scope) {
                return console->add_scope_item(ItemType_Object, parent);
            } else {
                return console->add_results_item(ItemType_Object, parent);
            }
        }();

        console_object_load(row, object);
    }
}

// Helper f-n that searches for objects and then adds them
void object_impl_add_objects_to_console_from_dns(ConsoleWidget *console, AdInterface &ad, const QList<QString> &dn_list, const QModelIndex &parent) {
    const QList<AdObject> object_list = [&]() {
        QList<AdObject> out;

        for (const QString &dn : dn_list) {
            const AdObject object = ad.search_object(dn);
            out.append(object);
        }

        return out;
    }();

    object_impl_add_objects_to_console(console, object_list, parent);
}

void console_object_load(const QList<QStandardItem *> row, const AdObject &object) {
    // Load attribute columns
    for (int i = 0; i < g_adconfig->get_columns().count(); i++) {
        if (g_adconfig->get_columns().count() > row.size()) {
            break;
        }

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

    const bool cannot_move = object.get_system_flag(SystemFlagsBit_DomainCannotMove);

    for (auto item : row) {
        item->setDragEnabled(!cannot_move);
    }
}

void console_object_item_data_load(QStandardItem *item, const AdObject &object) {
    item->setData(object.get_dn(), ObjectRole_DN);

    const QList<QString> object_classes = object.get_strings(ATTRIBUTE_OBJECT_CLASS);
    item->setData(QVariant(object_classes), ObjectRole_ObjectClasses);

    const QString object_category = object.get_string(ATTRIBUTE_OBJECT_CATEGORY);
    item->setData(object_category, ObjectRole_ObjectCategory);

    const bool cannot_move = object.get_system_flag(SystemFlagsBit_DomainCannotMove);
    item->setData(cannot_move, ObjectRole_CannotMove);

    const bool cannot_rename = object.get_system_flag(SystemFlagsBit_DomainCannotRename);
    item->setData(cannot_rename, ObjectRole_CannotRename);

    const bool cannot_delete = object.get_system_flag(SystemFlagsBit_CannotDelete);
    item->setData(cannot_delete, ObjectRole_CannotDelete);

    const bool account_disabled = object.get_account_option(AccountOption_Disabled, g_adconfig);
    item->setData(account_disabled, ObjectRole_AccountDisabled);

    console_object_item_load_icon(item, account_disabled);
}

QList<QString> object_impl_column_labels() {
    QList<QString> out;

    for (const QString &attribute : g_adconfig->get_columns()) {
        const QString attribute_display_name = g_adconfig->get_column_display_name(attribute);

        out.append(attribute_display_name);
    }

    return out;
}

QList<int> object_impl_default_columns() {
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

    attributes += ATTRIBUTE_USER_ACCOUNT_CONTROL;

    // NOTE: needed to know which icon to use for object
    attributes += ATTRIBUTE_OBJECT_CATEGORY;

    // NOTE: for context menu block inheritance checkbox
    attributes += ATTRIBUTE_GPOPTIONS;

    // NOTE: needed to know gpo status
    attributes += ATTRIBUTE_FLAGS;

    return attributes;
}

// NOTE: it is possible for a search to start while a
// previous one hasn't finished. For that reason, this f-n
// contains multiple workarounds for issues caused by that
// case.
void console_object_search(ConsoleWidget *console, const QModelIndex &index, const QString &base, const SearchScope scope, const QString &filter, const QList<QString> &attributes) {
    auto search_id_matches = [](QStandardItem *item, SearchThread *thread) {
        const int id_from_item = item->data(MyConsoleRole_SearchThreadId).toInt();
        const int thread_id = thread->get_id();

        const bool match = (id_from_item == thread_id);

        return match;
    };

    QStandardItem *item = console->get_item(index);

    // Set icon to indicate that item is in "search" state
    item->setIcon(g_icon_manager->get_indicator_icon(g_icon_manager->search_indicator));

    // NOTE: need to set this role to disable actions during
    // fetch
    item->setData(true, ObjectRole_Fetching);
    item->setDragEnabled(false);

    auto search_thread = new SearchThread(base, scope, filter, attributes);

    // NOTE: change item's search thread, this will be used
    // later to handle situations where a thread is started
    // while another is running
    item->setData(search_thread->get_id(), MyConsoleRole_SearchThreadId);

    const QPersistentModelIndex persistent_index = index;

    // NOTE: need to pass console as receiver object to
    // connect() even though we're using lambda as a slot.
    // This is to be able to define queuedconnection type,
    // because there's no connect() version with no receiver
    // which has a connection type argument.
    QObject::connect(
        search_thread, &SearchThread::results_ready,
        console,
        [=](const QHash<QString, AdObject> &results) {
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

            QStandardItem *item_now = console->get_item(persistent_index);

            // NOTE: if another thread was started for this
            // item, abort this thread
            const bool thread_id_match = search_id_matches(item_now, search_thread);
            if (!thread_id_match) {
                search_thread->stop();

                return;
            }

            object_impl_add_objects_to_console(console, results.values(), persistent_index);
        },
        Qt::QueuedConnection);
    QObject::connect(
        search_thread, &SearchThread::finished,
        console,
        [=]() {
            if (!persistent_index.isValid()) {
                return;
            }

            g_status->display_ad_messages(search_thread->get_ad_messages(), console);
            search_thread_display_errors(search_thread, console);

            QStandardItem *item_now = console->get_item(persistent_index);

            // NOTE: if another thread was started for this
            // item, don't change item data. It will be
            // changed by that other thread.
            const bool thread_id_match = search_id_matches(item_now, search_thread);
            if (!thread_id_match) {
                return;
            }

            const bool is_disabled = item_now->data(ObjectRole_AccountDisabled).toBool();
            console_object_item_load_icon(item_now, is_disabled);

            item_now->setData(false, ObjectRole_Fetching);
            item_now->setDragEnabled(true);

            search_thread->deleteLater();
        },
        Qt::QueuedConnection);

    search_thread->start();
}

void console_object_tree_init(ConsoleWidget *console, AdInterface &ad) {
    const QList<QStandardItem *> row = console->add_scope_item(ItemType_Object, console->domain_info_index());
    auto root = row[0];

    const QString top_dn = g_adconfig->domain_dn();
    const AdObject top_object = ad.search_object(top_dn);
    console_object_item_data_load(root, top_object);

    const QString domain = g_adconfig->domain().toLower();
    root->setText(domain);
}

QModelIndex get_object_tree_root(ConsoleWidget *console) {
    const QString head_dn = g_adconfig->domain_dn();
    const QModelIndex console_root = console->domain_info_index();
    const QList<QModelIndex> search_results = console->search_items(console_root, ObjectRole_DN, head_dn, {ItemType_Object});

    if (!search_results.isEmpty()) {
        // NOTE: domain object may also appear in queries,
        // so have to find the real head by finding the
        // index with no parent
        for (const QModelIndex &index : search_results) {
            const QModelIndex parent = index.parent();

            if (parent == console->domain_info_index()) {
                return index;
            }
        }

        return QModelIndex();
    } else {
        return QModelIndex();
    }
}

QString console_object_count_string(ConsoleWidget *console, const QModelIndex &index) {
    const int count = console->get_child_count(index);
    const QString out = QCoreApplication::translate("object_impl", "%n object(s)", "", count);

    return out;
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
    const bool target_is_fetching = target.data(ObjectRole_Fetching).toBool();

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

QList<QString> get_selected_dn_list_object(ConsoleWidget *console) {
    return get_selected_dn_list(console, ItemType_Object, ObjectRole_DN);
}

QString get_selected_target_dn_object(ConsoleWidget *console) {
    return get_selected_target_dn(console, ItemType_Object, ObjectRole_DN);
}

void console_object_delete_dn_list(ConsoleWidget *console, const QList<QString> &dn_list, const QModelIndex &tree_root, const int type, const int dn_role) {
    for (const QString &dn : dn_list) {
        const QList<QModelIndex> index_list = console->search_items(tree_root, dn_role, dn, {type});
        const QList<QPersistentModelIndex> persistent_list = persistent_index_list(index_list);

        for (const QPersistentModelIndex &index : persistent_list) {
            console->delete_item(index);
        }
    }
}

bool can_create_class_at_parent(const QString &create_class, const QString &parent_class) {
    // NOTE: to get full list of possible
    // superiors, need to use the all of the parent
    // classes too, not just the leaf class
    const QList<QString> action_object_class_list = g_adconfig->get_inherit_chain(create_class);
    const QList<QString> possible_superiors = g_adconfig->get_possible_superiors(QList<QString>(action_object_class_list));
    const bool out = possible_superiors.contains(parent_class);

    return out;
}

void console_object_item_load_icon(QStandardItem *item, bool disabled) {
    const QString category = dn_get_name(item->data(ObjectRole_ObjectCategory).toString());
    QIcon icon;

    if (item->data(ConsoleRole_Type).toInt() == ItemType_QueryItem) {
        icon = g_icon_manager->get_object_icon(ADMC_CATEGORY_QUERY_ITEM);
        item->setIcon(icon);
        return;
    }

    if (category == OBJECT_CATEGORY_PERSON) {
        icon = disabled ? g_icon_manager->get_icon_for_type(ItemIconType_Person_Blocked) :
                          g_icon_manager->get_icon_for_type(ItemIconType_Person_Clean);
        item->setIcon(icon);
        return;
    }
    else if (category == OBJECT_CATEGORY_COMPUTER) {
        icon = disabled ? g_icon_manager->get_icon_for_type(ItemIconType_Computer_Blocked) :
                          g_icon_manager->get_icon_for_type(ItemIconType_Computer_Clean);
        item->setIcon(icon);
        return;
    }
    else if (category == OBJECT_CATEGORY_GROUP) {
        icon = g_icon_manager->get_icon_for_type(ItemIconType_Group_Clean);
        item->setIcon(icon);
        return;
    }

    icon = g_icon_manager->get_object_icon(category);
    item->setIcon(icon);
}

bool console_object_deletion_dialog(ConsoleWidget *console, const QList<QModelIndex> &index_deleted_list) {
    QString main_message;
    if (index_deleted_list.size() == 1) {
        main_message = QCoreApplication::translate("ObjectImpl", "Are you sure you want to delete this object?");
    }
    else {
        main_message = QCoreApplication::translate("ObjectImpl", "Are you sure you want to delete these objects?");
    }

    QString contains_objects_message;
    int not_empty_containers_count = 0;
    for (QModelIndex index : index_deleted_list) {
        if (index.model()->hasChildren(index)) {
            ++not_empty_containers_count;
        }
    }
    if (not_empty_containers_count == 1 && index_deleted_list.size() == 1) {
        contains_objects_message = QCoreApplication::translate("ObjectImpl", " It contains other objects.");
    }
    else if (not_empty_containers_count >= 1) {
        contains_objects_message = QCoreApplication::translate("ObjectImpl", " Containers to be deleted contain other objects.");
    }

    const bool confirm_actions = settings_get_variant(SETTING_confirm_actions).toBool();
    if (not_empty_containers_count > 0 || confirm_actions) {
        QMessageBox::StandardButton answer = QMessageBox::question(console, QObject::tr("Confirm action"), main_message + contains_objects_message);
        return answer == QMessageBox::Yes;
    }
    return true;
}
