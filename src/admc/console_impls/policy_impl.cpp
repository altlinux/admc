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

#include "console_impls/policy_impl.h"

#include "adldap.h"
#include "console_impls/find_policy_impl.h"
#include "console_impls/found_policy_impl.h"
#include "console_impls/item_type.h"
#include "console_impls/policy_impl.h"
#include "console_impls/policy_ou_impl.h"
#include "console_impls/policy_root_impl.h"
#include "globals.h"
#include "results_widgets/policy_results_widget.h"
#include "properties_widgets/properties_dialog.h"
#include "rename_dialogs/rename_policy_dialog.h"
#include "select_dialogs/select_object_dialog.h"
#include "status.h"
#include "utils.h"
#include "managers/icon_manager.h"
#include "fsmo/fsmo_utils.h"
#include "managers/gplink_manager.h"

#include <QAction>
#include <QDebug>
#include <QMessageBox>
#include <QStandardItem>

void policy_add_links(const QList<ConsoleWidget *> &console_list, PolicyResultsWidget *policy_results, const QList<QString> &policy_list, const QList<QString> &ou_list);
void console_policy_update_policy_results(ConsoleWidget *console, PolicyResultsWidget *policy_results);
void console_policy_remove_link(const QList<ConsoleWidget *> &console_list, PolicyResultsWidget *policy_results, const int item_type, const int dn_role, const QString &ou_dn);

PolicyImpl::PolicyImpl(ConsoleWidget *console_arg)
: ConsoleImpl(console_arg) {
    policy_results = new PolicyResultsWidget(console_arg);
    set_results_widget(policy_results);

    add_link_action = new QAction(tr("Add link..."), this);
    edit_action = new QAction(tr("Edit..."), this);
    enforce_action = new QAction(tr("Enforced"), this);
    enforce_action->setCheckable(true);
    enforce_action->setData(GplinkOption_Enforced);
    disable_action = new QAction(tr("Disabled"), this);
    disable_action->setCheckable(true);
    disable_action->setData(GplinkOption_Disabled);
    update_gplink_option_check_actions();

    connect(
        add_link_action, &QAction::triggered,
        this, &PolicyImpl::on_add_link);
    connect(
        edit_action, &QAction::triggered,
        this, &PolicyImpl::on_edit);
    connect(
        policy_results, &PolicyResultsWidget::ou_gplink_changed,
        this, &PolicyImpl::on_ou_gplink_changed);
    connect(
        enforce_action, &QAction::triggered, [this](){on_change_gplink_option_action(enforce_action);});
    connect(
        disable_action, &QAction::triggered, [this](){on_change_gplink_option_action(disable_action);});
}

bool PolicyImpl::can_drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type) {
    UNUSED_ARG(target);
    UNUSED_ARG(target_type);
    UNUSED_ARG(dropped_list);

    const bool dropped_are_policy_ou = (dropped_type_list == QSet<int>({ItemType_PolicyOU}));

    return dropped_are_policy_ou;
}

void PolicyImpl::drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type) {
    UNUSED_ARG(target_type);
    UNUSED_ARG(dropped_type_list);

    const QString policy_dn = target.data(PolicyRole_DN).toString();
    const QList<QString> policy_list = {policy_dn};

    const QList<QModelIndex> dropped_list_normal = normal_index_list(dropped_list);
    const QList<QString> ou_list = index_list_to_dn_list(dropped_list_normal, PolicyOURole_DN);

    policy_add_links({console}, policy_results, policy_list, ou_list);
}

