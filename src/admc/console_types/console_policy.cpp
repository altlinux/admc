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
#include "console_actions.h"
#include "console_types/console_object.h"
#include "globals.h"
#include "gplink.h"
#include "policy_results_widget.h"
#include "settings.h"
#include "status.h"
#include "utils.h"

#include <QCoreApplication>
#include <QDebug>
#include <QList>
#include <QMenu>
#include <QStandardItem>

int policy_container_results_id;
int policy_results_id;

void setup_policy_item_data(QStandardItem *item, const AdObject &object);
void console_policy_results_load(const QList<QStandardItem *> &row, const AdObject &object);

void console_policy_scope_load(QStandardItem *item, const AdObject &object) {
    const QString display_name = object.get_string(ATTRIBUTE_DISPLAY_NAME);

    item->setText(display_name);

    setup_policy_item_data(item, object);
}

void console_policy_results_load(const QList<QStandardItem *> &row, const AdObject &object) {
    const QString display_name = object.get_string(ATTRIBUTE_DISPLAY_NAME);

    row[0]->setText(display_name);

    setup_policy_item_data(row[0], object);
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
    const QModelIndex policy_root_index = [&]() {
        const QList<QModelIndex> results = console->search_scope_by_role(ConsoleRole_Type, ItemType_PolicyRoot);

        if (!results.isEmpty()) {
            return results[0];
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
    const QString base = g_adconfig->domain_head();
    const SearchScope scope = SearchScope_All;
    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_GP_CONTAINER);
    const QList<QString> attributes = console_policy_search_attributes();
    const QHash<QString, AdObject> results = ad.search(base, scope, filter, attributes);

    for (const AdObject &object : results.values()) {
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

void console_policy_can_drop(const QList<QPersistentModelIndex> &dropped_list, const QPersistentModelIndex &target, const QSet<ItemType> &dropped_types, bool *ok) {
    const bool dropped_are_objects = (dropped_types == QSet<ItemType>({ItemType_Object}));
    if (!dropped_are_objects) {
        return;
    }

    const bool dropped_contain_ou = [&]() {
        for (const QPersistentModelIndex &index : dropped_list) {
            if (console_object_is_ou(index)) {
                return true;
            }
        }

        return false;
    }();

    if (!dropped_contain_ou) {
        return;
    }

    *ok = true;
}

void console_policy_drop(ConsoleWidget *console, const QList<QPersistentModelIndex> &dropped_list, const QPersistentModelIndex &target, PolicyResultsWidget *policy_results_widget) {
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
