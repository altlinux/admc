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

#include "console_types/console_query.h"

#include "adldap.h"
#include "central_widget.h"
#include "console_actions.h"
#include "console_types/console_object.h"
#include "globals.h"
#include "settings.h"
#include "utils.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFileDialog>
#include <QJsonDocument>
#include <QMenu>
#include <QMessageBox>
#include <QStack>
#include <QStandardItem>
#include <QStandardPaths>

#define QUERY_ROOT "QUERY_ROOT"

enum QueryColumn {
    QueryColumn_Name,
    QueryColumn_Description,

    QueryColumn_COUNT,
};

int console_query_folder_results_id;
QStandardItem *query_tree_head = nullptr;

bool copied_index_is_cut = false;
QPersistentModelIndex copied_index;

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

    const QList<QString> path_split = [&index]() {
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

    const QString path = [&path_split]() {
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

void console_query_folder_load(const QList<QStandardItem *> &row, const QString &name, const QString &description) {
    QStandardItem *main_item = row[0];
    main_item->setData(description, QueryItemRole_Description);
    main_item->setData(ItemType_QueryFolder, ConsoleRole_Type);
    main_item->setIcon(QIcon::fromTheme("folder"));

    row[QueryColumn_Name]->setText(name);
    row[QueryColumn_Description]->setText(description);
}

QModelIndex console_query_folder_create(ConsoleWidget *console, const QString &name, const QString &description, const QModelIndex &parent) {
    const QList<QStandardItem *> row = console->add_scope_item(console_query_folder_results_id, ScopeNodeType_Static, parent);
    console_query_folder_load(row, name, description);

    return row[0]->index();
}

void console_query_item_load(const QList<QStandardItem *> row, const QString &name, const QString &description, const QString &filter, const QByteArray &filter_state, const QString &base, const bool scope_is_children) {
    QStandardItem *main_item = row[0];
    main_item->setData(ItemType_QueryItem, ConsoleRole_Type);
    main_item->setData(description, QueryItemRole_Description);
    main_item->setData(filter, QueryItemRole_Filter);
    main_item->setData(filter_state, QueryItemRole_FilterState);
    main_item->setData(base, QueryItemRole_Base);
    main_item->setData(scope_is_children, QueryItemRole_ScopeIsChildren);
    main_item->setIcon(QIcon::fromTheme("emblem-system"));

    row[QueryColumn_Name]->setText(name);
    row[QueryColumn_Description]->setText(description);
}

void console_query_item_create(ConsoleWidget *console, const QString &name, const QString &description, const QString &filter, const QByteArray &filter_state, const QString &base, const bool scope_is_children, const QModelIndex &parent) {
    const QList<QStandardItem *> row = console->add_scope_item(console_object_results_id, ScopeNodeType_Dynamic, parent);

    console_query_item_load(row, name, description, filter, filter_state, base, scope_is_children);
}

void console_query_item_fetch(ConsoleWidget *console, const QModelIndex &index) {
    const QString filter = index.data(QueryItemRole_Filter).toString();
    const QString base = index.data(QueryItemRole_Base).toString();
    const QList<QString> search_attributes = console_object_search_attributes();
    const SearchScope scope = [&]() {
        const bool scope_is_children = index.data(QueryItemRole_ScopeIsChildren).toBool();
        if (scope_is_children) {
            return SearchScope_Children;
        } else {
            return SearchScope_All;
        }
    }();

    console_object_search(console, index, base, scope, filter, search_attributes);
}

void console_query_tree_init(ConsoleWidget *console) {
    query_tree_head = console->add_top_item(console_query_folder_results_id, ScopeNodeType_Static);
    query_tree_head->setText(QCoreApplication::translate("query", "Saved Queries"));
    query_tree_head->setIcon(QIcon::fromTheme("folder"));
    query_tree_head->setData(ItemType_QueryRoot, ConsoleRole_Type);
    query_tree_head->setDragEnabled(false);

    // Add rest of tree
    const QHash<QString, QVariant> folder_list = g_settings->get_variant(VariantSetting_QueryFolders).toHash();
    const QHash<QString, QVariant> item_list = g_settings->get_variant(VariantSetting_QueryItems).toHash();

    QStack<QPersistentModelIndex> folder_stack;
    folder_stack.append(query_tree_head->index());
    while (!folder_stack.isEmpty()) {
        const QPersistentModelIndex folder_index = folder_stack.pop();

        const QList<QString> child_list = [&]() {
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
                const QPersistentModelIndex child_index = console_query_folder_create(console, name, description, folder_index);

                folder_stack.append(child_index);
            }
        }
    }
}

// Saves current state of queries tree to settings. Should
// be called after every modication to queries tree
void console_query_tree_save(ConsoleWidget *console) {
    QHash<QString, QVariant> folder_list;
    QHash<QString, QVariant> item_list;

    QStack<QModelIndex> stack;
    stack.append(query_tree_head->index());

    const QAbstractItemModel *model = query_tree_head->model();

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

        const QList<QString> child_list = [&]() {
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

bool console_query_or_folder_name_is_good(ConsoleWidget *console, const QString &name, const QModelIndex &parent_index, QWidget *parent_widget, const QModelIndex &current_index) {
    if (name.isEmpty()) {
        const QString error_text = QString(QCoreApplication::translate("query.cpp", "Name may not be empty"));
        QMessageBox::warning(parent_widget, QCoreApplication::translate("query.cpp", "Error"), error_text);

        return false;
    }

    const QString current_name = current_index.data(Qt::DisplayRole).toString();

    const QList<QString> sibling_names = [&]() {
        QList<QString> out;

        const QAbstractItemModel *model = parent_index.model();

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
    menu->addAction(actions->get(ConsoleAction_QueryCutItemOrFolder));
    menu->addAction(actions->get(ConsoleAction_QueryCopyItemOrFolder));
    menu->addAction(actions->get(ConsoleAction_QueryPasteItemOrFolder));
    menu->addAction(actions->get(ConsoleAction_QueryDeleteItemOrFolder));
    menu->addAction(actions->get(ConsoleAction_QueryExport));
    menu->addAction(actions->get(ConsoleAction_QueryImport));
}

void console_query_actions_get_state(const QModelIndex &index, const bool single_selection, QSet<ConsoleAction> *visible_actions, QSet<ConsoleAction> *disabled_actions) {
    const ItemType type = (ItemType) index.data(ConsoleRole_Type).toInt();

    QSet<ConsoleAction> my_visible_actions;

    if (single_selection) {
        if (type == ItemType_QueryRoot || type == ItemType_QueryFolder) {
            my_visible_actions.insert(ConsoleAction_QueryCreateFolder);
            my_visible_actions.insert(ConsoleAction_QueryCreateItem);
            my_visible_actions.insert(ConsoleAction_QueryImport);
        }

        if (type == ItemType_QueryFolder || type == ItemType_QueryItem) {
            my_visible_actions.insert(ConsoleAction_QueryCutItemOrFolder);
            my_visible_actions.insert(ConsoleAction_QueryCopyItemOrFolder);
        }

        if (type == ItemType_QueryFolder) {
            if (copied_index.isValid()) {
                my_visible_actions.insert(ConsoleAction_QueryPasteItemOrFolder);
            }

            my_visible_actions.insert(ConsoleAction_QueryEditFolder);
            my_visible_actions.insert(ConsoleAction_QueryImport);
        }

        if (type == ItemType_QueryItem) {
            my_visible_actions.insert(ConsoleAction_QueryEditItem);
            my_visible_actions.insert(ConsoleAction_QueryExport);
        }
    }

    if (type == ItemType_QueryItem || type == ItemType_QueryFolder) {
        my_visible_actions.insert(ConsoleAction_QueryDeleteItemOrFolder);
    }

    visible_actions->unite(my_visible_actions);

    const bool is_fetching = console_get_item_fetching(index);
    if (is_fetching) {
        disabled_actions->unite(my_visible_actions);
    }
}

QModelIndex console_query_get_root_index(ConsoleWidget *console) {
    const QList<QModelIndex> results = console->search_items(query_tree_head->index(), ConsoleRole_Type, ItemType_QueryRoot);

    if (!results.isEmpty()) {
        return results[0];
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

    const bool target_is_fetching = console_get_item_fetching(target);
    if (target_is_fetching) {
        return;
    }

    const ItemType target_type = (ItemType) target.data(ConsoleRole_Type).toInt();

    const bool target_is_query_folder_or_root = (target_type == ItemType_QueryFolder || target_type == ItemType_QueryRoot);

    *ok = target_is_query_folder_or_root;
}

void console_query_drop(ConsoleWidget *console, const QList<QPersistentModelIndex> &dropped_list, const QPersistentModelIndex &target) {
    console_query_move(console, dropped_list, target);
}

void console_query_move(ConsoleWidget *console, const QList<QPersistentModelIndex> &index_list, const QModelIndex &new_parent_index, const bool delete_old_branch) {
    // Check for name conflict
    for (const QPersistentModelIndex &index : index_list) {
        const QString name = index.data(Qt::DisplayRole).toString();
        if (!console_query_or_folder_name_is_good(console, name, new_parent_index, console, index)) {
            return;
        }
    }

    for (const QPersistentModelIndex &old_index : index_list) {
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
                const QString base = index.data(QueryItemRole_Base).toString();
                const QString name = index.data(Qt::DisplayRole).toString();
                const bool scope_is_children = index.data(QueryItemRole_ScopeIsChildren).toBool();

                console_query_item_create(console, name, description, filter, filter_state, base, scope_is_children, new_parent);
            } else if (type == ItemType_QueryFolder) {
                const QString name = index.data(Qt::DisplayRole).toString();
                const QString description = index.data(QueryItemRole_Description).toString();
                const QModelIndex folder_index = console_query_folder_create(console, name, description, new_parent);

                new_parent_map[index] = QPersistentModelIndex(folder_index);
            }

            QAbstractItemModel *index_model = (QAbstractItemModel *) index.model();
            for (int row = 0; row < index_model->rowCount(index); row++) {
                const QModelIndex child = index_model->index(row, 0, index);
                stack.append(QPersistentModelIndex(child));
            }
        }

        // Delete branch at old location
        if (delete_old_branch) {
            console->delete_item(old_index);
        }
    }

    console_query_tree_save(console);
}

void console_query_export(ConsoleWidget *console) {
    const QModelIndex index = console->get_selected_item();

    const QString file_path = [&]() {
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
    const QModelIndex parent_index = console->get_selected_item();

    const QString file_path = [&]() {
        const QString caption = QCoreApplication::translate("console_query.cpp", "Import Query");
        const QString dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        const QString file_filter = QCoreApplication::translate("console_query.cpp", "JSON (*.json)");

        const QString out = QFileDialog::getOpenFileName(console, caption, dir, file_filter);

        return out;
    }();

    if (file_path.isEmpty()) {
        return;
    }

    const QHash<QString, QVariant> data = [&]() {
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

void console_query_cut(ConsoleWidget *console) {
    copied_index = console->get_selected_item();
    copied_index_is_cut = true;
}

void console_query_copy(ConsoleWidget *console) {
    copied_index = console->get_selected_item();
    copied_index_is_cut = false;
}

void console_query_paste(ConsoleWidget *console) {
    const QModelIndex parent_index = console->get_selected_item();
    const bool delete_old_branch = copied_index_is_cut;
    console_query_move(console, {copied_index}, parent_index, delete_old_branch);
}

QHash<QString, QVariant> console_query_item_save(const QModelIndex &index) {
    const QString name = index.data(Qt::DisplayRole).toString();
    const QString description = index.data(QueryItemRole_Description).toString();
    const QString base = index.data(QueryItemRole_Base).toString();
    const QString filter = index.data(QueryItemRole_Filter).toString();
    const QByteArray filter_state = index.data(QueryItemRole_FilterState).toByteArray();
    const bool scope_is_children = index.data(QueryItemRole_ScopeIsChildren).toBool();

    QHash<QString, QVariant> data;
    data["name"] = name;
    data["description"] = description;
    data["base"] = base;
    data["filter"] = filter;
    data["filter_state"] = filter_state.toHex();
    data["scope_is_children"] = scope_is_children;

    return data;
}

void console_query_item_load(ConsoleWidget *console, const QHash<QString, QVariant> &data, const QModelIndex &parent_index) {
    if (data.isEmpty()) {
        return;
    }

    const QString name = data["name"].toString();
    const QString description = data["description"].toString();
    const QString base = data["base"].toString();
    const bool scope_is_children = data["scope_is_children"].toBool();
    const QString filter = data["filter"].toString();
    const QByteArray filter_state = QByteArray::fromHex(data["filter_state"].toString().toLocal8Bit());

    if (!console_query_or_folder_name_is_good(console, name, parent_index, console, QModelIndex())) {
        return;
    }

    console_query_item_create(console, name, description, filter, filter_state, base, scope_is_children, parent_index);
}