void PolicyImpl::selected_as_scope(const QModelIndex &index) {
    AdInterface ad;
    if (ad_failed(ad, console)) {
        return;
    }

    // When selecting a policy, check it's permissions to
    // make sure that they permissions of GPT and GPC match.
    // If they don't, offer to update GPT permissions.
    const QString selected_gpo = index.data(PolicyRole_DN).toString();
    bool ok = true;
    bool perms_ok = true;
    const bool was_fetched = console_item_get_was_fetched(index);
    // Dont check perms if item was already fetched. It helps to avoid excessive freezes.
    if (!was_fetched) {
        perms_ok  = ad.gpo_check_perms(selected_gpo, &ok);
    }

    if (!perms_ok && ok) {
        const QString title = tr("Incorrect permissions detected");
        const QString text = tr("Permissions for this policy's GPT don't match the permissions for it's GPC object. Would you like to update GPT permissions?");

        auto sync_warning_dialog = new QMessageBox(console);
        sync_warning_dialog->setAttribute(Qt::WA_DeleteOnClose);
        sync_warning_dialog->setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        sync_warning_dialog->setWindowTitle(title);
        sync_warning_dialog->setText(text);
        sync_warning_dialog->setIcon(QMessageBox::Warning);

        connect(
            sync_warning_dialog, &QDialog::accepted,
            console,
            [this, selected_gpo]() {
                AdInterface ad_inner;
                if (ad_failed(ad_inner, console)) {
                    return;
                }

                ad_inner.gpo_sync_perms(selected_gpo);

                g_status->display_ad_messages(ad_inner, console);
            });
    }

    g_status->log_messages(ad);

    policy_results->update(index);
}

QList<QAction *> PolicyImpl::get_all_custom_actions() const {
    return {
        enforce_action,
        disable_action,
        add_link_action,
        edit_action,
    };
}

QSet<QAction *> PolicyImpl::get_custom_actions(const QModelIndex &index, const bool single_selection) const {
    QSet<QAction *> out;

    if (single_selection) {
        out.insert(add_link_action);
        out.insert(edit_action);
        if (index.parent().data(ConsoleRole_Type).toInt() == ItemType_PolicyOU)
        {
            update_gplink_option_check_actions();
            out.insert(enforce_action);
            out.insert(disable_action);
        }
    }

    return out;
}

QSet<StandardAction> PolicyImpl::get_standard_actions(const QModelIndex &index, const bool single_selection) const {
    UNUSED_ARG(index);

    QSet<StandardAction> out;

    out.insert(StandardAction_Delete);

    if (single_selection) {
        out.insert(StandardAction_Rename);
        out.insert(StandardAction_Refresh);
        out.insert(StandardAction_Properties);
    }

    return out;
}

void PolicyImpl::rename(const QList<QModelIndex> &index_list) {
    UNUSED_ARG(index_list);

    console_policy_rename({console}, policy_results, ItemType_Policy, PolicyRole_DN);
}

void PolicyImpl::delete_action(const QList<QModelIndex> &index_list) {
    const QModelIndex parent_index = index_list[0].parent();
    const ItemType parent_type = (ItemType) console_item_get_type(parent_index);
    const bool parent_is_ou = (parent_type == ItemType_PolicyOU);

    if (parent_is_ou) {
        const QString ou_dn = parent_index.data(PolicyOURole_DN).toString();

        console_policy_remove_link({console}, policy_results, ItemType_Policy, PolicyRole_DN, ou_dn);
    } else {
        console_policy_delete({console}, policy_results, ItemType_Policy, PolicyRole_DN);
    }
}

void PolicyImpl::refresh(const QList<QModelIndex> &index_list) {
    const QModelIndex index = index_list[0];

    policy_results->update(index);
}

void PolicyImpl::properties(const QList<QModelIndex> &index_list) {
    UNUSED_ARG(index_list);

    console_policy_properties({console}, policy_results, ItemType_Policy, PolicyRole_DN);
}

void PolicyImpl::on_add_link() {
    console_policy_add_link({console}, policy_results, ItemType_Policy, PolicyRole_DN);
}

void PolicyImpl::on_edit() {
    console_policy_edit(console, ItemType_Policy, PolicyRole_DN);
}

