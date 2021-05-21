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

#include "console_types/console_query.h"

#include "console_types/console_object.h"
#include "central_widget.h"
#include "utils.h"
#include "globals.h"
#include "settings.h"
#include "adldap.h"
#include "console_actions.h"

#include <QCoreApplication>
#include <QStandardItem>
#include <QStack>
#include <QDebug>
#include <QMessageBox>
#include <QMenu>
#include <QFileDialog>
#include <QStandardPaths>
#include <QJsonDocument>

#define QUERY_ROOT "QUERY_ROOT"

enum QueryColumn {
    QueryColumn_Name,
    QueryColumn_Description,

    QueryColumn_COUNT,
};

int console_query_folder_results_id;

QHash<QString, QVariant> console_query_item_save(const QModelIndex &index);
void console_query_item_load(ConsoleWidget *console, const QHash<QString, QVariant> &data, const QModelIndex &parent_index);

QList<QString> console_query_folder_header_labels() {
    return {
        QCoreApplication::translate("query_folder.cpp", "Name"),
        QCoreApplication::translate("query_folder.cpp", "Description"),
    };
}

QList<int> console_query_folder_default_columns() {
    return {QueryColumn_Name, QueryColumn_Description};
}

// NOTE: use constant QUERY_ROOT for root because query root
// item has different name depending on app language. The
// end result is that path for root is equal to
// "QUERY_ROOT", while all other paths start with
// "QUERY_ROOT/a/b/c..."
QString console_query_folder_path(const QModelIndex &index) {
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

void console_query_folder_load(QStandardItem *scope_item, const QList<QStandardItem *> &results_row, const QString &name, const QString &description) {
    auto load_main_item =
    [&](QStandardItem *item) {
        item->setData(description, QueryItemRole_Description);
        item->setData(ItemType_QueryFolder, ConsoleRole_Type);
        item->setIcon(QIcon::fromTheme("folder"));
    };

    load_main_item(scope_item);
    scope_item->setText(name);

    load_main_item(results_row[0]);
    results_row[QueryColumn_Name]->setText(name);
    results_row[QueryColumn_Description]->setText(description);
}

QModelIndex console_query_folder_create(ConsoleWidget *console, const QString &name, const QString &description, const QModelIndex &parent) {
    QStandardItem *scope_item;
    QList<QStandardItem *> results_row;
    console->add_buddy_scope_and_results(console_query_folder_results_id, ScopeNodeType_Static, parent, &scope_item, &results_row);
    console_query_folder_load(scope_item, results_row, name, description);

    return scope_item->index();
}

void console_query_item_load(QStandardItem *scope_item, const QList<QStandardItem *> results_row, const QString &name, const QString &description, const QString &filter, const QByteArray &filter_state, const QString &search_base) {
    auto load_main_item =
    [&](QStandardItem *item) {
        item->setData(ItemType_QueryItem, ConsoleRole_Type);
        item->setData(description, QueryItemRole_Description);
        item->setData(filter, QueryItemRole_Filter);
        item->setData(filter_state, QueryItemRole_FilterState);
        item->setData(search_base, QueryItemRole_SearchBase);
        item->setIcon(QIcon::fromTheme("emblem-system"));
    };

    if (scope_item != nullptr) {
        load_main_item(scope_item);
        scope_item->setText(name);
    }

    if (!results_row.isEmpty()) {
        load_main_item(results_row[0]);
        results_row[QueryColumn_Name]->setText(name);
        results_row[QueryColumn_Description]->setText(description);
    }
}

void console_query_item_create(ConsoleWidget *console, const QString &name, const QString &description, const QString &filter, const QByteArray &filter_state, const QString &search_base, const QModelIndex &parent) {
    QStandardItem *scope_item;
    QList<QStandardItem *> results_row;
    console->add_buddy_scope_and_results(console_object_results_id, ScopeNodeType_Dynamic, parent, &scope_item, &results_row);

    console_query_item_load(scope_item, results_row, name, description, filter, filter_state, search_base);
}

void console_query_item_fetch(ConsoleWidget *console, const QModelIndex &index) {
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    show_busy_indicator();

    const QString filter = index.data(QueryItemRole_Filter).toString();
    const QString search_base = index.data(QueryItemRole_SearchBase).toString();
    const QList<QString> search_attributes = console_object_search_attributes();

    const QHash<QString, AdObject> search_results = ad.search(filter, search_attributes, SearchScope_All, search_base);
    for (const AdObject &object : search_results.values()) {
        const QList<QStandardItem *> results_row = console->add_results_row(index);
        console_object_results_load(results_row, object);
    }

    hide_busy_indicator();
}

void console_query_tree_init(ConsoleWidget *console) {
    // Add head item
    QStandardItem *head_item = console->add_scope_item(console_query_folder_results_id, ScopeNodeType_Static, QModelIndex());
    head_item->setText(QCoreApplication::translate("query", "Saved Queries"));
    head_item->setIcon(QIcon::fromTheme("folder"));
    head_item->setData(ItemType_QueryRoot, ConsoleRole_Type);
    head_item->setDragEnabled(false);

    // Add rest of tree
    const QHash<QString, QVariant> folder_list = g_settings->get_variant(VariantSetting_QueryFolders).toHash();
    const QHash<QString, QVariant> item_list = g_settings->get_variant(VariantSetting_QueryItems).toHash();

    QStack<QPersistentModelIndex> folder_stack;
    folder_stack.append(head_item->index());
    while (!folder_stack.isEmpty()) {
        const QPersistentModelIndex folder_index = folder_stack.pop();

        const QList<QString> child_list =
        [&]() {
            const QString folder_path = console_query_folder_path(folder_index);
            const QHash<QString, QVariant> folder_data = folder_list[folder_path].toHash();
            
            return folder_data["child_list"].toStringList();
        }();

        // Go through children and add them as folders or
        // query items
        for (const QString &path : child_list) {
            if (item_list.contains(path)) {
                // Query item
                const QHash<QString, QVariant> data = item_list[path].toHash();
                console_query_item_load(console, data, folder_index);
            } else if (folder_list.contains(path)) {
                // Query folder
                const QHash<QString, QVariant> data = folder_list[path].toHash();

                const QString name = data["name"].toString();
                const QString description = data["description"].toString();
                const QPersistentModelIndex child_scope_index = console_query_folder_create(console, name, description, folder_index);

                folder_stack.append(child_scope_index);
            }
        }
    }
}

// Saves current state of queries tree to settings. Should
// be called after every modication to queries tree
void console_query_tree_save(ConsoleWidget *console) {
    QHash<QString, QVariant> folder_list;
    QHash<QString, QVariant> item_list;

    const QModelIndex query_root_index = console_query_get_root_index(console);
    if (!query_root_index.isValid()) {
        return;
    }

    QStack<QModelIndex> stack;
    stack.append(query_root_index);

    const QAbstractItemModel *model = query_root_index.model();

    while (!stack.isEmpty()) {
        const QModelIndex index = stack.pop();

        // Add children to stack
        for (int i = 0; i < model->rowCount(index); i++) {
            const QModelIndex child = model->index(i, 0, index);

            stack.append(child);
        }

        const QString path = console_query_folder_path(index);
        const QString parent_path = console_query_folder_path(index.parent());
        const ItemType type = (ItemType) index.data(ConsoleRole_Type).toInt();

        const QList<QString> child_list =
        [&]() {
            QList<QString> out;

            for (int i = 0; i < model->rowCount(index); i++) {
                const QModelIndex child = model->index(i, 0, index);
                const QString child_path = console_query_folder_path(child);

                out.append(child_path);
            }

            return out;
        }();

        if (type == ItemType_QueryRoot) {
            QHash<QString, QVariant> data;
            data["child_list"] = QVariant(child_list);

            folder_list[path] = data;
        } else if (type == ItemType_QueryFolder) {
            const QString name = index.data(Qt::DisplayRole).toString();
            const QString description = index.data(QueryItemRole_Description).toString();

            QHash<QString, QVariant> data;
            data["name"] = name;
            data["description"] = description;
            data["child_list"] = QVariant(child_list);

            folder_list[path] = data;
        } else {
            const QHash<QString, QVariant> data = console_query_item_save(index);

            item_list[path] = data;
        }
    }

    const QVariant folder_variant = QVariant(folder_list);
    const QVariant item_variant = QVariant(item_list);

    g_settings->set_variant(VariantSetting_QueryFolders, folder_variant);
    g_settings->set_variant(VariantSetting_QueryItems, item_variant);
}

bool console_query_or_folder_name_is_good(const QString &name, const QModelIndex &parent_index_raw, QWidget *parent_widget, const QModelIndex &current_index) {
    if (name.isEmpty()) {
        const QString error_text = QString(QCoreApplication::translate("query.cpp", "Name may not be empty"));
        QMessageBox::warning(parent_widget, QCoreApplication::translate("query.cpp", "Error"), error_text);

        return false;
    }

    const QModelIndex parent_index = console_item_convert_to_scope_index(parent_index_raw);

    const QString current_name = current_index.data(Qt::DisplayRole).toString();

    const QList<QString> sibling_names =
    [&]() {
        QList<QString> out;

        QAbstractItemModel *model = (QAbstractItemModel *) parent_index.model();

        for (int row = 0; row < model->rowCount(parent_index); row++) {
            const QModelIndex sibling = model->index(row, 0, parent_index);
            const QString sibling_name = sibling.data(Qt::DisplayRole).toString();

            const bool this_is_query_itself = (sibling == current_index);
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
        const QString error_text = QString(QCoreApplication::translate("query.cpp", "There's already an item with this name."));
        QMessageBox::warning(parent_widget, QCoreApplication::translate("query.cpp", "Error"), error_text);
    } else if (name_contains_slash) {
        const QString error_text = QString(QCoreApplication::translate("query.cpp", "Names cannot contain \"/\"."));
        QMessageBox::warning(parent_widget, QCoreApplication::translate("query.cpp", "Error"), error_text);
    }

    const bool name_is_good = (!name_conflict && !name_contains_slash);

    return name_is_good;
}

void console_query_actions_add_to_menu(ConsoleActions *actions, QMenu *menu) {
    menu->addAction(actions->get(ConsoleAction_QueryEditFolder));
    menu->addAction(actions->get(ConsoleAction_QueryEditItem));
    menu->addAction(actions->get(ConsoleAction_QueryMoveItemOrFolder));
    menu->addAction(actions->get(ConsoleAction_QueryDeleteItemOrFolder));
    menu->addAction(actions->get(ConsoleAction_QueryExport));
    menu->addAction(actions->get(ConsoleAction_QueryImport));
}

void console_query_actions_get_state(const QModelIndex &index, const bool single_selection, QSet<ConsoleAction> *visible_actions, QSet<ConsoleAction> *disabled_actions) {
    const ItemType type = (ItemType) index.data(ConsoleRole_Type).toInt();

    if (single_selection) {
        if (type == ItemType_QueryRoot || type == ItemType_QueryFolder) {
            visible_actions->insert(ConsoleAction_QueryCreateFolder);
            visible_actions->insert(ConsoleAction_QueryCreateItem);
            visible_actions->insert(ConsoleAction_QueryImport);
        }

        if (type == ItemType_QueryFolder) {
            visible_actions->insert(ConsoleAction_QueryEditFolder);
            visible_actions->insert(ConsoleAction_QueryImport);
        }

        if (type == ItemType_QueryItem) {
            visible_actions->insert(ConsoleAction_QueryEditItem);
            visible_actions->insert(ConsoleAction_QueryExport);
        }
    }

    if (type == ItemType_QueryItem || type == ItemType_QueryFolder) {
        visible_actions->insert(ConsoleAction_QueryMoveItemOrFolder);
        visible_actions->insert(ConsoleAction_QueryDeleteItemOrFolder);
    }
}

QModelIndex console_query_get_root_index(ConsoleWidget *console) {
    const QList<QModelIndex> search_results = console->search_scope_by_role(ConsoleRole_Type, ItemType_QueryRoot);

    if (!search_results.isEmpty()) {
        return search_results[0];
    } else {
        qDebug() << "Failed to find query root";

        return QModelIndex();
    }
}

void console_query_can_drop(const QList<QPersistentModelIndex> &dropped_list, const QPersistentModelIndex &target, const QSet<ItemType> &dropped_types, bool *ok) {
    const bool dropped_are_query_item_or_folder = (dropped_types - QSet<ItemType>({ItemType_QueryItem, ItemType_QueryFolder})).isEmpty();
    if (!dropped_are_query_item_or_folder) {
        return;
    }

    const ItemType target_type = (ItemType) target.data(ConsoleRole_Type).toInt();

    const bool target_is_query_folder_or_root = (target_type == ItemType_QueryFolder || target_type == ItemType_QueryRoot);

    *ok = target_is_query_folder_or_root;
}

void console_query_drop(ConsoleWidget *console, const QList<QPersistentModelIndex> &dropped_list, const QPersistentModelIndex &target) {
    console_query_move(console, dropped_list, target);
}

void console_query_move(ConsoleWidget *console, const QList<QPersistentModelIndex> &index_list, const QModelIndex &new_parent_index_raw) {
    const QModelIndex new_parent_index = console_item_convert_to_scope_index(new_parent_index_raw);

    // Check for name conflict
    for (const QPersistentModelIndex &index_raw : index_list) {
        const QModelIndex index = console_item_convert_to_scope_index(index_raw);

        const QString name = index.data(Qt::DisplayRole).toString();
        if (!console_query_or_folder_name_is_good(name, new_parent_index, console, index)) {
            return;
        }
    }

    for (const QPersistentModelIndex &old_index_raw : index_list) {
        // NOTE: convert both moved index and target index to
        // their scope index equivalents (if they aren't scope
        // indexes already). Do this because move process
        // requires iteration through scope tree.
        const QModelIndex old_index = console_item_convert_to_scope_index(old_index_raw);

        // Create a copy of the tree branch at new location. Go
        // down the branch and replicate all of the children.

        // This map contains mapping between old parents of
        // indexes in branch and new parents. Filled out as the
        // branch is copied over to new location.
        QHash<QPersistentModelIndex, QPersistentModelIndex> new_parent_map;
        new_parent_map[old_index.parent()] = new_parent_index;

        QStack<QPersistentModelIndex> stack;
        stack.append(old_index);
        while (!stack.isEmpty()) {
            const QPersistentModelIndex index = stack.pop();

            const QPersistentModelIndex new_parent = new_parent_map[index.parent()];

            const ItemType type = (ItemType) index.data(ConsoleRole_Type).toInt();
            if (type == ItemType_QueryItem) {
                const QString description = index.data(QueryItemRole_Description).toString();
                const QString filter = index.data(QueryItemRole_Filter).toString();
                const QByteArray filter_state = index.data(QueryItemRole_FilterState).toByteArray();
                const QString search_base = index.data(QueryItemRole_SearchBase).toString();
                const QString name = index.data(Qt::DisplayRole).toString();

                console_query_item_create(console, name, description, filter, filter_state, search_base, new_parent);
            } else if (type == ItemType_QueryFolder) {
                const QString name = index.data(Qt::DisplayRole).toString();
                const QString description = index.data(QueryItemRole_Description).toString();
                const QModelIndex folder_index = console_query_folder_create(console, name, description, new_parent);

                new_parent_map[index] = QPersistentModelIndex(folder_index); 
            }

            QAbstractItemModel *index_model = (QAbstractItemModel *)index.model();
            for (int row = 0; row < index_model->rowCount(index); row++) {
                const QModelIndex child = index_model->index(row, 0, index);
                stack.append(QPersistentModelIndex(child));
            }
        }

        // Delete branch at old location
        console->delete_item(old_index);
    }

    console->sort_scope();
    console_query_tree_save(console);
}

void console_query_export(ConsoleWidget *console) {
    const QModelIndex index = get_selected_scope_index(console);

    const QString file_path =
    [&]() {
        const QString query_name = index.data(Qt::DisplayRole).toString();

        const QString caption = QCoreApplication::translate("console_query.cpp", "Export Query");
        const QString suggested_file = QString("%1/%2.json").arg(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), query_name);
        const QString filter = QCoreApplication::translate("console_query.cpp", "JSON (*.json)");

        const QString out = QFileDialog::getSaveFileName(console, caption, suggested_file, filter);

        return out;
    }();

    if (file_path.isEmpty()) {
        return;
    }

    const QHash<QString, QVariant> data = console_query_item_save(index);
    const QByteArray json_bytes = QJsonDocument::fromVariant(data).toJson();

    QFile file(file_path);
    file.open(QIODevice::WriteOnly);
    file.write(json_bytes);
}

void console_query_import(ConsoleWidget *console) {
    const QModelIndex parent_index = get_selected_scope_index(console);

    const QString file_path =
    [&]() {
        const QString caption = QCoreApplication::translate("console_query.cpp", "Import Query");
        const QString dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        const QString file_filter = QCoreApplication::translate("console_query.cpp", "JSON (*.json)");

        const QString out = QFileDialog::getOpenFileName(console, caption, dir, file_filter);

        return out;
    }();

    if (file_path.isEmpty()) {
        return;
    }

    const QHash<QString, QVariant> data = 
    [&]() {
        QFile file(file_path);
        file.open(QIODevice::ReadOnly);
        const QByteArray json_bytes = file.readAll();

        const QJsonDocument json_document = QJsonDocument::fromJson(json_bytes);

        if (json_document.isNull()) {
            const QString error_text = QString(QCoreApplication::translate("query.cpp", "Query file is corrupted."));
            QMessageBox::warning(console, QCoreApplication::translate("query.cpp", "Error"), error_text);

            return QHash<QString, QVariant>();
        }

        const QHash<QString, QVariant> out = json_document.toVariant().toHash();

        return out;
    }();

    console_query_item_load(console, data, parent_index);

    console_query_tree_save(console);
}

QHash<QString, QVariant> console_query_item_save(const QModelIndex &index) {
    const QString name = index.data(Qt::DisplayRole).toString();
    const QString description = index.data(QueryItemRole_Description).toString();
    const QString search_base = index.data(QueryItemRole_SearchBase).toString();
    const QString filter = index.data(QueryItemRole_Filter).toString();
    const QByteArray filter_state = index.data(QueryItemRole_FilterState).toByteArray();

    QHash<QString, QVariant> data;
    data["name"] = name;
    data["description"] = description;
    data["search_base"] = search_base;
    data["filter"] = filter;
    data["filter_state"] = QString(filter_state.toHex());

    return data;
}

void console_query_item_load(ConsoleWidget *console, const QHash<QString, QVariant> &data, const QModelIndex &parent_index) {
    if (data.isEmpty()) {
        return;
    }

    const QString name = data["name"].toString();
    const QString description = data["description"].toString();
    const QString search_base = data["search_base"].toString();
    const QString filter = data["filter"].toString();
    const QByteArray filter_state = QByteArray::fromHex(data["filter_state"].toString().toLocal8Bit());

    if (!console_query_or_folder_name_is_good(name, parent_index, console, QModelIndex())) {
        return;
    }

    console_query_item_create(console, name, description, filter, filter_state, search_base, parent_index);
}
