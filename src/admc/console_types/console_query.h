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

#ifndef CONSOLE_QUERY_H
#define CONSOLE_QUERY_H

#include "console_widget/console_widget.h"
#include "console_actions.h"
#include "central_widget.h"

class QStandardItem;
class QModelIndex;
class QStandardItem;
class QString;
class ConsoleWidget;
class ConsoleActions;
template <typename T> class QList;

enum QueryItemRole {
    QueryItemRole_Description = ConsoleRole_LAST + 1,
    QueryItemRole_Filter = ConsoleRole_LAST + 2,
    QueryItemRole_FilterState = ConsoleRole_LAST + 3,
    QueryItemRole_SearchBase = ConsoleRole_LAST + 4,

    QueryItemRole_LAST = ConsoleRole_LAST + 7,
};

extern int console_query_folder_results_id;

QList<QString> console_query_folder_header_labels();
QList<int> console_query_folder_default_columns();
void console_query_folder_load(QStandardItem *scope_item, const QList<QStandardItem *> &results_row, const QString &name, const QString &description);
// Returns index of the scope item
QModelIndex console_query_folder_create(ConsoleWidget *console, const QString &name, const QString &description, const QModelIndex &parent);
void console_query_item_load(QStandardItem *scope_item, const QList<QStandardItem *> results_row, const QString &name, const QString &description, const QString &filter, const QByteArray &filter_state, const QString &search_base);
void console_query_item_create(ConsoleWidget *console, const QString &name, const QString &description, const QString &filter, const QByteArray &filter_state, const QString &search_base, const QModelIndex &parent);
void console_query_item_fetch(ConsoleWidget *console, const QModelIndex &index);
void console_query_tree_init(ConsoleWidget *console);
void console_query_tree_save(ConsoleWidget *console);
bool console_query_or_folder_name_is_good(const QString &name, const QModelIndex &parent_index, QWidget *parent_widget, const QModelIndex &current_index);
void console_query_actions_add_to_menu(ConsoleActions *actions, QMenu *menu);
void console_query_actions_get_state(const QModelIndex &index, const bool single_selection, QSet<ConsoleAction> *visible_actions, QSet<ConsoleAction> *disabled_actions);
QString console_query_folder_path(const QModelIndex &index);
QModelIndex console_query_get_root_index(ConsoleWidget *console);
void console_query_can_drop(const QList<QPersistentModelIndex> &dropped_list, const QPersistentModelIndex &target, const QSet<ItemType> &dropped_types, bool *ok);
void console_query_drop(ConsoleWidget *console, const QList<QPersistentModelIndex> &dropped_list, const QPersistentModelIndex &target);
void console_query_move(ConsoleWidget *console, const QList<QPersistentModelIndex> &index_list, const QModelIndex &new_parent_index);
void console_query_export(ConsoleWidget *console);
void console_query_import(ConsoleWidget *console);

#endif /* CONSOLE_QUERY_H */
