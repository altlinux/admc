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

#include "console_types/policy_root_impl.h"

#include "adldap.h"
#include "central_widget.h"
#include "console_types/object_impl.h"
#include "console_types/console_policy.h"
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

QList<QString> console_policy_header_labels() {
    return {QCoreApplication::translate("policy_model", "Name")};
}

QList<int> console_policy_default_columns() {
    return {0};
}

void PolicyRootImpl::create_policy_in_console(const AdObject &object) {
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

void PolicyRootImpl::fetch(const QModelIndex &index) {
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

PolicyRootImpl::PolicyRootImpl(ConsoleWidget *console_arg)
: ConsoleImpl(console_arg) {
    create_policy_dialog = new CreatePolicyDialog(console);

    create_policy_action = new QAction(tr("Create policy"), this);

    connect(
        create_policy_action, &QAction::triggered,
        create_policy_dialog, &QDialog::open);
    connect(
        create_policy_dialog, &CreatePolicyDialog::created_policy,
        this, &PolicyRootImpl::on_dialog_created_policy);
}

QList<QAction *> PolicyRootImpl::get_all_custom_actions() const {
    QList<QAction *> out;

    out.append(create_policy_action);

    return out;
}

QSet<QAction *> PolicyRootImpl::get_custom_actions(const QModelIndex &index, const bool single_selection) const {
    QSet<QAction *> out;

    out.insert(create_policy_action);

    return out;
}

QSet<StandardAction> PolicyRootImpl::get_standard_actions(const QModelIndex &index, const bool single_selection) const {
    QSet<StandardAction> out;

    out.insert(StandardAction_Refresh);

    return out;
}

void PolicyRootImpl::refresh(const QList<QModelIndex> &index_list) {
    if (index_list.size() != 1) {
        return;
    }

    const QModelIndex index = index_list[0];

    console->delete_children(index);
    fetch(index);
}

// NOTE: not adding policy object to the domain
// tree, but i think it's ok?
void PolicyRootImpl::on_dialog_created_policy(const QString &dn) {
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