void PolicyImpl::on_ou_gplink_changed(const QString &ou_dn, const Gplink &gplink, const QString &policy_dn, GplinkOption option) {
    QModelIndex ou_item_index = search_policy_ou_index(console, ou_dn);
    if (!ou_item_index.isValid())
        return;

    update_ou_gplink_data(gplink.to_string(), ou_item_index);

    QModelIndex target_policy_index = get_ou_child_policy_index(console, ou_item_index, policy_dn);
    if (!target_policy_index.isValid())
        return;

    // Case of link deletion
    if (!gplink.contains(policy_dn)) {
        console->delete_item(target_policy_index);
        return;
    }

    // Case of option change
    if (option != GplinkOption_NoOption) {
        set_policy_item_icon(target_policy_index, gplink.get_option(policy_dn, option), option);
    }
}

void PolicyImpl::on_change_gplink_option_action(QAction *action)
{
    AdInterface ad;
    if (ad_failed(ad, console)) {
        return;
    }

    GplinkOption option = (GplinkOption)action->data().toInt();
    const QModelIndex policy_index = console->get_current_scope_item();
    const QString gpo_dn = policy_index.data(PolicyRole_DN).toString();
    const QModelIndex ou_index = policy_index.parent();
    const QString ou_dn = ou_index.data(PolicyOURole_DN).toString();

    bool checked = action->isChecked();

    const QString gplink_string = g_gplink_manager->ou_gplink(ou_dn);
    Gplink gplink = Gplink(gplink_string);
    gplink.set_option(gpo_dn, option, checked);
    const QString updated_gplink_string = gplink.to_string();

    bool success = ad.attribute_replace_string(ou_dn, ATTRIBUTE_GPLINK, updated_gplink_string);
    if (success) {
        update_ou_gplink_data(updated_gplink_string, ou_index);
        set_policy_item_icon(policy_index, checked, option);
        policy_results->update(policy_index);
    }
    else {
        action->toggle();
    }

    g_status->display_ad_messages(ad, console);
}

void PolicyImpl::update_gplink_option_check_actions() const
{
    const QModelIndex policy_index = console->get_current_scope_item();
    if (policy_index.parent().data(ConsoleRole_Type).toInt() != ItemType_PolicyOU)
        return;

    QStandardItem *item = console->get_item(policy_index);
    enforce_action->setChecked(policy_is_enforced(item));
    disable_action->setChecked(policy_is_disabled(item));
}

void PolicyImpl::set_policy_item_icon(const QModelIndex &policy_index, bool is_checked, GplinkOption option)
{
    QStandardItem *target_policy_item = console->get_item(policy_index);
    if (target_policy_item->parent()->data(ConsoleRole_Type).toInt() != ItemType_PolicyOU)
        return;

    if (option == GplinkOption_Enforced) {
        set_enforced_policy_icon(target_policy_item, is_checked);
    } else if (option == GplinkOption_Disabled) {
        set_disabled_policy_icon(target_policy_item, is_checked);
    }
}

void set_policy_link_icon(QStandardItem *policy_item, bool is_enforced, bool is_disabled) {
    if (is_enforced) {
        if (!is_disabled)
            policy_item->setIcon(g_icon_manager->item_icon(ItemIcon_Policy_Enforced));
        else
            policy_item->setIcon(g_icon_manager->item_icon(ItemIcon_Policy_Enforced_Disabled));
    } else {
        if (!is_disabled)
            policy_item->setIcon(g_icon_manager->item_icon(ItemIcon_Policy_Link));
        else
            policy_item->setIcon(g_icon_manager->item_icon(ItemIcon_Policy_Link_Disabled));
    }
}

void set_enforced_policy_icon(QStandardItem *policy_item, bool is_enforced) {
    bool is_disabled = policy_is_disabled(policy_item);

    set_policy_link_icon(policy_item, is_enforced, is_disabled);
}

