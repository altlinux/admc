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

void load_object_row(const QList<QStandardItem *> row, const AdObject &object);
void load_object_item_data(QStandardItem *item, const AdObject &object);
QList<QString> object_model_header_labels();
QList<int> object_model_default_columns();
QList<QString> object_model_search_attributes();
void setup_object_scope_item(QStandardItem *item, const AdObject &object);
void setup_object_results_row(const QList<QStandardItem *> row, const AdObject &object);
void disable_drag_if_object_cant_be_moved(const QList<QStandardItem *> &items, const AdObject &object);
void console_delete_objects(ConsoleWidget *console, const QList<QString> &dn_list, const bool ignore_query_tree = false);
void console_update_object(ConsoleWidget *console, const AdObject &object);
void console_move_objects(ConsoleWidget *console, AdInterface &ad, const QList<QString> &old_dn_list, const QList<QString> &new_dn_list, const QString &new_parent_dn);
void console_move_objects(ConsoleWidget *console, AdInterface &ad, const QList<QString> &old_dn_list, const QString &new_parent_dn);
bool console_add_objects_check(ConsoleWidget *console, const QModelIndex &parent);
void console_add_objects(ConsoleWidget *console, const QList<AdObject> &object_list, const QModelIndex &parent);
void console_add_objects(ConsoleWidget *console, AdInterface &ad, const QList<QString> &dn_list, const QModelIndex &parent);
void fetch_object(ConsoleWidget *console, FilterDialog *filter_dialog, const QModelIndex &index);
QModelIndex init_object_tree(ConsoleWidget *console, AdInterface &ad);

#endif /* OBJECT_H */
