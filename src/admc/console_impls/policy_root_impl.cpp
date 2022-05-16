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

#include "console_impls/policy_root_impl.h"

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

#include <QCoreApplication>
#include <QDebug>
#include <QList>
#include <QMenu>
#include <QProcess>
#include <QStandardItem>

PolicyRootImpl::PolicyRootImpl(ConsoleWidget *console_arg)
: ConsoleImpl(console_arg) {
    set_results_view(new ResultsView(console_arg));
}

void PolicyRootImpl::fetch(const QModelIndex &index) {
    const QList<QStandardItem *> row = console->add_scope_item(ItemType_AllPoliciesFolder, index);
    QStandardItem *item = row[0];
    item->setText(tr("All policies"));
    item->setIcon(QIcon::fromTheme("folder"));
}

void PolicyRootImpl::refresh(const QList<QModelIndex> &index_list) {
    const QModelIndex index = index_list[0];

    console->delete_children(index);
    fetch(index);
}

QSet<StandardAction> PolicyRootImpl::get_standard_actions(const QModelIndex &index, const bool single_selection) const {
    UNUSED_ARG(index);
    UNUSED_ARG(single_selection);

    QSet<StandardAction> out;

    out.insert(StandardAction_Refresh);

    return out;
}

QList<QString> PolicyRootImpl::column_labels() const {
    return {tr("Name")};
}

QList<int> PolicyRootImpl::default_columns() const {
    return {0};
}

void console_policy_tree_init(ConsoleWidget *console) {
    const QList<QStandardItem *> head_row = console->add_scope_item(ItemType_PolicyRoot, QModelIndex());
    auto policy_tree_head = head_row[0];
    policy_tree_head->setText(QCoreApplication::translate("policy_root_impl", "Group Policy Objects"));
    policy_tree_head->setDragEnabled(false);
    policy_tree_head->setIcon(QIcon::fromTheme("folder"));
}
