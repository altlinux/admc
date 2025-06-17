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

#include "console_impls/policy_root_impl.h"

#include "adldap.h"
#include "console_impls/all_policies_folder_impl.h"
#include "console_impls/item_type.h"
#include "console_impls/policy_ou_impl.h"
#include "console_widget/results_view.h"
#include "globals.h"
#include "gplink.h"
#include "status.h"
#include "utils.h"
#include "managers/icon_manager.h"
#include "managers/gplink_manager.h"

#include <QList>
#include <QStandardItem>

PolicyRootImpl::PolicyRootImpl(ConsoleWidget *console_arg)
: ConsoleImpl(console_arg) {
    set_results_view(new ResultsView(console_arg));
}

void PolicyRootImpl::fetch(const QModelIndex &index) {
    AdInterface ad;
    if (ad_failed(ad, console)) {
        return;
    }

    // Add domain object
    const QList<QStandardItem *> domain_row = console->add_scope_item(ItemType_PolicyOU, index);
    QStandardItem *domain_item = domain_row[0];
    const QString domain_dn = g_adconfig->domain_dn();
    const AdObject domain_object = ad.search_object(domain_dn);

    policy_ou_impl_load_item_data(domain_item, domain_object);

    const QString domain_name = g_adconfig->domain().toLower();
    domain_item->setText(domain_name);
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
    const QList<QStandardItem *> head_row = console->add_scope_item(ItemType_PolicyRoot, console->domain_info_index());
    auto policy_tree_head = head_row[0];
    policy_tree_head->setText(QCoreApplication::translate("policy_root_impl", "Group Policy Objects"));
    policy_tree_head->setDragEnabled(false);
    policy_tree_head->setIcon(g_icon_manager->category_icon(ADMC_CATEGORY_GP_OBJECTS));
    console->set_item_sort_index(head_row[0]->index(), 1);
}

QModelIndex get_policy_tree_root(ConsoleWidget *console) {
    const QModelIndex out = console->search_item(console->domain_info_index(), {ItemType_PolicyRoot});

    return out;
}
