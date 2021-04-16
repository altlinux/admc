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
#include "adldap.h"
#include "object_actions.h"

#include <QCoreApplication>
#include <QStandardItem>
#include <QStack>
#include <QDebug>
#include <QMessageBox>
#include <QMenu>

#define QUERY_ROOT "QUERY_ROOT"

enum QueryColumn {
    QueryColumn_Name,
    QueryColumn_Description,

    QueryColumn_COUNT,
};

int query_folder_results_id;
QPersistentModelIndex query_tree_head;

QString query_folder_path(const QModelIndex &index);

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

void query_folder_load(QStandardItem *scope_item, const QList<QStandardItem *> &results_row, const QString &name, const QString &description) {
    auto load_main_item =
    [&](QStandardItem *item) {
        item->setData(description, QueryItemRole_Description);
        item->setData(ItemType_QueryFolder, ConsoleRole_Type);
        item->setIcon(QIcon::fromTheme("folder"));
    };

    load_main_item(scope_item);
    scope_item->setText(name);
    scope_item->setDragEnabled(false);

    load_main_item(results_row[0]);
    results_row[QueryColumn_Name]->setText(name);
    results_row[QueryColumn_Description]->setText(description);
    for (QStandardItem *item : results_row) {
        item->setDragEnabled(false);
    }
}

QModelIndex query_folder_create(ConsoleWidget *console, const QString &name, const QString &description, const QModelIndex &parent) {
    QStandardItem *scope_item;
    QList<QStandardItem *> results_row;
    console->add_buddy_scope_and_results(query_folder_results_id, ScopeNodeType_Static, parent, &scope_item, &results_row);
    query_folder_load(scope_item, results_row, name, description);

    return scope_item->index();
}

void query_item_create(ConsoleWidget *console, const QString &name, const QString &description, const QString &filter, const QString &search_base, const QModelIndex &parent) {
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
    scope_item->setDragEnabled(false);

    load_main_item(results_row[0]);
    results_row[QueryColumn_Name]->setText(name);
    results_row[QueryColumn_Description]->setText(description);
    for (QStandardItem *item : results_row) {
        item->setDragEnabled(false);
    }
}

void query_item_fetch(ConsoleWidget *console, const QModelIndex &index) {
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    show_busy_indicator();

    const QString filter = index.data(QueryItemRole_Filter).toString();
    const QString search_base = index.data(QueryItemRole_SearchBase).toString();
    const QList<QString> search_attributes = object_search_attributes();

    const QHash<QString, AdObject> search_results = ad.search(filter, search_attributes, SearchScope_All, search_base);
    for (const AdObject &object : search_results.values()) {
        const QList<QStandardItem *> results_row = console->add_results_row(index);
        object_results_load(results_row, object);
    }

    hide_busy_indicator();
}

void query_tree_init(ConsoleWidget *console) {
    auto path_to_name =
    [](const QString &path) {
        const int separator_i = path.lastIndexOf("/");
        const QString name = path.mid(separator_i + 1);

        return name;
    };

    // Add head item
    QStandardItem *head_item = console->add_scope_item(query_folder_results_id, ScopeNodeType_Static, QModelIndex());
    head_item->setText(QCoreApplication::translate("query", "Saved Queries"));
    head_item->setIcon(QIcon::fromTheme("folder"));
    head_item->setData(ItemType_QueryRoot, ConsoleRole_Type);

    // Add rest of tree
    const QHash<QString, QVariant> folders_map = g_settings->get_variant(VariantSetting_QueryFolders).toHash();
    const QHash<QString, QVariant> info_map = g_settings->get_variant(VariantSetting_QueryInfo).toHash();

    QStack<QModelIndex> query_stack;
    query_stack.append(head_item->index());
    while (!query_stack.isEmpty()) {
        const QModelIndex index = query_stack.pop();
        const QString path = query_folder_path(index);

        // Go through children and add them as folders or
        // query items
        const QList<QString> children = folders_map[path].toStringList();
        for (const QString &child_path : children) {
            const QHash<QString, QVariant> info = info_map[child_path].toHash();
            const bool is_query_item = info["is_query_item"].toBool();

            if (is_query_item) {
                // Query item
                const QString description = info["description"].toString();
                const QString filter = info["filter"].toString();
                const QString search_base = info["search_base"].toString();
                const QString name = path_to_name(child_path);
                query_item_create(console, name, description, filter, search_base, index);
            } else {
                // Query folder
                const QString name = path_to_name(child_path);
                const QString description = info["description"].toString();
                const QModelIndex child_scope_index = query_folder_create(console, name, description, index);

                query_stack.append(child_scope_index);
            }
        }
    }

    query_tree_head = head_item->index();
}

