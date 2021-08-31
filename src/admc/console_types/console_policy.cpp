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

#include "console_types/console_policy.h"

#include "adldap.h"
#include "central_widget.h"
#include "console_types/object_impl.h"
#include "globals.h"
#include "gplink.h"
#include "policy_results_widget.h"
#include "settings.h"
#include "status.h"
#include "utils.h"
#include "rename_policy_dialog.h"
#include "select_object_dialog.h"
#include "create_policy_dialog.h"
#include "central_widget.h"
#include "item_type.h"

#include <QCoreApplication>
#include <QDebug>
#include <QList>
#include <QMenu>
#include <QStandardItem>
#include <QProcess>

void console_policy_load(const QList<QStandardItem *> &row, const AdObject &object) {
    QStandardItem *main_item = row[0];
    main_item->setIcon(QIcon::fromTheme("folder-templates"));
    main_item->setData(object.get_dn(), PolicyRole_DN);
    
    const QString display_name = object.get_string(ATTRIBUTE_DISPLAY_NAME);
    row[0]->setText(display_name);
}

QList<QString> console_policy_header_labels() {
    return {QCoreApplication::translate("policy_model", "Name")};
}

QList<int> console_policy_default_columns() {
    return {0};
}

QList<QString> console_policy_search_attributes() {
    return {ATTRIBUTE_DISPLAY_NAME};
}

void ConsolePolicyRoot::create_policy_in_console(const AdObject &object) {
    const QModelIndex policy_root_index = [&]() {
        const QList<QModelIndex> search_results = console->search_items(QModelIndex(), ConsoleRole_Type, ItemType_PolicyRoot, ItemType_PolicyRoot);

        if (!search_results.isEmpty()) {
            return search_results[0];
        } else {
            return QModelIndex();
        }
    }();

    if (!policy_root_index.isValid()) {
        qDebug() << "Policy tree head index is invalid";

        return;
    }

    const QList<QStandardItem *> row = console->add_scope_item(ItemType_Policy, policy_root_index);

    console_policy_load(row, object);
}

void console_policy_tree_init(ConsoleWidget *console, AdInterface &ad) {
    const QList<QStandardItem *> head_row = console->add_scope_item(ItemType_PolicyRoot, QModelIndex());
    auto policy_tree_head = head_row[0];
    policy_tree_head->setText(QCoreApplication::translate("policy", "Group Policy Objects"));
    policy_tree_head->setDragEnabled(false);
    policy_tree_head->setIcon(QIcon::fromTheme("folder"));
}

bool ConsolePolicy::can_drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type) {
    const bool dropped_are_objects = (dropped_type_list == QSet<int>({ItemType_Object}));
    if (!dropped_are_objects) {
        return false;
    }

    const bool dropped_contain_ou = [&]() {
        for (const QPersistentModelIndex &index : dropped_list) {
            if (console_object_is_ou(index)) {
                return true;
            }
        }

        return false;
    }();

    return dropped_contain_ou;
}

void ConsolePolicy::drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type) {
    const QString policy_dn = target.data(PolicyRole_DN).toString();
    const QList<QString> policy_list = {policy_dn};

    const QList<QString> ou_list = [&]() {
        QList<QString> out;

        // NOTE: when multi-selecting, selection may contain
        // a mix of OU and non-OU objects. In that case just
        // ignore non-OU objects and link OU's only
        for (const QPersistentModelIndex &index : dropped_list) {
            if (console_object_is_ou(index)) {
                const QString dn = index.data(ObjectRole_DN).toString();
                out.append(dn);
            }
        }

        return out;
    }();

    console_policy_add_link(console, policy_list, ou_list, policy_results_widget);

    // NOTE: don't need to sync changes in policy results
    // widget because when drag and dropping you will select
    // the policy which will update results automatically
}

void console_policy_add_link(ConsoleWidget *console, const QList<QString> &policy_list, const QList<QString> &ou_list, PolicyResultsWidget *policy_results_widget) {
    AdInterface ad;
    if (ad_failed(ad)) {
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

    g_status()->display_ad_messages(ad, console);
}

void ConsolePolicyRoot::fetch(const QModelIndex &index) {
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    const QString base = g_adconfig->domain_head();
    const SearchScope scope = SearchScope_All;
    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_GP_CONTAINER);
    const QList<QString> attributes = console_policy_search_attributes();
    const QHash<QString, AdObject> results = ad.search(base, scope, filter, attributes);

    for (const AdObject &object : results.values()) {
        create_policy_in_console(object);
    }
}

void ConsolePolicy::on_add_link() {
    auto dialog = new SelectObjectDialog({CLASS_OU}, SelectObjectDialogMultiSelection_Yes, console);
    dialog->setWindowTitle(QCoreApplication::translate("console_policy", "Add Link"));

    QObject::connect(
        dialog, &SelectObjectDialog::accepted,
        [=]() {
            const QList<QString> gpos = get_selected_dn_list(console, ItemType_Policy, PolicyRole_DN);

            const QList<QString> ou_list = dialog->get_selected();

            console_policy_add_link(console, gpos, ou_list, policy_results_widget);

            const QModelIndex current_scope = console->get_current_scope_item();
            policy_results_widget->update(current_scope);
        });

    dialog->open();
}

