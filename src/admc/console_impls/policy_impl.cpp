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
#include "console_impls/item_type.h"
#include "console_impls/policy_ou_impl.h"
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
#include <QStandardItem>

PolicyImpl::PolicyImpl(ConsoleWidget *console_arg)
: ConsoleImpl(console_arg) {
    policy_results_widget = new PolicyResultsWidget(console_arg);
    set_results_widget(policy_results_widget);

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

    add_link(policy_list, ou_list);

    // Need to refresh so that results widget is updated
    // because linking a gpo changes contents of results.
    const QModelIndex current_scope = console->get_current_scope_item();
    console->refresh_scope(current_scope);
}

void PolicyImpl::selected_as_scope(const QModelIndex &index) {
    policy_results_widget->update(index);
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

    AdInterface ad;
    if (ad_failed(ad, console)) {
        return;
    }

    const QModelIndex index = console->get_selected_item(ItemType_Policy);
    const QString dn = index.data(PolicyRole_DN).toString();

    auto dialog = new RenamePolicyDialog(ad, dn, console);
    dialog->open();

    connect(
        dialog, &QDialog::accepted,
        this,
        [this, index, dn]() {
            AdInterface ad_inner;
            if (ad_failed(ad_inner, console)) {
                return;
            }

            const AdObject object = ad_inner.search_object(dn);

            const QList<QStandardItem *> row = console->get_row(index);
            console_policy_load(row, object);
        });
}

void PolicyImpl::delete_action(const QList<QModelIndex> &index_list) {
    if (index_list.isEmpty()) {
        return;
    }

    const QModelIndex parent_index = index_list[0].parent();
    const ItemType parent_type = (ItemType) console_item_get_type(parent_index);
    const bool parent_is_ou = (parent_type == ItemType_PolicyOU);
    const bool parent_is_all_policies = (parent_type == ItemType_AllPoliciesFolder);


    const QString confirmation_text = [&]() {
        if (parent_is_ou) {
            return tr("Are you sure you want to unlink this policy from the OU? Note that the actual policy object won't be deleted.");
        } else if (parent_is_all_policies) {
            return tr("Are you sure you want to delete this policy and all of it's links?");
        } else {
            return QString();
        }
    }();
    const bool confirmed = confirmation_dialog(confirmation_text, console);
    if (!confirmed) {
        return;
    }

    AdInterface ad;
    if (ad_failed(ad, console)) {
        return;
    }

    show_busy_indicator();

    const QList<QPersistentModelIndex> persistent_list = persistent_index_list(index_list);

    if (parent_is_ou) {
        const QString parent_dn = parent_index.data(PolicyOURole_DN).toString();

        const QString gplink_new_string = [&]() {
            Gplink gplink = [&]() {
                const AdObject parent_object = ad.search_object(parent_dn);
                const QString gplink_old_string = parent_object.get_string(ATTRIBUTE_GPLINK);
                const Gplink out = Gplink(gplink_old_string);

                return out;
            }();

            for (const QPersistentModelIndex &index : persistent_list) {
                const QString dn = index.data(PolicyRole_DN).toString();

                gplink.remove(dn);
            }

            const QString out = gplink.to_string();

            return out;
        }();

        const bool replace_success = ad.attribute_replace_string(parent_dn, ATTRIBUTE_GPLINK, gplink_new_string);

        if (replace_success) {
            for (const QPersistentModelIndex &index : persistent_list) {
                console->delete_item(index);
            }

            const QModelIndex current_scope = console->get_current_scope_item();
            policy_results_widget->update(current_scope);
        }
    } else if (parent_is_all_policies) {
        for (const QPersistentModelIndex &index : persistent_list) {
            const QString dn = index.data(PolicyRole_DN).toString();

            bool deleted_object = false;
            ad.gpo_delete(dn, &deleted_object);

            // NOTE: object may get deleted successfuly but
            // deleting GPT fails which makes gpo_delete() fail
            // as a whole, but we still want to remove gpo from
            // the console in that case
            if (deleted_object) {
                console->delete_item(index);
            }
        }
    }

    hide_busy_indicator();

    g_status->display_ad_messages(ad, console);
}

void PolicyImpl::refresh(const QList<QModelIndex> &index_list) {
    const QModelIndex index = index_list[0];

    policy_results_widget->update(index);
}

void PolicyImpl::properties(const QList<QModelIndex> &index_list) {
    AdInterface ad;
    if (ad_failed(ad, console)) {
        return;
    }

    const QModelIndex index = index_list[0];
    const QString dn = index.data(PolicyRole_DN).toString();

    bool dialog_is_new;
    PropertiesDialog *dialog = PropertiesDialog::open_for_target(ad, dn, &dialog_is_new);

    // Need to update results when properties are applied,
    // in case links were modified in links tab
    auto on_propeties_applied = [&]() {
        const QModelIndex current_scope = console->get_current_scope_item();
        policy_results_widget->update(current_scope);
    };

    if (dialog_is_new) {
        connect(
            dialog, &PropertiesDialog::applied,
            on_propeties_applied);
    }
}

void PolicyImpl::add_link(const QList<QString> &policy_list, const QList<QString> &ou_list) {
    AdInterface ad;
    if (ad_failed(ad, console)) {
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

    // Update policy results widget since link state changed
    const QModelIndex current_scope = console->get_current_scope_item();
    policy_results_widget->update(current_scope);

    hide_busy_indicator();

    g_status->display_ad_messages(ad, console);
}

void PolicyImpl::on_add_link() {
    auto dialog = new SelectObjectDialog({CLASS_OU}, SelectObjectDialogMultiSelection_Yes, console);
    dialog->setWindowTitle(tr("Add Link"));
    dialog->open();

    connect(
        dialog, &SelectObjectDialog::accepted,
        this,
        [this, dialog]() {
            const QList<QString> gpos = get_selected_dn_list(console, ItemType_Policy, PolicyRole_DN);

            const QList<QString> ou_list = dialog->get_selected();

            add_link(gpos, ou_list);

            const QModelIndex current_scope = console->get_current_scope_item();
            policy_results_widget->update(current_scope);
        });
}

void PolicyImpl::on_edit() {
    const QString dn = get_selected_target_dn(console, ItemType_Policy, PolicyRole_DN);

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

        filesys_path.replace(QString("\\"),QString("/"));
        auto contents = filesys_path.split("/", QString::KeepEmptyParts);
        if (contents.size() > 3 && !current_dc.isEmpty())
        {
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
        policy_name
    };

    process->setArguments(args);

    connect(
        process, &QProcess::errorOccurred,
        this, &PolicyImpl::on_gpui_error);

    process->start(QIODevice::ReadOnly);
}

void PolicyImpl::on_gpui_error(QProcess::ProcessError error) {
    const bool failed_to_start = (error == QProcess::FailedToStart);

    if (failed_to_start) {
        const QString error_text = "Failed to start gpui. Check that it's installed.";
        qDebug() << error_text;
        g_status->add_message(error_text, StatusType_Error);
        error_log({error_text}, console);
    }
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