void set_disabled_policy_icon(QStandardItem *policy_item, bool is_disabled)
{
    bool is_enforced = policy_is_enforced(policy_item);

    set_policy_link_icon(policy_item, is_enforced, is_disabled);
}

void console_policy_load(const QList<QStandardItem *> &row, const AdObject &object) {
    QStandardItem *main_item = row[0];
    console_policy_load_item(main_item, object);
}

void console_policy_load_item(QStandardItem *main_item, const AdObject &object) {
    main_item->setData(object.get_dn(), PolicyRole_DN);

    if (main_item->parent() != nullptr &&
            main_item->parent()->data(ConsoleRole_Type).toInt() == ItemType_PolicyOU) {
        bool is_enforced = policy_is_enforced(main_item);
        bool is_disabled = policy_is_disabled(main_item);

        set_policy_link_icon(main_item, is_enforced, is_disabled);
    } else {
        main_item->setIcon(g_icon_manager->object_icon(object));
    }

    const QString display_name = object.get_string(ATTRIBUTE_DISPLAY_NAME);
    main_item->setText(display_name);
    const QString gpo_status = gpo_status_from_int(object.get_int(ATTRIBUTE_FLAGS));
    main_item->setData(gpo_status, PolicyRole_GPO_Status);
}

void console_policy_edit(ConsoleWidget *console, const int item_type, const int dn_role) {
    const QString dn = get_selected_target_dn(console, item_type, dn_role);

    console_policy_edit(dn, console);
}

void console_policy_rename(const QList<ConsoleWidget *> &console_list, PolicyResultsWidget *policy_results, const int item_type, const int dn_role) {
    AdInterface ad;
    if (ad_failed(ad, console_list[0])) {
        return;
    }

    const QString dn = get_selected_target_dn(console_list[0], item_type, dn_role);

    auto dialog = new RenamePolicyDialog(ad, dn, console_list[0]);
    dialog->open();

    QObject::connect(
        dialog, &QDialog::accepted,
        console_list[0],
        [console_list, policy_results, dn]() {
            AdInterface ad_inner;
            if (ad_failed(ad_inner, console_list[0])) {
                return;
            }

            const AdObject object = ad_inner.search_object(dn);

            // NOTE: ok to not update dn after rename
            // because policy rename doesn't change dn, since "policy name" is displayName
            auto apply_changes = [&dn, &object, policy_results](ConsoleWidget *target_console) {
                const QModelIndex policy_root = get_policy_tree_root(target_console);

                // NOTE: there can be duplicate items for
                // one policy because policy may be
                // displayed under multiple OU's
                if (policy_root.isValid()) {
                    const QList<QModelIndex> index_list = target_console->search_items(policy_root, PolicyRole_DN, dn, {ItemType_Policy});

                    for (const QModelIndex &index : index_list) {
                        const QList<QStandardItem *> row = target_console->get_row(index);
                        console_policy_load(row, object);
                    }
                }

                const QModelIndex find_policy_root = get_find_policy_root(target_console);

                if (find_policy_root.isValid()) {
                    const QModelIndex index = target_console->search_item(find_policy_root, FoundPolicyRole_DN, dn, {ItemType_FoundPolicy});

                    if (index.isValid()) {
                        const QList<QStandardItem *> row = target_console->get_row(index);
                        console_policy_load(row, object);
                    }
                }

                console_policy_update_policy_results(target_console, policy_results);
            };

            for (ConsoleWidget *target_console : console_list) {
                apply_changes(target_console);
            }
        });
}

void console_policy_add_link(const QList<ConsoleWidget *> &console_list, PolicyResultsWidget *policy_results, const int item_type, const int dn_role) {
    auto dialog = new SelectObjectDialog({CLASS_OU}, SelectObjectDialogMultiSelection_Yes, console_list[0]);
    dialog->setWindowTitle(QCoreApplication::translate("PolicyImpl", "Add Link"));
    dialog->open();

    QObject::connect(
        dialog, &SelectObjectDialog::accepted,
        console_list[0],
        [console_list, policy_results, dialog, item_type, dn_role]() {
            const QList<QString> gpo_list = get_selected_dn_list(console_list[0], item_type, dn_role);

            const QList<QString> ou_list = dialog->get_selected();

            policy_add_links(console_list, policy_results, gpo_list, ou_list);
        });
}

