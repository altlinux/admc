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

class QStandardItem;
class AdObject;
class AdInterface;
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

void setup_policy_scope_item(QStandardItem *item, const AdObject &object);
void setup_policy_results_row(const QList<QStandardItem *> &row, const AdObject &object);
void setup_policy_item_data(QStandardItem *item, const AdObject &object);
QList<QString> policy_model_header_labels();
QList<int> policy_model_default_columns();
QList<QString> policy_model_search_attributes();
void console_add_policy(ConsoleWidget *console, const QModelIndex &policies_index, const AdObject &object);
void console_update_policy(ConsoleWidget *console, const QModelIndex &index, const AdObject &object);
QModelIndex init_policy_tree(ConsoleWidget *console, AdInterface &ad);

#endif /* POLICY_H */
