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

#ifndef CONSOLE_POLICY_H
#define CONSOLE_POLICY_H

#include "central_widget.h"
#include "console_actions.h"
#include "console_widget/console_widget.h"

class QStandardItem;
class AdObject;
class AdInterface;
class ConsoleActions;
class PolicyResultsWidget;
template <typename T>
class QList;

/**
 * Some f-ns used for models that store objects.
 */

enum PolicyRole {
    PolicyRole_DN = ConsoleRole_LAST + 1,

    PolicyRole_LAST = ConsoleRole_LAST + 2,
};

extern int policy_container_results_id;
extern int policy_results_id;

void console_policy_results_load(const QList<QStandardItem *> &row, const AdObject &object);
QList<QString> console_policy_header_labels();
QList<int> console_policy_default_columns();
QList<QString> console_policy_search_attributes();
void console_policy_create(ConsoleWidget *console, const AdObject &object);
void console_policy_tree_init(ConsoleWidget *console, AdInterface &ad);
void console_policy_actions_add_to_menu(ConsoleActions *actions, QMenu *menu);
void console_policy_actions_get_state(const QModelIndex &index, const bool single_selection, QSet<ConsoleAction> *visible_actions, QSet<ConsoleAction> *disabled_actions);
void console_policy_can_drop(const QList<QPersistentModelIndex> &dropped_list, const QPersistentModelIndex &target, const QSet<ItemType> &dropped_types, bool *ok);
void console_policy_drop(ConsoleWidget *console, const QList<QPersistentModelIndex> &dropped_list, const QPersistentModelIndex &target, PolicyResultsWidget *policy_results_widget);
void console_policy_add_link(ConsoleWidget *console, const QList<QString> &policy_list, const QList<QString> &ou_list, PolicyResultsWidget *policy_results_widget);

#endif /* CONSOLE_POLICY_H */