void console_policy_remove_link(const QList<ConsoleWidget *> &console_list, PolicyResultsWidget *policy_results, const int item_type, const int dn_role, const QString &ou_dn) {
    const QList<QString> dn_list = get_selected_dn_list(console_list[0], item_type, dn_role);

    const QString confirmation_text = QCoreApplication::translate("PolicyImpl", "Are you sure you want to unlink this policy from the OU? Note that the actual policy object won't be deleted.");
    const bool confirmed = confirmation_dialog(confirmation_text, console_list[0]);
    if (!confirmed) {
        return;
    }

    AdInterface ad;
    if (ad_failed(ad, console_list[0])) {
        return;
    }

    show_busy_indicator();

    const QString gplink_new_string = [&]() {
        Gplink gplink = [&]() {
            const AdObject parent_object = ad.search_object(ou_dn);
            const QString gplink_old_string = parent_object.get_string(ATTRIBUTE_GPLINK);
            const Gplink out = Gplink(gplink_old_string);

            return out;
        }();

        for (const QString &dn : dn_list) {
            gplink.remove(dn);
        }

        const QString out = gplink.to_string();

        return out;
    }();

    const bool replace_success = ad.attribute_replace_string(ou_dn, ATTRIBUTE_GPLINK, gplink_new_string);

    if (replace_success) {
        auto apply_changes = [&ou_dn, &dn_list, &gplink_new_string, policy_results](ConsoleWidget *target_console) {
            const QModelIndex policy_root = get_policy_tree_root(target_console);

            // NOTE: there can be duplicate items for
            // one policy because policy may be
            // displayed under multiple OU's
            if (policy_root.isValid()) {
                const QModelIndex ou_index = target_console->search_item(policy_root, PolicyOURole_DN, ou_dn, {ItemType_PolicyOU});

                if (ou_index.isValid()) {
                    update_ou_gplink_data(gplink_new_string, ou_index);

                    for (const QString &dn : dn_list) {
                        const QModelIndex gpo_index = get_ou_child_policy_index(target_console, ou_index, dn);

                        target_console->delete_item(gpo_index);
                    }
                }
            }

            console_policy_update_policy_results(target_console, policy_results);
        };

        for (ConsoleWidget *target_console : console_list) {
            apply_changes(target_console);
        }
    }

    hide_busy_indicator();

    g_status->display_ad_messages(ad, console_list[0]);
}

