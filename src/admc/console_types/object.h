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

#ifndef OBJECT_H
#define OBJECT_H

#include "console_widget/console_widget.h"
#include "central_widget.h"

class QStandardItem;
class AdObject;
class AdInterface;
class FilterDialog;
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

extern int object_results_id;

void object_scope_load(QStandardItem *item, const AdObject &object);
void object_results_load(const QList<QStandardItem *> row, const AdObject &object);
QList<QString> object_header_labels();
QList<int> object_default_columns();
QList<QString> object_search_attributes();
void object_delete(ConsoleWidget *console, const QList<QString> &dn_list, const bool ignore_query_tree = false);
void object_move(ConsoleWidget *console, AdInterface &ad, const QList<QString> &old_dn_list, const QList<QString> &new_dn_list, const QString &new_parent_dn);
void object_move(ConsoleWidget *console, AdInterface &ad, const QList<QString> &old_dn_list, const QString &new_parent_dn);
void object_create(ConsoleWidget *console, const QList<AdObject> &object_list, const QModelIndex &parent);
void object_create(ConsoleWidget *console, AdInterface &ad, const QList<QString> &dn_list, const QModelIndex &parent);
void object_fetch(ConsoleWidget *console, FilterDialog *filter_dialog, const QModelIndex &index);
QModelIndex object_tree_init(ConsoleWidget *console, AdInterface &ad);
void object_can_drop(const QList<QModelIndex> &dropped_list, const QModelIndex &target, const QSet<ItemType> &dropped_types, bool *ok);
void object_drop(ConsoleWidget *console, const QList<QModelIndex> &dropped_list, const QModelIndex &target);

#endif /* OBJECT_H */
