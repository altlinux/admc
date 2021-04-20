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

#ifndef POLICY_H
#define POLICY_H

#include "console_widget/console_widget.h"
#include "console_actions.h"

class QStandardItem;
class AdObject;
class AdInterface;
class ConsoleActions;
template <typename T> class QList;

/**
 * Some f-ns used for models that store objects.
 */

enum PolicyRole {
    PolicyRole_DN = ConsoleRole_LAST + 1,
    
    PolicyRole_LAST = ConsoleRole_LAST + 2,
};

extern int policy_container_results_id;
extern int policy_results_id;

void policy_scope_load(QStandardItem *item, const AdObject &object);
void policy_results_load(const QList<QStandardItem *> &row, const AdObject &object);
QList<QString> policy_model_header_labels();
QList<int> policy_model_default_columns();
QList<QString> policy_model_search_attributes();
void policy_create(ConsoleWidget *console, const AdObject &object);
void policy_tree_init(ConsoleWidget *console, AdInterface &ad);
void policy_add_actions_to_menu(ConsoleActions *actions, QMenu *menu);
void policy_show_hide_actions(ConsoleActions *actions, const QList<QModelIndex> &indexes);
void policy_get_action_state(const QModelIndex &index, const bool single_selection, QSet<ConsoleAction> *visible_actions, QSet<ConsoleAction> *disabled_actions);

#endif /* POLICY_H */
