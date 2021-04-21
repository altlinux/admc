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

#ifndef CONSOLE_OBJECT_H
#define CONSOLE_OBJECT_H

#include "console_widget/console_widget.h"
#include "central_widget.h"
#include "console_actions.h"

class QStandardItem;
class AdObject;
class AdInterface;
class FilterDialog;
class ConsoleActions;
class QMenu;
class PolicyResultsWidget;
template <typename T> class QList;

/**
 * Some f-ns used for models that store objects.
 */

enum ObjectRole {
    ObjectRole_DN = ConsoleRole_LAST + 1,
    ObjectRole_ObjectClasses = ConsoleRole_LAST + 2,
    ObjectRole_CannotMove = ConsoleRole_LAST + 3,
    ObjectRole_CannotRename = ConsoleRole_LAST + 4,
    ObjectRole_CannotDelete = ConsoleRole_LAST + 5,
    ObjectRole_AccountDisabled = ConsoleRole_LAST + 6,
    
    ObjectRole_LAST = ConsoleRole_LAST + 7,
};

extern int console_object_results_id;

void console_object_scope_load(QStandardItem *item, const AdObject &object);
void console_object_results_load(const QList<QStandardItem *> row, const AdObject &object);
QList<QString> console_object_header_labels();
QList<int> console_object_default_columns();
QList<QString> console_object_search_attributes();
void console_object_delete(ConsoleWidget *console, const QList<QString> &dn_list, const bool ignore_query_tree = false);
void console_object_move(ConsoleWidget *console, AdInterface &ad, const QList<QString> &old_dn_list, const QList<QString> &new_dn_list, const QString &new_parent_dn);
void console_object_move(ConsoleWidget *console, AdInterface &ad, const QList<QString> &old_dn_list, const QString &new_parent_dn);
void console_object_create(ConsoleWidget *console, const QList<AdObject> &object_list, const QModelIndex &parent);
void console_object_create(ConsoleWidget *console, AdInterface &ad, const QList<QString> &dn_list, const QModelIndex &parent);
void console_object_fetch(ConsoleWidget *console, FilterDialog *filter_dialog, const QModelIndex &index);
QModelIndex console_object_tree_init(ConsoleWidget *console, AdInterface &ad);
void console_object_can_drop(const QList<QModelIndex> &dropped_list, const QModelIndex &target, const QSet<ItemType> &dropped_types, bool *ok);
void console_object_drop(ConsoleWidget *console, const QList<QModelIndex> &dropped_list, const QSet<ItemType> &dropped_types, const QModelIndex &target, PolicyResultsWidget *policy_results_widget);
void console_object_actions_add_to_menu(ConsoleActions *actions, QMenu *menu);
void console_object_actions_get_state(const QModelIndex &index, const bool single_selection, QSet<ConsoleAction> *visible_actions, QSet<ConsoleAction> *disabled_actions);

// NOTE: this f-ns don't do anything with console. Just
// convenience code for reuse
QList<QString> object_operation_delete(const QList<QString> &targets, QWidget *parent);
QList<QString> object_operation_set_disabled(const QList<QString> &targets, const bool disabled, QWidget *parent);
void object_operation_add_to_group(const QList<QString> &targets, QWidget *parent);

bool console_object_is_ou(const QModelIndex &index);

#endif /* CONSOLE_OBJECT_H */