void console_policy_delete(const QList<ConsoleWidget *> &console_list, PolicyResultsWidget *policy_results, const int item_type, const int dn_role) {
    AdInterface ad;
    if (ad_failed(ad, console_list[0])) {
        return;
    }

    if (!current_dc_is_master_for_role(ad, FSMORole_PDCEmulation) && gpo_edit_without_PDC_disabled) {
        QMessageBox::StandardButton answer = QMessageBox::question(console_list[0], QObject::tr("Deletion is not available"),
                                                                   QObject::tr("ADMC is connected to DC without the PDC-Emulator role - "
                                                                   "group policy deletion is prohibited by the setting. "
                                                                   "Connect to PDC-Emulator?"));
        if (answer == QMessageBox::Yes) {
            connect_to_PDC_emulator(ad, console_list[0]);
            return;
        }
        else {
            return;
        }
    }

    const QList<QString> dn_list = get_selected_dn_list(console_list[0], item_type, dn_role);

    const QString confirmation_text = QCoreApplication::translate("PolicyImpl", "Are you sure you want to delete this policy and all of it's links?");
    const bool confirmed = confirmation_dialog(confirmation_text, console_list[0]);
    QStringList not_deleted_dn_list;
    if (!confirmed) {
        return;
    }

    show_busy_indicator();

    const QList<QString> deleted_list = [&]() {
        QList<QString> out;
        for (const QString &dn : dn_list) {
            bool deleted_object = false;
            ad.gpo_delete(dn, &deleted_object);

            // NOTE: object may get deleted successfuly but
            // deleting GPT fails which makes gpo_delete() fail
            // as a whole, but we still want to remove gpo from
            // the console in that case
            if (deleted_object) {
                out.append(dn);
            } else {
                not_deleted_dn_list.append(dn);
            }
        }

        return out;
    }();

    auto apply_changes = [&deleted_list, policy_results](ConsoleWidget *target_console) {
        const QModelIndex policy_root = get_policy_tree_root(target_console);

        // NOTE: there can be duplicate items for
        // one policy because policy may be
        // displayed under multiple OU's
        if (policy_root.isValid()) {
            for (const QString &dn : deleted_list) {
                const QList<QModelIndex> index_list = target_console->search_items(policy_root, PolicyRole_DN, dn, {ItemType_Policy});
                const QList<QPersistentModelIndex> persistent_list = persistent_index_list(index_list);

                for (const QPersistentModelIndex &index : persistent_list) {
                    target_console->delete_item(index);
                }
            }
        }

        const QModelIndex find_policy_root = get_find_policy_root(target_console);

        if (find_policy_root.isValid()) {
            for (const QString &dn : deleted_list) {
                const QList<QModelIndex> index_list = target_console->search_items(find_policy_root, FoundPolicyRole_DN, dn, {ItemType_FoundPolicy});
                const QList<QPersistentModelIndex> persistent_list = persistent_index_list(index_list);

                for (const QPersistentModelIndex &index : persistent_list) {
                    target_console->delete_item(index);
                }
            }
        }

        console_policy_update_policy_results(target_console, policy_results);
    };

    for (ConsoleWidget *target_console : console_list) {
        apply_changes(target_console);
    }

    hide_busy_indicator();

    g_status->log_messages(ad);
    if (!not_deleted_dn_list.isEmpty()) {
        QString message;
        if (not_deleted_dn_list.size() == 1) {
            message = PolicyImpl::tr("Failed to delete group policy");
            AdObject not_deleted_object = ad.search_object(not_deleted_dn_list.first());
            if (!not_deleted_object.is_empty() && not_deleted_object.get_bool("isCriticalSystemObject"))
                message += PolicyImpl::tr(": this is a critical policy");
        } else {
            message = PolicyImpl::tr("Failed to delete the following group policies: \n");
            for (QString not_deleted_dn : not_deleted_dn_list) {
                AdObject not_deleted_object = ad.search_object(not_deleted_dn);
                message += '\n' + not_deleted_object.get_string("displayName");
                if (not_deleted_object.get_bool("isCriticalSystemObject"))
                    message += PolicyImpl::tr(" (critical policy)");
            }
        }
        QMessageBox::warning(console_list[0], "", message);
    }
}