void ConsolePolicy::on_edit() {
    const QString dn = get_selected_dn(console, ItemType_Policy, PolicyRole_DN);

    const QString filesys_path = [&]() {
        AdInterface ad;
        if (ad_failed(ad)) {
            return QString();
        }

        const AdObject object = ad.search_object(dn);
        const QString out = object.get_string(ATTRIBUTE_GPC_FILE_SYS_PATH);

        return out;
    }();

    auto process = new QProcess(console);
    process->setProgram("gpui");

    const QList<QString> args = {
        dn,
        filesys_path,
    };
    process->setArguments(args);

    QObject::connect(
        process, &QProcess::errorOccurred,
        [this](QProcess::ProcessError error) {
            const bool failed_to_start = (error == QProcess::FailedToStart);

            if (failed_to_start) {
                const QString error_text = "Failed to start gpui. Check that it's installed.";
                qDebug() << error_text;
                g_status()->add_message(error_text, StatusType_Error);
                error_log({error_text}, console);
            }
        });

    process->start(QIODevice::ReadOnly);
}

ConsolePolicy::ConsolePolicy(PolicyResultsWidget *policy_results_widget_arg, ConsoleWidget *console_arg)
: ConsoleImpl(console_arg) {
    policy_results_widget = policy_results_widget_arg;

    rename_dialog = new RenamePolicyDialog(console_arg);

    add_link_action = new QAction(tr("Add link..."), this);
    edit_action = new QAction(tr("Edit..."), this);

    connect(
        add_link_action, &QAction::triggered,
        this, &ConsolePolicy::on_add_link);
    connect(
        edit_action, &QAction::triggered,
        this, &ConsolePolicy::on_edit);
}

void ConsolePolicy::selected_as_scope(const QModelIndex &index) {
    policy_results_widget->update(index);
}

QList<QAction *> ConsolePolicy::get_all_custom_actions() const {
    return {
        add_link_action,
        edit_action,
    };
}

QSet<QAction *> ConsolePolicy::get_custom_actions(const QModelIndex &index, const bool single_selection) const {
    QSet<QAction *> out;

    if (single_selection) {
        out.insert(add_link_action);
        out.insert(edit_action);
    }

    return out;
}

QSet<StandardAction> ConsolePolicy::get_standard_actions(const QModelIndex &index, const bool single_selection) const {
    QSet<StandardAction> out;

    out.insert(StandardAction_Delete);

    if (single_selection) {
        out.insert(StandardAction_Rename);
        out.insert(StandardAction_Refresh);
    }

    return out;
}

void ConsolePolicy::rename(const QList<QModelIndex> &index_list) {
    rename_dialog->open();
}

void ConsolePolicy::delete_action(const QList<QModelIndex> &index_list) {
    const bool confirmed = confirmation_dialog(QCoreApplication::translate("console_policy", "Are you sure you want to delete this policy and all of it's links?"), console);
    if (!confirmed) {
        return;
    }

    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    show_busy_indicator();

    const QList<QPersistentModelIndex> persistent_list = persistent_index_list(index_list);

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

    hide_busy_indicator();

    g_status()->display_ad_messages(ad, console);
}

void ConsolePolicy::refresh(const QList<QModelIndex> &index_list) {
    if (index_list.size() != 1) {
        return;
    }

    const QModelIndex index = index_list[0];

    policy_results_widget->update(index);
}

ConsolePolicyRoot::ConsolePolicyRoot(ConsoleWidget *console_arg)
: ConsoleImpl(console_arg) {
    create_policy_dialog = new CreatePolicyDialog(console);

    create_policy_action = new QAction(tr("Create policy"), this);

    connect(
        create_policy_action, &QAction::triggered,
        create_policy_dialog, &QDialog::open);
    connect(
        create_policy_dialog, &CreatePolicyDialog::created_policy,
        this, &ConsolePolicyRoot::on_dialog_created_policy);
}

QList<QAction *> ConsolePolicyRoot::get_all_custom_actions() const {
    QList<QAction *> out;

    out.append(create_policy_action);

    return out;
}

QSet<QAction *> ConsolePolicyRoot::get_custom_actions(const QModelIndex &index, const bool single_selection) const {
    QSet<QAction *> out;

    out.insert(create_policy_action);

    return out;
}

QSet<StandardAction> ConsolePolicyRoot::get_standard_actions(const QModelIndex &index, const bool single_selection) const {
    QSet<StandardAction> out;

    out.insert(StandardAction_Refresh);

    return out;
}

void ConsolePolicyRoot::refresh(const QList<QModelIndex> &index_list) {
    if (index_list.size() != 1) {
        return;
    }

    const QModelIndex index = index_list[0];

    console->delete_children(index);
    fetch(index);
}

// NOTE: not adding policy object to the domain
// tree, but i think it's ok?
void ConsolePolicyRoot::on_dialog_created_policy(const QString &dn) {
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    const QString base = dn;
    const SearchScope scope = SearchScope_Object;
    const QString filter = QString();
    const QList<QString> attributes = console_policy_search_attributes();
    const QHash<QString, AdObject> results = ad.search(base, scope, filter, attributes);

    const AdObject object = results[dn];

    create_policy_in_console(object);
}
