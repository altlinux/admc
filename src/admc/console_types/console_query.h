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

#ifndef CONSOLE_QUERY_H
#define CONSOLE_QUERY_H

#include "central_widget.h"
#include "console_actions.h"
#include "console_widget/console_widget.h"
#include "console_widget/console_impl.h"
#include "console_types/my_console_role.h"

class QStandardItem;
class QModelIndex;
class QStandardItem;
class QString;
class ConsoleWidget;
class ConsoleActions;
template <typename T>
class QList;

enum QueryItemRole {
    QueryItemRole_Description = MyConsoleRole_LAST + 1,
    QueryItemRole_Filter,
    QueryItemRole_FilterState,
    QueryItemRole_Base,
    QueryItemRole_ScopeIsChildren,

    QueryItemRole_LAST,
};

enum QueryColumn {
    QueryColumn_Name,
    QueryColumn_Description,

    QueryColumn_COUNT,
};

QList<QString> console_query_folder_header_labels();
QList<int> console_query_folder_default_columns();
void console_query_folder_load(const QList<QStandardItem *> &row, const QString &name, const QString &description);
QModelIndex console_query_folder_create(ConsoleWidget *console, const QString &name, const QString &description, const QModelIndex &parent);
void console_query_item_load(const QList<QStandardItem *> row, const QString &name, const QString &description, const QString &filter, const QByteArray &filter_state, const QString &base, const bool scope_is_children);
void console_query_item_create(ConsoleWidget *console, const QString &name, const QString &description, const QString &filter, const QByteArray &filter_state, const QString &base, const bool scope_is_children, const QModelIndex &parent);
void console_query_tree_init(ConsoleWidget *console);
void console_query_tree_save(ConsoleWidget *console);
bool console_query_or_folder_name_is_good(ConsoleWidget *console, const QString &name, const QModelIndex &parent_index, QWidget *parent_widget, const QModelIndex &current_index);
void console_query_actions_add_to_menu(ConsoleActions *actions, QMenu *menu);
void console_query_actions_get_state(const QModelIndex &index, const bool single_selection, QSet<ConsoleAction> *visible_actions, QSet<ConsoleAction> *disabled_actions);
QString console_query_folder_path(const QModelIndex &index);
void console_query_move(ConsoleWidget *console, const QList<QPersistentModelIndex> &index_list, const QModelIndex &new_parent_index, const bool delete_old_branch = true);
QStandardItem *console_query_head();
void connect_query_actions(ConsoleWidget *console, ConsoleActions *actions);

class ConsoleQueryItem final : public ConsoleImpl {
    Q_OBJECT

public:
    using ConsoleImpl::ConsoleImpl;

    QSet<StandardAction> get_standard_actions(const QModelIndex &index) const override;

    void fetch(const QModelIndex &index) override;
    void refresh(const QList<QModelIndex> &index_list) override;
    QString get_description(const QModelIndex &index) const override;
};

class ConsoleQueryFolder final : public ConsoleImpl {
    Q_OBJECT

public:
    using ConsoleImpl::ConsoleImpl;

    bool can_drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type) override;
    void drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type) override;
};

#endif /* CONSOLE_QUERY_H */