void console_policy_properties(const QList<ConsoleWidget *> &console_list, PolicyResultsWidget *policy_results, const int item_type, const int dn_role) {
    AdInterface ad;
    if (ad_failed(ad, console_list[0])) {
        return;
    }

    const QString dn = get_selected_target_dn(console_list[0], item_type, dn_role);

    bool dialog_is_new;
    PropertiesDialog *dialog = PropertiesDialog::open_for_target(ad, dn, &dialog_is_new);

    auto on_propeties_applied = [console_list, policy_results, dn]() {
        AdInterface ad_inner;
        if (ad_failed(ad_inner, console_list[0])) {
            return;
        }

        const AdObject object = ad_inner.search_object(dn);

        auto apply_changes = [policy_results, &dn, &object](ConsoleWidget *target_console) {
            const QModelIndex policy_root = get_policy_tree_root(target_console);

            if (policy_root.isValid()) {
                const QList<QModelIndex> index_list = target_console->search_items(policy_root, PolicyRole_DN, dn, {ItemType_Policy});

                for (const QModelIndex &index : index_list) {
                    const QList<QStandardItem *> row = target_console->get_row(index);
                    console_policy_load(row, object);
                }

                const QModelIndex find_policy_root = get_find_policy_root(target_console);

                if (find_policy_root.isValid()) {
                    const QModelIndex index = target_console->search_item(find_policy_root, FoundPolicyRole_DN, dn, {ItemType_FoundPolicy});

                    if (index.isValid()) {
                        const QList<QStandardItem *> row = target_console->get_row(index);
                        console_policy_load(row, object);
                    }
                }
            }

            console_policy_update_policy_results(target_console, policy_results);
        };

        for (ConsoleWidget *target_console : console_list) {
            apply_changes(target_console);
        }
    };

    if (dialog_is_new) {
        QObject::connect(
            dialog, &PropertiesDialog::applied,
            console_list[0], on_propeties_applied);
    }
}

void policy_add_links(const QList<ConsoleWidget *> &console_list, PolicyResultsWidget *policy_results, const QList<QString> &policy_list, const QList<QString> &ou_list) {
    AdInterface ad;
    if (ad_failed(ad, console_list[0])) {
        return;
    }

    show_busy_indicator();

    for (const QString &ou_dn : ou_list) {
        const QString base = ou_dn;
        const SearchScope scope = SearchScope_Object;
        const QString filter = QString();
        const QList<QString> attributes = {ATTRIBUTE_GPLINK};
        const QHash<QString, AdObject> results = ad.search(base, scope, filter, attributes);

        const AdObject ou_object = results[ou_dn];
        const QString gplink_string = ou_object.get_string(ATTRIBUTE_GPLINK);
        Gplink gplink = Gplink(gplink_string);

        for (const QString &policy : policy_list) {
            gplink.add(policy);
        }

        const QString gplink_str = gplink.to_string();
        ad.attribute_replace_string(ou_dn, ATTRIBUTE_GPLINK, gplink_str);
        g_gplink_manager->set_gplink(ou_dn, gplink_str);
    }

    // TODO: serch for all policy objects once, then add
    // them to OU's
    const QList<AdObject> gpo_object_list = [&]() {
        const QString base = g_adconfig->policies_dn();
        const SearchScope scope = SearchScope_Children;
        const QString filter = filter_dn_list(policy_list);
        const QList<QString> attributes = QList<QString>();

        const QHash<QString, AdObject> search_results = ad.search(base, scope, filter, attributes);

        const QList<AdObject> out = search_results.values();

        return out;
    }();

    // NOTE: ok to not update dn after rename
    // because policy rename doesn't change dn,
    // since "policy name" is displayName
    auto apply_changes = [&ou_list, &gpo_object_list](ConsoleWidget *target_console) {
        const QModelIndex policy_root = get_policy_tree_root(target_console);

        if (policy_root.isValid()) {
            for (const QString &ou_dn : ou_list) {
                const QModelIndex ou_index = target_console->search_item(policy_root, PolicyOURole_DN, ou_dn, {ItemType_PolicyOU});

                if (!ou_index.isValid()) {
                    continue;
                }

                const bool ou_was_fetched = console_item_get_was_fetched(ou_index);

                if (!ou_was_fetched) {
                    continue;
                }

                policy_ou_impl_add_objects_to_console(target_console, gpo_object_list, ou_index);
                target_console->refresh_scope(ou_index);
            }
        }
    };

    for (ConsoleWidget *target_console : console_list) {
        apply_changes(target_console);
    }

    console_policy_update_policy_results(console_list[0], policy_results);

    hide_busy_indicator();

    g_status->display_ad_messages(ad, console_list[0]);
}

