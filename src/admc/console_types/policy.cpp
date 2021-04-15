/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
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

#include "console_types/policy.h"

#include "adldap.h"
#include "globals.h"
#include "settings.h"
#include "utils.h"
#include "central_widget.h"

#include <QStandardItem>
#include <QList>

int policy_container_results_id;
int policy_results_id;

void setup_policy_scope_item(QStandardItem *item, const AdObject &object) {
    const QString display_name = object.get_string(ATTRIBUTE_DISPLAY_NAME);

    item->setText(display_name);

    setup_policy_item_data(item, object);
}

void setup_policy_results_row(const QList<QStandardItem *> &row, const AdObject &object) {
    const QString display_name = object.get_string(ATTRIBUTE_DISPLAY_NAME);

    row[0]->setText(display_name);

    setup_policy_item_data(row[0], object);

    for (QStandardItem *item : row) {
        item->setDragEnabled(false);
    }
}

void setup_policy_item_data(QStandardItem *item, const AdObject &object) {
    item->setIcon(QIcon::fromTheme("folder-templates"));
    item->setData(ItemType_Policy, ConsoleRole_Type);
    item->setData(object.get_dn(), PolicyRole_DN);
}

QList<QString> policy_model_header_labels() {
    return {QCoreApplication::translate("policy_model", "Name")};
}

QList<int> policy_model_default_columns() {
    return {0};
}

QList<QString> policy_model_search_attributes() {
    return {ATTRIBUTE_DISPLAY_NAME};
}

void console_add_policy(ConsoleWidget *console, const QModelIndex &policies_index, const AdObject &object) {
    QStandardItem *scope_item;
    QList<QStandardItem *> results_row;
    console->add_buddy_scope_and_results(policy_results_id, ScopeNodeType_Static, policies_index, &scope_item, &results_row);

    setup_policy_scope_item(scope_item, object);
    setup_policy_results_row(results_row, object);
}

void console_update_policy(ConsoleWidget *console, const QModelIndex &index, const AdObject &object) {
    auto update_helper =
    [&](const QModelIndex &the_index) {
        const bool is_scope = console->is_scope_item(the_index);

        if (is_scope) {
            QStandardItem *scope_item = console->get_scope_item(the_index);
            setup_policy_scope_item(scope_item, object);
        } else {
            QList<QStandardItem *> results_row = console->get_results_row(the_index);
            setup_policy_results_row(results_row, object);
        }
    };

    update_helper(index);

    const QModelIndex buddy = console->get_buddy(index);
    if (buddy.isValid()) {
        update_helper(buddy);
    }
}
