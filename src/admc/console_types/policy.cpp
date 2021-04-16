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
#include "console_actions.h"

#include <QStandardItem>
#include <QList>
#include <QCoreApplication>
#include <QMenu>
#include <QDebug>

int policy_container_results_id;
int policy_results_id;

void setup_policy_item_data(QStandardItem *item, const AdObject &object);
void policy_results_load(const QList<QStandardItem *> &row, const AdObject &object);

void policy_scope_load(QStandardItem *item, const AdObject &object) {
    const QString display_name = object.get_string(ATTRIBUTE_DISPLAY_NAME);

    item->setText(display_name);

    setup_policy_item_data(item, object);

    item->setDragEnabled(false);
}

void policy_results_load(const QList<QStandardItem *> &row, const AdObject &object) {
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

void policy_create(ConsoleWidget *console, const QModelIndex &policies_index, const AdObject &object) {
    QStandardItem *scope_item;
    QList<QStandardItem *> results_row;
    console->add_buddy_scope_and_results(policy_results_id, ScopeNodeType_Static, policies_index, &scope_item, &results_row);

    policy_scope_load(scope_item, object);
    policy_results_load(results_row, object);
}

QModelIndex policy_tree_init(ConsoleWidget *console, AdInterface &ad) {
    // Add head item
    QStandardItem *head_item = console->add_scope_item(policy_container_results_id, ScopeNodeType_Static, QModelIndex());
    head_item->setText(QCoreApplication::translate("policy", "Group Policy Objects"));
    head_item->setDragEnabled(false);
    head_item->setIcon(QIcon::fromTheme("folder"));
    head_item->setData(ItemType_PolicyRoot, ConsoleRole_Type);

    // Add children
    const QList<QString> policy_search_attributes = policy_model_search_attributes();
    const QString policy_search_filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_GP_CONTAINER);

    const QHash<QString, AdObject> policy_search_results = ad.search(policy_search_filter, policy_search_attributes, SearchScope_All);

    for (const AdObject &object : policy_search_results.values()) {
        policy_create(console, head_item->index(), object);
    }

    return head_item->index();
}

void policy_add_actions_to_menu(ConsoleActions *actions, QMenu *menu) {
    menu->addAction(actions->get(ObjectAction_PolicyAddLink));
    
    menu->addSeparator();
    
    menu->addAction(actions->get(ObjectAction_PolicyRename));
    menu->addAction(actions->get(ObjectAction_PolicyDelete));
}

void policy_show_hide_actions(ConsoleActions *actions, const QList<QModelIndex> &indexes) {
    const bool single_selection = (indexes.size() == 1);

    if (indexes_are_of_type(indexes, QSet<int>({ItemType_PolicyRoot}))) {
        if (single_selection) {
            actions->show(ObjectAction_PolicyCreate);
        }
    } else if (indexes_are_of_type(indexes, QSet<int>({ItemType_Policy}))) {
        if (single_selection) {
            actions->show(ObjectAction_PolicyAddLink);
            actions->show(ObjectAction_PolicyRename);
            actions->show(ObjectAction_PolicyDelete);
        } else {
            actions->show(ObjectAction_PolicyDelete);
        }
    }
}
