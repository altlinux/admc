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

#include "console_impls/policy_impl.h"

#include "adldap.h"
#include "console_impls/find_policy_impl.h"
#include "console_impls/found_policy_impl.h"
#include "console_impls/item_type.h"
#include "console_impls/policy_impl.h"
#include "console_impls/policy_ou_impl.h"
#include "console_impls/policy_root_impl.h"
#include "globals.h"
#include "gplink.h"
#include "policy_results_widget.h"
#include "properties_dialog.h"
#include "rename_policy_dialog.h"
#include "select_object_dialog.h"
#include "status.h"
#include "utils.h"

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

    connect(
        add_link_action, &QAction::triggered,
        this, &PolicyImpl::on_add_link);
    connect(
        edit_action, &QAction::triggered,
        this, &PolicyImpl::on_edit);
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
    const bool perms_ok = ad.gpo_check_perms(selected_gpo, &ok);

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

    policy_results->update(selected_gpo);
}

QList<QAction *> PolicyImpl::get_all_custom_actions() const {
    return {
        add_link_action,
        edit_action,
    };
}

QSet<QAction *> PolicyImpl::get_custom_actions(const QModelIndex &index, const bool single_selection) const {
    UNUSED_ARG(index);

    QSet<QAction *> out;

    if (single_selection) {
        out.insert(add_link_action);
        out.insert(edit_action);
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
    UNUSED_ARG(index_list);

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

void console_policy_load(const QList<QStandardItem *> &row, const AdObject &object) {
    QStandardItem *main_item = row[0];
    console_policy_load_item(main_item, object);
}

void console_policy_load_item(QStandardItem *main_item, const AdObject &object) {
    const QIcon icon = get_object_icon(object);
    main_item->setIcon(icon);
    main_item->setData(object.get_dn(), PolicyRole_DN);

    const QString display_name = object.get_string(ATTRIBUTE_DISPLAY_NAME);
    main_item->setText(display_name);
}

void console_policy_edit(ConsoleWidget *console, const int item_type, const int dn_role) {
    const QString dn = get_selected_target_dn(console, item_type, dn_role);

    // TODO: remove this when gpui is able to load
    // policy name on their own
    const QString policy_name = [&]() {
        AdInterface ad;
        if (ad_failed(ad, console)) {
            return QString();
        }

        const AdObject object = ad.search_object(dn);
        return object.get_string(ATTRIBUTE_DISPLAY_NAME);
    }();

    const QString path = [&]() {
        AdInterface ad;
        if (ad_failed(ad, console)) {
            return QString();
        }

        const AdObject object = ad.search_object(dn);
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
            const QString error_text = "Failed to start gpui. Check that it's installed.";
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
        auto apply_changes = [&ou_dn, &dn_list, policy_results](ConsoleWidget *target_console) {
            const QModelIndex policy_root = get_policy_tree_root(target_console);

            // NOTE: there can be duplicate items for
            // one policy because policy may be
            // displayed under multiple OU's
            if (policy_root.isValid()) {
                const QModelIndex ou_index = target_console->search_item(policy_root, PolicyOURole_DN, ou_dn, {ItemType_PolicyOU});

                if (ou_index.isValid()) {
                    for (const QString &dn : dn_list) {
                        const QModelIndex gpo_index = target_console->search_item(ou_index, PolicyRole_DN, dn, {ItemType_Policy});

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
    const QList<QString> dn_list = get_selected_dn_list(console_list[0], item_type, dn_role);

    const QString confirmation_text = QCoreApplication::translate("PolicyImpl", "Are you sure you want to delete this policy and all of it's links?");
    const bool confirmed = confirmation_dialog(confirmation_text, console_list[0]);
    QStringList not_deleted_dn_list;
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

    if (!not_deleted_dn_list.isEmpty())
    {
        QString message;
        if (not_deleted_dn_list.size() == 1)
        {
            message = PolicyImpl::tr("Failed to delete group policy");
            AdObject not_deleted_object = ad.search_object(not_deleted_dn_list.first());
            if (!not_deleted_object.is_empty() && not_deleted_object.get_bool("isCriticalSystemObject"))
                message += PolicyImpl::tr(": this is a critical policy");
        }
        else
        {
            message = PolicyImpl::tr("Failed to delete the following group policies: \n");
            for (QString not_deleted_dn : not_deleted_dn_list)
            {
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

        ad.attribute_replace_string(ou_dn, ATTRIBUTE_GPLINK, gplink.to_string());
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