// Saves current state of queries tree to settings. Should
// be called after every modication to queries tree
void query_tree_save() {
    // folder path = {list of children}
    // data = {path => data map containing info about
    // query folder/item}
    QHash<QString, QVariant> folders;
    QHash<QString, QVariant> info_map;

    QStack<QModelIndex> stack;
    stack.append(query_tree_head);

    const QAbstractItemModel *model = query_tree_head.model();

    while (!stack.isEmpty()) {
        const QModelIndex index = stack.pop();

        // Add children to stack
        for (int i = 0; i < model->rowCount(index); i++) {
            const QModelIndex child = model->index(i, 0, index);

            stack.append(child);
        }

        // NOTE: don't save head item, it's created manually
        const bool is_query_root = !index.parent().isValid();
        if (is_query_root) {
            continue;
        }

        const QString path = query_folder_path(index);
        const QString parent_path = query_folder_path(index.parent());
        const ItemType type = (ItemType) index.data(ConsoleRole_Type).toInt();

        QList<QString> child_folders = folders[parent_path].toStringList();
        child_folders.append(path);
        folders[parent_path] = QVariant(child_folders);

        const QString description = index.data(QueryItemRole_Description).toString();

        if (type == ItemType_QueryFolder) {
            QHash<QString, QVariant> info;
            info["is_query_item"] = QVariant(false);
            info["description"] = QVariant(description);
            info_map[path] = QVariant(info);
        } else {
            const QString filter = index.data(QueryItemRole_Filter).toString();
            const QString search_base = index.data(QueryItemRole_SearchBase).toString();

            QHash<QString, QVariant> info;
            info["is_query_item"] = QVariant(true);
            info["description"] = QVariant(description);
            info["filter"] = QVariant(filter);
            info["search_base"] = QVariant(search_base);
            info_map[path] = QVariant(info);
        }
    }

    const QVariant folders_variant = QVariant(folders);
    const QVariant info_map_variant = QVariant(info_map);

    g_settings->set_variant(VariantSetting_QueryFolders, folders_variant);
    g_settings->set_variant(VariantSetting_QueryInfo, info_map_variant);
}

bool query_name_is_good(const QString &name, const QModelIndex &parent_index, QWidget *parent_widget, const QModelIndex &current_index) {
    const QString current_name = current_index.data(Qt::DisplayRole).toString();

    const QList<QString> sibling_names =
    [&]() {
        QList<QString> out;

        QAbstractItemModel *model = (QAbstractItemModel *) parent_index.model();

        for (int row = 0; row < model->rowCount(parent_index); row++) {
            const QModelIndex sibling = model->index(row, 0, parent_index);
            const QString sibling_name = sibling.data(Qt::DisplayRole).toString();

            const bool this_is_query_itself = (sibling_name == current_name);
            if (this_is_query_itself) {
                continue;
            }
            
            out.append(sibling_name);
        }

        return out;
    }();

    const bool name_conflict = sibling_names.contains(name);
    const bool name_contains_slash = name.contains("/");

    if (name_conflict) {
        const QString error_text = QString(QCoreApplication::translate("query.cpp", "There's already a folder with this name."));
        QMessageBox::warning(parent_widget, QCoreApplication::translate("query.cpp", "Error"), error_text);
    } else if (name_contains_slash) {
        const QString error_text = QString(QCoreApplication::translate("query.cpp", "Folder names cannot contain \"/\"."));
        QMessageBox::warning(parent_widget, QCoreApplication::translate("query.cpp", "Error"), error_text);
    }

    const bool name_is_good = (!name_conflict && !name_contains_slash);

    return name_is_good;
}

void query_add_actions_to_menu(ObjectActions *actions, QMenu *menu) {
    menu->addAction(actions->get(ObjectAction_QueryEditFolder));
    menu->addAction(actions->get(ObjectAction_QueryDeleteItemOrFolder));
}

void query_show_hide_actions(ObjectActions *actions, const QList<QModelIndex> &indexes) {
    const bool single_selection = (indexes.size() == 1);

    if (indexes_are_of_type(indexes, QSet<int>({ItemType_QueryRoot}))) {
        actions->show(ObjectAction_QueryCreateFolder);
        actions->show(ObjectAction_QueryCreateItem);
    } else if (indexes_are_of_type(indexes, QSet<int>({ItemType_QueryFolder}))) {
        if (single_selection) {
            actions->show(ObjectAction_QueryCreateFolder);
            actions->show(ObjectAction_QueryCreateItem);
            actions->show(ObjectAction_QueryEditFolder);
        }
        actions->show(ObjectAction_QueryDeleteItemOrFolder);
    } else if (indexes_are_of_type(indexes, QSet<int>({ItemType_QueryItem}))) {
        actions->show(ObjectAction_QueryDeleteItemOrFolder);
    } else if (indexes_are_of_type(indexes, QSet<int>({ItemType_QueryItem, ItemType_QueryFolder}))) {
        actions->show(ObjectAction_QueryDeleteItemOrFolder);
    }
}
