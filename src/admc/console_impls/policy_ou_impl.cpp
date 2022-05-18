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

#include "console_impls/policy_ou_impl.h"

#include "adldap.h"
#include "console_impls/item_type.h"
#include "console_impls/object_impl.h"
#include "console_impls/policy_impl.h"
#include "console_impls/all_policies_folder_impl.h"
#include "console_widget/results_view.h"
#include "create_policy_dialog.h"
#include "globals.h"
#include "gplink.h"
#include "policy_results_widget.h"
#include "rename_policy_dialog.h"
#include "select_object_dialog.h"
#include "settings.h"
#include "status.h"
#include "utils.h"
#include "search_thread.h"
#include "select_policy_dialog.h"

#include <QCoreApplication>
#include <QDebug>
#include <QList>
#include <QMenu>
#include <QProcess>
#include <QStandardItem>

// TODO: Duplicating code related to object search in
// object_impl, but there are some differences that
// complicate code sharing.

void console_policy_ou_search(ConsoleWidget *console, const QModelIndex &index, const QString &base, const SearchScope scope, const QString &filter, const QList<QString> &attributes);

PolicyOUImpl::PolicyOUImpl(ConsoleWidget *console_arg)
: ConsoleImpl(console_arg) {
    set_results_view(new ResultsView(console_arg));

    create_ou_action = new QAction(tr("Create OU"), this);
    link_gpo_action = new QAction(tr("Link existing GPO"), this);

    connect(
        create_ou_action, &QAction::triggered,
        this, &PolicyOUImpl::create_ou);
    connect(
        link_gpo_action, &QAction::triggered,
        this, &PolicyOUImpl::link_gpo);
}

// TODO: perform searches in separate threads
void PolicyOUImpl::fetch(const QModelIndex &index) {
    AdInterface ad;
    if (ad_failed(ad, console)) {
        return;
    }

    const QString dn = index.data(ObjectRole_DN).toString();
    const QString domain_dn = g_adconfig->domain_dn();
    const bool is_domain = (dn == domain_dn);

    // Add child OU's
    {
        const QString base = dn;
        const SearchScope scope = SearchScope_Children;
        const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_OU);
        const QList<QString> attributes = console_object_search_attributes();

        const QHash<QString, AdObject> results = ad.search(base, scope, filter, attributes);

        policy_ou_impl_add_objects_to_console(console, results.values(), index);
    }

    // Add "All policies" folder if this is domain
    if (is_domain) {
        const QList<QStandardItem *> all_policies_row = console->add_scope_item(ItemType_AllPoliciesFolder, index);
        QStandardItem *all_policies_item = all_policies_row[0];
        all_policies_item->setText(tr("All policies"));
        all_policies_item->setIcon(QIcon::fromTheme("folder"));
        // Set sort index for "All policies" to 1 so it's always
        // at the bottom of the policy tree
        console->set_item_sort_index(all_policies_item->index(), 1);
    }

    // Add policies linked to this OU
    if (!is_domain) {
        const QList<QString> gpo_list = [&]() {
            const AdObject parent_object = ad.search_object(dn);
            const QString gplink_string = parent_object.get_string(ATTRIBUTE_GPLINK);
            const Gplink gplink = Gplink(gplink_string);
            const QList<QString> out = gplink.get_gpo_list();

            return out;
        }();

        if (!gpo_list.isEmpty()) {
            const QString base = g_adconfig->policies_dn();
            const SearchScope scope = SearchScope_Children;
            const QString filter = [&]() {

                const QList<QString> subfilter_list = [&]() {
                    QList<QString> out;

                    for (const QString &gpo : gpo_list) {
                        const QString subfilter = filter_CONDITION(Condition_Equals, ATTRIBUTE_DN, gpo);
                        out.append(subfilter);
                    }

                    return out;
                }();

                const QString out = filter_OR(subfilter_list);

                return out;
            }();
            const QList<QString> attributes = {
                ATTRIBUTE_DISPLAY_NAME
            };

            const QHash<QString, AdObject> results = ad.search(base, scope, filter, attributes);

            for (const AdObject &object : results.values()) {
                const QList<QStandardItem *> row = console->add_scope_item(ItemType_Policy, index);

                console_policy_load(row, object);
            }
        }
    }
}

