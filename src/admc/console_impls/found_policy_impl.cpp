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

#include "console_impls/found_policy_impl.h"

#include "adldap.h"
#include "console_impls/find_policy_impl.h"
#include "console_impls/item_type.h"
#include "console_impls/policy_impl.h"
#include "utils.h"
#include "globals.h"
#include "managers/icon_manager.h"

#include <QAction>
#include <QStandardItem>

FoundPolicyImpl::FoundPolicyImpl(ConsoleWidget *console_arg)
: ConsoleImpl(console_arg) {
    console_list = {
        console,
    };

    add_link_action = new QAction(tr("Add link..."), this);
    edit_action = new QAction(tr("Edit..."), this);

    connect(
        add_link_action, &QAction::triggered,
        this, &FoundPolicyImpl::on_add_link);
    connect(
        edit_action, &QAction::triggered,
        this, &FoundPolicyImpl::on_edit);
}

void FoundPolicyImpl::set_buddy_console(ConsoleWidget *buddy_console) {
    console_list = {
        console,
        buddy_console,
    };
}

QList<QAction *> FoundPolicyImpl::get_all_custom_actions() const {
    return {
        add_link_action,
        edit_action,
    };
}

QSet<QAction *> FoundPolicyImpl::get_custom_actions(const QModelIndex &index, const bool single_selection) const {
    UNUSED_ARG(index);

    QSet<QAction *> out;

    if (single_selection) {
        out.insert(add_link_action);
        out.insert(edit_action);
    }

    return out;
}

QSet<StandardAction> FoundPolicyImpl::get_standard_actions(const QModelIndex &index, const bool single_selection) const {
    UNUSED_ARG(index);

    QSet<StandardAction> out;

    out.insert(StandardAction_Delete);

    if (single_selection) {
        out.insert(StandardAction_Rename);
        out.insert(StandardAction_Properties);
    }

    return out;
}

void FoundPolicyImpl::rename(const QList<QModelIndex> &index_list) {
    UNUSED_ARG(index_list);

    PolicyResultsWidget *policy_results = nullptr;
    console_policy_rename(console_list, policy_results, ItemType_FoundPolicy, FoundPolicyRole_DN);
}

void FoundPolicyImpl::delete_action(const QList<QModelIndex> &index_list) {
    UNUSED_ARG(index_list);

    PolicyResultsWidget *policy_results = nullptr;
    console_policy_delete(console_list, policy_results, ItemType_FoundPolicy, FoundPolicyRole_DN);
}

void FoundPolicyImpl::properties(const QList<QModelIndex> &index_list) {
    UNUSED_ARG(index_list);

    PolicyResultsWidget *policy_results = nullptr;
    console_policy_properties(console_list, policy_results, ItemType_FoundPolicy, FoundPolicyRole_DN);
}

void FoundPolicyImpl::on_add_link() {
    PolicyResultsWidget *policy_results = nullptr;

    console_policy_add_link(console_list, policy_results, ItemType_FoundPolicy, FoundPolicyRole_DN);
}

void FoundPolicyImpl::on_edit() {
    console_policy_edit(console, ItemType_FoundPolicy, FoundPolicyRole_DN);
}

void found_policy_impl_load(const QList<QStandardItem *> &row, const AdObject &object) {
    QStandardItem *main_item = row[0];

    const QIcon icon = g_icon_manager->get_object_icon(object);
    main_item->setIcon(icon);
    main_item->setData(object.get_dn(), FoundPolicyRole_DN);

    const QString display_name = object.get_string(ATTRIBUTE_DISPLAY_NAME);
    row[FindPolicyColumn_Name]->setText(display_name);

    const QString cn = object.get_string(ATTRIBUTE_CN);
    row[FindPolicyColumn_GUID]->setText(cn);
}
