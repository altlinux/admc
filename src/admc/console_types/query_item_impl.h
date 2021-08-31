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

#include "console_widget/console_impl.h"
#include "console_types/my_console_role.h"

enum QueryItemRole {
    QueryItemRole_Description = MyConsoleRole_LAST + 1,
    QueryItemRole_Filter,
    QueryItemRole_FilterState,
    QueryItemRole_Base,
    QueryItemRole_ScopeIsChildren,
    QueryItemRole_IsRoot,

    QueryItemRole_LAST,
};

enum QueryColumn {
    QueryColumn_Name,
    QueryColumn_Description,

    QueryColumn_COUNT,
};

void console_query_item_load(const QList<QStandardItem *> row, const QString &name, const QString &description, const QString &filter, const QByteArray &filter_state, const QString &base, const bool scope_is_children);
void console_query_item_create(ConsoleWidget *console, const QString &name, const QString &description, const QString &filter, const QByteArray &filter_state, const QString &base, const bool scope_is_children, const QModelIndex &parent);
void console_query_item_load(ConsoleWidget *console, const QHash<QString, QVariant> &data, const QModelIndex &parent_index);
QHash<QString, QVariant> console_query_item_save(const QModelIndex &index);

class QueryItemImplItem final : public ConsoleImpl {
    Q_OBJECT

public:
    QueryItemImplItem(ConsoleWidget *console_arg);

    void fetch(const QModelIndex &index) override;
    QString get_description(const QModelIndex &index) const override;

    QList<QAction *> get_all_custom_actions() const override;
    QSet<QAction *> get_custom_actions(const QModelIndex &index, const bool single_selection) const override;
    QSet<StandardAction> get_standard_actions(const QModelIndex &index, const bool single_selection) const override;

    void refresh(const QList<QModelIndex> &index_list) override;
    void delete_action(const QList<QModelIndex> &index_list) override;
    void cut(const QList<QModelIndex> &index_list) override;
    void copy(const QList<QModelIndex> &index_list) override;

private:
    QAction *edit_action;
    QAction *export_action;

    void on_edit();
    void on_export();

};

#endif /* CONSOLE_QUERY_H */