void PolicyOUImpl::refresh(const QList<QModelIndex> &index_list) {
    const QModelIndex index = index_list[0];

    console->delete_children(index);
    fetch(index);
}

void PolicyOUImpl::activate(const QModelIndex &index) {
    properties({index});
}

QList<QAction *> PolicyOUImpl::get_all_custom_actions() const {
    QList<QAction *> out;

    out.append(create_ou_action);
    out.append(link_gpo_action);

    return out;
}

QSet<QAction *> PolicyOUImpl::get_custom_actions(const QModelIndex &index, const bool single_selection) const {
    UNUSED_ARG(index);

    QSet<QAction *> out;

    if (single_selection) {
        out.insert(create_ou_action);
        out.insert(link_gpo_action);
    }

    return out;
}

QSet<StandardAction> PolicyOUImpl::get_standard_actions(const QModelIndex &index, const bool single_selection) const {
    UNUSED_ARG(index);
    UNUSED_ARG(single_selection);

    QSet<StandardAction> out;

    out.insert(StandardAction_Properties);

    const bool can_refresh = console_item_get_was_fetched(index);
    if (can_refresh) {
        out.insert(StandardAction_Refresh);
    }
    out.insert(StandardAction_Rename);
    out.insert(StandardAction_Delete);

    return out;
}

QList<QString> PolicyOUImpl::column_labels() const {
    return {tr("Name")};
}

QList<int> PolicyOUImpl::default_columns() const {
    return {0};
}

void PolicyOUImpl::create_ou() {
    const QString parent_dn = get_selected_target_dn(console, ItemType_PolicyOU, ObjectRole_DN);

    console_object_create(console, nullptr, CLASS_OU, parent_dn);
}

void PolicyOUImpl::link_gpo() {
    AdInterface ad;
    if (ad_failed(ad, console)) {
        return;
    }

    auto dialog = new SelectPolicyDialog(ad, console);
    dialog->open();

    connect(
        dialog, &QDialog::accepted,
        this,
        [this, dialog]() {
            AdInterface ad_inner;
            if (ad_failed(ad_inner, console)) {
                return;
            }

            const QString target_dn = get_selected_target_dn(console, ItemType_PolicyOU, ObjectRole_DN);
            
            Gplink gplink = [&]() {
                const AdObject target_object = ad_inner.search_object(target_dn);
                const QString gplink_string = target_object.get_string(ATTRIBUTE_GPLINK);
                const Gplink out = Gplink(gplink_string);

                return out;
            }();

            const QList<QString> gpo_list = dialog->get_selected_dns();

            for (const QString &gpo : gpo_list) {
                gplink.add(gpo);
            }

            const QString new_gplink_string = gplink.to_string();
            ad_inner.attribute_replace_string(target_dn, ATTRIBUTE_GPLINK, new_gplink_string);

            g_status->display_ad_messages(ad_inner, console);
        });
}

void PolicyOUImpl::properties(const QList<QModelIndex> &index_list) {
    console_object_properties(console, nullptr, index_list);
}

void PolicyOUImpl::rename(const QList<QModelIndex> &index_list) {
    console_object_rename(console, nullptr, index_list);
}

void PolicyOUImpl::delete_action(const QList<QModelIndex> &index_list) {
    console_object_delete(console, nullptr, index_list);
}

void policy_ou_impl_add_ou_from_dns(ConsoleWidget *console, AdInterface &ad, const QList<QString> &dn_list, const QModelIndex &parent) {
    const QList<AdObject> object_list = [&]() {
        QList<AdObject> out;

        for (const QString &dn : dn_list) {
            const AdObject object = ad.search_object(dn);
            out.append(object);
        }

        return out;
    }();

    policy_ou_impl_add_objects_to_console(console, object_list, parent);
}

void policy_ou_impl_add_objects_to_console(ConsoleWidget *console, const QList<AdObject> &object_list, const QModelIndex &parent) {
    for (const AdObject &object : object_list) {
        const bool is_ou = object.is_class(CLASS_OU);
        if (!is_ou) {
            continue;
        }

        const QList<QStandardItem *> row = console->add_scope_item(ItemType_PolicyOU, parent);
        console_object_item_data_load(row[0], object);

        const QString item_text = object.get_string(ATTRIBUTE_NAME);
        row[0]->setText(item_text);
    }
}
