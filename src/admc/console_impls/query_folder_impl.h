/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#ifndef QUERY_FOLDER_IMPL_H
#define QUERY_FOLDER_IMPL_H

/**
 * Impl for a query folder which stores query items. Located
 * in the query tree.
 */

#include "console_widget/console_impl.h"

class QueryFolderImpl final : public ConsoleImpl {
    Q_OBJECT

public:
    QueryFolderImpl(ConsoleWidget *console_arg);

    bool can_drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type) override;
    void drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type) override;

    QList<QAction *> get_all_custom_actions() const override;
    QSet<QAction *> get_custom_actions(const QModelIndex &index, const bool single_selection) const override;
    QSet<StandardAction> get_standard_actions(const QModelIndex &index, const bool single_selection) const override;

    void delete_action(const QList<QModelIndex> &index_list) override;
    void cut(const QList<QModelIndex> &index_list) override;
    void copy(const QList<QModelIndex> &index_list) override;
    void paste(const QList<QModelIndex> &index_list) override;

    QList<QString> column_labels() const override;
    QList<int> default_columns() const override;

private:
    QAction *new_action;
    QAction *edit_action;
    QAction *import_action;
    bool copied_is_cut;
    QList<QPersistentModelIndex> copied_list;

    void on_import();
    void on_create_query_folder();
    void on_create_query_item();
    void on_edit_query_folder();
};

void console_query_tree_init(ConsoleWidget *console);
void console_query_tree_save(ConsoleWidget *console);
QModelIndex get_query_tree_root(ConsoleWidget *console);
QList<QString> console_query_folder_header_labels();
QList<int> console_query_folder_default_columns();
QString console_query_folder_path(const QModelIndex &index);
QModelIndex console_query_folder_create(ConsoleWidget *console, const QString &name, const QString &description, const QModelIndex &parent);
void console_query_folder_load(const QList<QStandardItem *> &row, const QString &name, const QString &description);
bool console_query_or_folder_name_is_good(const QString &name, const QModelIndex &parent_index, QWidget *parent_widget, const QModelIndex &current_index);
bool console_query_or_folder_name_is_good(const QString &name, const QList<QString> &sibling_names, QWidget *parent_widget);
void query_action_delete(ConsoleWidget *console, const QList<QModelIndex> &index_list);
QList<QString> get_sibling_name_list(const QModelIndex &parent_index, const QModelIndex &index_to_omit);

#endif /* QUERY_FOLDER_IMPL_H */
