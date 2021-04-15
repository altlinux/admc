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

#include "console_types/query.h"

#include "console_types/object.h"
#include "central_widget.h"
#include "utils.h"
#include "globals.h"
#include "settings.h"

#include <QCoreApplication>
#include <QStandardItem>
#include <QStack>
#include <QDebug>

#define QUERY_ROOT "QUERY_ROOT"

enum QueryColumn {
    QueryColumn_Name,
    QueryColumn_Description,

    QueryColumn_COUNT,
};

int query_folder_results_id;

QList<QString> query_folder_header_labels() {
    return {
        QCoreApplication::translate("query_folder.cpp", "Name"),
        QCoreApplication::translate("query_folder.cpp", "Description"),
    };
}

QList<int> query_folder_default_columns() {
    return {QueryColumn_Name, QueryColumn_Description};
}

// NOTE: use constant QUERY_ROOT for root because query root
// item has different name depending on app language. The
// end result is that path for root is equal to
// "QUERY_ROOT", while all other paths start with
// "QUERY_ROOT/a/b/c..."
QString query_folder_path(const QModelIndex &index) {
    const bool is_query_root = !index.parent().isValid();
    if (is_query_root) {
        return QString(QUERY_ROOT);
    }

    const QList<QString> path_split =
    [&index]() {
        QList<QString> out;

        QModelIndex current = index;
        while (current.isValid()) {
            const QString name = current.data(Qt::DisplayRole).toString();
            out.prepend(name);

            current = current.parent();
        }

        // NOTE: remove root
        out.removeAt(0);

        return out;
    }();

    const QString path =
    [&path_split]() {
        QString out;

        for (int i = 0; i < path_split.size(); i++) {
            const QString part = path_split[i];

            if (i == 0) {
                out += QString(QUERY_ROOT) + "/";
            } else {
                out += "/";
            }

            out += part;
        }

        return out;
    }();

    return path;
}

QString path_to_name(const QString &path) {
    const int separator_i = path.lastIndexOf("/");
    const QString name = path.mid(separator_i + 1);

    return name;
}

QModelIndex add_query_folder(ConsoleWidget *console, const QString &name, const QString &description, const QModelIndex &parent) {
    auto load_main_item =
    [&](QStandardItem *item) {
        item->setData(description, QueryItemRole_Description);
        item->setData(ItemType_QueryFolder, ConsoleRole_Type);
        item->setIcon(QIcon::fromTheme("folder"));
    };

    QStandardItem *scope_item;
    QList<QStandardItem *> results_row;
    console->add_buddy_scope_and_results(query_folder_results_id, ScopeNodeType_Static, parent, &scope_item, &results_row);

    load_main_item(scope_item);
    scope_item->setText(name);

    load_main_item(results_row[0]);
    results_row[QueryColumn_Name]->setText(name);
    results_row[QueryColumn_Description]->setText(description);

    return scope_item->index();
}

void add_query_item(ConsoleWidget *console, const QString &name, const QString &description, const QString &filter, const QString &search_base, const QModelIndex &parent) {
    auto load_main_item =
    [&](QStandardItem *item) {
        item->setData(ItemType_QueryItem, ConsoleRole_Type);
        item->setData(description, QueryItemRole_Description);
        item->setData(filter, QueryItemRole_Filter);
        item->setData(search_base, QueryItemRole_SearchBase);
        item->setIcon(QIcon::fromTheme("emblem-system"));
    };

    QStandardItem *scope_item;
    QList<QStandardItem *> results_row;
    console->add_buddy_scope_and_results(object_results_id, ScopeNodeType_Dynamic, parent, &scope_item, &results_row);

    load_main_item(scope_item);
    scope_item->setText(name);

    load_main_item(results_row[0]);
    results_row[QueryColumn_Name]->setText(name);
    results_row[QueryColumn_Description]->setText(description);
}

QList<QString> get_sibling_names(const QModelIndex &parent) {
    QList<QString> out;

    QAbstractItemModel *model = (QAbstractItemModel *) parent.model();

    for (int row = 0; row < model->rowCount(parent); row++) {
        const QModelIndex sibling = model->index(row, 0, parent);
        const QString name = sibling.data(Qt::DisplayRole).toString();
        out.append(name);
    }

    return out;
}
