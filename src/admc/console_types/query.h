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

#ifndef QUERY_H
#define QUERY_H

#include "console_widget/console_widget.h"

class QStandardItem;
class QModelIndex;
class QStandardItem;
class QString;
class ConsoleWidget;
class ObjectActions;
template <typename T> class QList;

enum QueryItemRole {
    QueryItemRole_Description = ConsoleRole_LAST + 1,
    QueryItemRole_Filter = ConsoleRole_LAST + 2,
    QueryItemRole_SearchBase = ConsoleRole_LAST + 3,

    QueryItemRole_LAST = ConsoleRole_LAST + 7,
};

extern int query_folder_results_id;

QList<QString> query_folder_header_labels();
QList<int> query_folder_default_columns();
void query_folder_load(QStandardItem *scope_item, const QList<QStandardItem *> &results_row, const QString &name, const QString &description);
// Returns index of the scope item
QModelIndex query_folder_create(ConsoleWidget *console, const QString &name, const QString &description, const QModelIndex &parent);
void query_item_create(ConsoleWidget *console, const QString &name, const QString &description, const QString &filter, const QString &search_base, const QModelIndex &parent);
void query_item_fetch(ConsoleWidget *console, const QModelIndex &index);
void query_tree_init(ConsoleWidget *console);
void query_tree_save();
bool query_name_is_good(const QString &name, const QModelIndex &parent_index, QWidget *parent_widget, const QModelIndex &current_index);
void query_add_actions_to_menu(ObjectActions *actions, QMenu *menu);
void query_show_hide_actions(ObjectActions *actions, const QList<QModelIndex> &indexes);

#endif /* QUERY_H */
