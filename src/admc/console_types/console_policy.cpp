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

#include "console_types/console_policy.h"

#include "adldap.h"
#include "globals.h"
#include "settings.h"
#include "utils.h"
#include "central_widget.h"
#include "console_actions.h"

#include <QStandardItem>
#include <QList>
#include <QCoreApplication>
#include <QMenu>
#include <QDebug>

int policy_container_results_id;
int policy_results_id;

void setup_policy_item_data(QStandardItem *item, const AdObject &object);
void console_policy_results_load(const QList<QStandardItem *> &row, const AdObject &object);

void console_policy_scope_load(QStandardItem *item, const AdObject &object) {
    const QString display_name = object.get_string(ATTRIBUTE_DISPLAY_NAME);

    item->setText(display_name);

    setup_policy_item_data(item, object);

    item->setDragEnabled(false);
}

void console_policy_results_load(const QList<QStandardItem *> &row, const AdObject &object) {
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

QList<QString> console_policy_header_labels() {
    return {QCoreApplication::translate("policy_model", "Name")};
}

QList<int> console_policy_default_columns() {
    return {0};
}

QList<QString> console_policy_search_attributes() {
    return {ATTRIBUTE_DISPLAY_NAME};
}

void console_policy_create(ConsoleWidget *console, const AdObject &object) {
    const QModelIndex policy_root_index =
    [&]() {
        const QList<QModelIndex> search_results = console->search_scope_by_role(ConsoleRole_Type, ItemType_PolicyRoot);

        if (!search_results.isEmpty()) {
            return search_results[0];
        } else {
            return QModelIndex();
        }
    }();

    if (!policy_root_index.isValid()) {
        qDebug() << "Failed to find policy root";
        
        return;
    }

    QStandardItem *scope_item;
    QList<QStandardItem *> results_row;
    console->add_buddy_scope_and_results(policy_results_id, ScopeNodeType_Static, policy_root_index, &scope_item, &results_row);

    console_policy_scope_load(scope_item, object);
    console_policy_results_load(results_row, object);
}

void console_policy_tree_init(ConsoleWidget *console, AdInterface &ad) {
    // Add head item
    QStandardItem *head_item = console->add_scope_item(policy_container_results_id, ScopeNodeType_Static, QModelIndex());
    head_item->setText(QCoreApplication::translate("policy", "Group Policy Objects"));
    head_item->setDragEnabled(false);
    head_item->setIcon(QIcon::fromTheme("folder"));
    head_item->setData(ItemType_PolicyRoot, ConsoleRole_Type);

    // Add children
    const QList<QString> policy_search_attributes = console_policy_search_attributes();
    const QString policy_search_filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_GP_CONTAINER);

    const QHash<QString, AdObject> policy_search_results = ad.search(policy_search_filter, policy_search_attributes, SearchScope_All);

    for (const AdObject &object : policy_search_results.values()) {
        console_policy_create(console, object);
    }
}

void console_policy_actions_add_to_menu(ConsoleActions *actions, QMenu *menu) {
    menu->addAction(actions->get(ConsoleAction_PolicyAddLink));
    
    menu->addSeparator();
    
    menu->addAction(actions->get(ConsoleAction_PolicyRename));
    menu->addAction(actions->get(ConsoleAction_PolicyDelete));
}

void console_policy_actions_get_state(const QModelIndex &index, const bool single_selection, QSet<ConsoleAction> *visible_actions, QSet<ConsoleAction> *disabled_actions) {
    const ItemType type = (ItemType) index.data(ConsoleRole_Type).toInt();

    if (type == ItemType_PolicyRoot) {
        visible_actions->insert(ConsoleAction_PolicyCreate);
    }

    if (type == ItemType_Policy) {
        if (single_selection) {
            visible_actions->insert(ConsoleAction_PolicyAddLink);
            visible_actions->insert(ConsoleAction_PolicyRename);
            visible_actions->insert(ConsoleAction_PolicyDelete);
        } else {
            visible_actions->insert(ConsoleAction_PolicyDelete);
        }
    }
}