// Update policy results widget if it's currently displayed
void console_policy_update_policy_results(ConsoleWidget *console, PolicyResultsWidget *policy_results) {
    if (policy_results == nullptr) {
        return;
    }

    // NOTE: we always use Policy item type because that is
    // the only type which uses PolicyResultsWidget
    const QString results_gpo = policy_results->get_current_gpo();
    const QString scope_gpo = get_selected_target_dn(console, ItemType_Policy, PolicyRole_DN);
    const bool results_is_shown = (results_gpo == scope_gpo);

    if (results_is_shown) {
        policy_results->update(results_gpo);
    }
}

bool policy_is_enforced(QStandardItem *policy_item)
{
    bool is_enforced = false;
    const QString ou_dn = policy_item->parent()->data(PolicyOURole_DN).toString();
    const QString gplink_string = g_gplink_manager->ou_gplink(ou_dn);
    const Gplink gplink = Gplink(gplink_string);
    is_enforced = gplink.enforced_gpo_dn_list().contains(policy_item->data(PolicyRole_DN).toString());

    return is_enforced;
}

bool policy_is_disabled(QStandardItem *policy_item)
{
    bool is_disabled = false;
    const QString ou_dn = policy_item->parent()->data(PolicyOURole_DN).toString();
    const QString gplink_string = g_gplink_manager->ou_gplink(ou_dn);
    const Gplink gplink = Gplink(gplink_string);
    is_disabled = gplink.disabled_gpo_dn_list().contains(policy_item->data(PolicyRole_DN).toString());

    return is_disabled;
}

void console_policy_edit(const QString &policy_dn, ConsoleWidget *console) {
    AdInterface ad;
    bool ad_fail = ad_failed(ad, console);
    if (ad_fail) {
        return;
    }

    if (!current_dc_is_master_for_role(ad, FSMORole_PDCEmulation) && gpo_edit_without_PDC_disabled) {
        QMessageBox::StandardButton answer = QMessageBox::question(console, QObject::tr("Edition is not available"),
                                                                   QObject::tr("ADMC is connected to DC without the PDC-Emulator role - "
                                                                   "group policy editing is prohibited by the setting. "
                                                                   "Connect to PDC-Emulator?"));
        if (answer == QMessageBox::Yes) {
            connect_to_PDC_emulator(ad, console);
            return;
        }
        else {
            return;
        }
    }

    // TODO: remove this when gpui is able to load
    // policy name on their own
    const QString policy_name = [&]() {
        const AdObject object = ad.search_object(policy_dn);
        return object.get_string(ATTRIBUTE_DISPLAY_NAME);
    }();

    const QString path = [&]() {
        const AdObject object = ad.search_object(policy_dn);
        QString filesys_path = object.get_string(ATTRIBUTE_GPC_FILE_SYS_PATH);

        const QString current_dc = ad.get_dc();

        filesys_path.replace(QString("\\"), QString("/"));
        auto contents = filesys_path.split("/", Qt::KeepEmptyParts);
        if (contents.size() > 3 && !current_dc.isEmpty()) {
            contents[2] = current_dc;
        }
        filesys_path = contents.join("/");
        filesys_path.prepend(QString("smb:"));

        return filesys_path;
    }();

    auto process = new QProcess(console);
    process->setProgram("gpui-main");

    const QList<QString> args = {
        QString("-p"),
        path,
        QString("-n"),
        policy_name,
    };

    process->setArguments(args);

    auto on_gpui_error = [console](QProcess::ProcessError error) {
        const bool failed_to_start = (error == QProcess::FailedToStart);

        if (failed_to_start) {
            const QString error_text = QObject::tr("Failed to start GPUI. Check that it's installed.");
            qDebug() << error_text;
            g_status->add_message(error_text, StatusType_Error);
            error_log({error_text}, console);
        }
    };

    QObject::connect(
        process, &QProcess::errorOccurred,
        console, on_gpui_error);

    process->start(QIODevice::ReadOnly);
}
