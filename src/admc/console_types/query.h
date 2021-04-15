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
template <typename T> class QList;

enum QueryItemRole {
    QueryItemRole_Description = ConsoleRole_LAST + 1,
    QueryItemRole_Filter = ConsoleRole_LAST + 2,
    QueryItemRole_SearchBase = ConsoleRole_LAST + 3,

    QueryItemRole_LAST = ConsoleRole_LAST + 7,
};

/**
 * Some f-ns used for models that store queries.
 */

QList<QString> query_folder_header_labels();
QList<int> query_folder_default_columns();
void load_query_folder(QStandardItem *item, const QList<QStandardItem *> &results_row, const QString &name, const QString &description);
QString query_folder_path(const QModelIndex &index);
QString path_to_name(const QString &path);
// Returns index of the scope item
QModelIndex add_query_folder(ConsoleWidget *console, const int query_folder_results_id, const QString &name, const QString &description, const QModelIndex &parent);
void add_query_item(ConsoleWidget *console, const int object_results_id, const QString &name, const QString &description, const QString &filter, const QString &search_base, const QModelIndex &parent);
QList<QString> get_sibling_names(const QModelIndex &parent);

#endif /* QUERY_H */
