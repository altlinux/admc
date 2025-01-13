/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2022 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
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

#include "console_impls/query_folder_impl.h"

#include "adldap.h"
#include "console_impls/item_type.h"
#include "console_impls/object_impl.h"
#include "console_impls/query_item_impl.h"
#include "console_widget/results_view.h"
#include "create_dialogs/create_query_folder_dialog.h"
#include "create_dialogs/create_query_item_dialog.h"
#include "edit_query_widgets/edit_query_folder_dialog.h"
#include "globals.h"
#include "settings.h"
#include "utils.h"
#include "managers/icon_manager.h"

#include <QFileDialog>
#include <QJsonDocument>
#include <QMenu>
#include <QStack>
#include <QStandardItem>
#include <QStandardPaths>

#define QUERY_ROOT "QUERY_ROOT"

void console_query_move(ConsoleWidget *console, const QList<QPersistentModelIndex> &index_list, const QModelIndex &new_parent_index, const bool delete_old_branch = true);

QueryFolderImpl::QueryFolderImpl(ConsoleWidget *console_arg)
: ConsoleImpl(console_arg) {
    copied_is_cut = false;

    set_results_view(new ResultsView(console_arg));

    auto create_query_folder_action = new QAction(tr("Query folder"), this);
    auto create_query_item_action = new QAction(tr("Query item"), this);

    auto new_menu = new QMenu(tr("New"), console_arg);
    new_action = new_menu->menuAction();

    new_menu->addAction(create_query_folder_action);
    new_menu->addAction(create_query_item_action);

    edit_action = new QAction(tr("Edit"), this);

    import_action = new QAction(tr("&Import query..."), this);

    connect(
        create_query_folder_action, &QAction::triggered,
        this, &QueryFolderImpl::on_create_query_folder);
    connect(
        create_query_item_action, &QAction::triggered,
        this, &QueryFolderImpl::on_create_query_item);
    connect(
        edit_action, &QAction::triggered,
        this, &QueryFolderImpl::on_edit_query_folder);
    connect(
        import_action, &QAction::triggered,
        this, &QueryFolderImpl::on_import);
}

void QueryFolderImpl::on_create_query_folder() {
    auto dialog = new CreateQueryFolderDialog(console);

    const QModelIndex parent_index = console->get_selected_item(ItemType_QueryFolder);
    const QList<QString> sibling_name_list = get_sibling_name_list(parent_index, QModelIndex());

    dialog->set_sibling_name_list(sibling_name_list);

    dialog->open();

    connect(
        dialog, &QDialog::accepted,
        this,
        [this, dialog, parent_index]() {
            const QString name = dialog->name();
            const QString description = dialog->description();

            console_query_folder_create(console, name, description, parent_index);

            console_query_tree_save(console);
        });
}

void QueryFolderImpl::on_create_query_item() {
    const QModelIndex parent_index = console->get_selected_item(ItemType_QueryFolder);
    const QList<QString> sibling_name_list = get_sibling_name_list(parent_index, QModelIndex());

    auto dialog = new CreateQueryItemDialog(sibling_name_list, console);
    dialog->open();

    connect(
        dialog, &QDialog::accepted,
        this,
        [this, dialog, parent_index]() {
            const QString name = dialog->name();
            const QString description = dialog->description();
            const QString filter = dialog->filter();
            const QString base = dialog->base();
            const QByteArray filter_state = dialog->filter_state();
            const bool scope_is_children = dialog->scope_is_children();

            console_query_item_create(console, name, description, filter, filter_state, base, scope_is_children, parent_index);

            console_query_tree_save(console);
        });
}

void QueryFolderImpl::on_edit_query_folder() {
    auto dialog = new EditQueryFolderDialog(console);

    const QModelIndex index = console->get_selected_item(ItemType_QueryFolder);

    {
        const QString name = index.data(Qt::DisplayRole).toString();
        const QString description = index.data(QueryItemRole_Description).toString();
        const QModelIndex parent_index = index.parent();
        const QList<QString> sibling_name_list = get_sibling_name_list(parent_index, index);

        dialog->set_data(sibling_name_list, name, description);
    }

    dialog->open();

    connect(
        dialog, &QDialog::accepted,
        this,
        [this, dialog, index]() {
            const QString name = dialog->name();
            const QString description = dialog->description();

            const QList<QStandardItem *> row = console->get_row(index);
            console_query_folder_load(row, name, description);

            console_query_tree_save(console);
        });
}

bool QueryFolderImpl::can_drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type) {
    UNUSED_ARG(dropped_list);
    UNUSED_ARG(target);
    UNUSED_ARG(target_type);

    const bool dropped_is_target = dropped_list.contains(target);

    if (dropped_is_target) {
        return false;
    }

    const bool dropped_are_query_item_or_folder = (dropped_type_list - QSet<int>({ItemType_QueryItem, ItemType_QueryFolder})).isEmpty();

    return dropped_are_query_item_or_folder;
}

void QueryFolderImpl::drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type) {
    UNUSED_ARG(dropped_type_list);
    UNUSED_ARG(target_type);

    console_query_move(console, dropped_list, target);
}

QList<QAction *> QueryFolderImpl::get_all_custom_actions() const {
    QList<QAction *> out;

    out.append(new_action);
    out.append(edit_action);
    out.append(import_action);

    return out;
}

QSet<QAction *> QueryFolderImpl::get_custom_actions(const QModelIndex &index, const bool single_selection) const {
    UNUSED_ARG(index);

    const bool is_root = [&]() {
        QStandardItem *item = console->get_item(index);
        const bool out = item->data(QueryItemRole_IsRoot).toBool();

        return out;
    }();

    QSet<QAction *> out;

    if (single_selection) {
        if (is_root) {
            out.insert(new_action);
            out.insert(import_action);
        } else {
            out.insert(new_action);
            out.insert(edit_action);
            out.insert(import_action);
        }
    }

    return out;
}

QSet<StandardAction> QueryFolderImpl::get_standard_actions(const QModelIndex &index, const bool single_selection) const {
    UNUSED_ARG(index);

    const bool is_root = [&]() {
        QStandardItem *item = console->get_item(index);
        const bool out = item->data(QueryItemRole_IsRoot).toBool();

        return out;
    }();

    QSet<StandardAction> out;

    if (!is_root) {
        out.insert(StandardAction_Delete);
    }

    if (single_selection) {
        if (is_root) {
            out.insert(StandardAction_Paste);
        } else {
            out.insert(StandardAction_Cut);
            out.insert(StandardAction_Copy);
            out.insert(StandardAction_Paste);
        }
    }

    return out;
}

void QueryFolderImpl::delete_action(const QList<QModelIndex> &index_list) {
    query_action_delete(console, index_list);
}

void QueryFolderImpl::cut(const QList<QModelIndex> &index_list) {
    copied_list = persistent_index_list(index_list);
    copied_is_cut = true;
}

void QueryFolderImpl::copy(const QList<QModelIndex> &index_list) {
    copied_list = persistent_index_list(index_list);
    copied_is_cut = false;
}

void QueryFolderImpl::paste(const QList<QModelIndex> &index_list) {
    const QModelIndex parent_index = index_list[0];

    const bool recursive_cut_and_paste = (copied_is_cut && copied_list.contains(parent_index));
    if (recursive_cut_and_paste) {
        message_box_warning(console, tr("Error"), tr("Can't cut and paste query folder into itself."));

        return;
    }

    const bool parent_is_same = [&]() {
        for (const QModelIndex &index : copied_list) {
            const QModelIndex this_parent = index.parent();

            if (this_parent == parent_index) {
                return true;
            }
        }

        return false;
    }();

    // TODO: this is a band-aid on top of name conflict
    // check inside move(), try to make this
    // centralized if possible

    // Prohibit copy+paste into same parent.
    // console_query_move() does check for name
    // conflicts but doesn't handle this edge case.
    if (!copied_is_cut && parent_is_same) {
        message_box_warning(console, tr("Error"), tr("There's already an item with this name."));

        return;
    }

    const bool delete_old_branch = copied_is_cut;
    console_query_move(console, copied_list, parent_index, delete_old_branch);
}

void QueryFolderImpl::on_import() {
    const QModelIndex parent_index = console->get_selected_item(ItemType_QueryFolder);

    const QList<QString> path_list = [&]() {
        const QString caption = QCoreApplication::translate("query_item_impl.cpp", "Import Query");
        const QString dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        const QString file_filter = QCoreApplication::translate("query_item_impl.cpp", "JSON (*.json)");

        const QList<QString> out = QFileDialog::getOpenFileNames(console, caption, dir, file_filter);

        return out;
    }();

    if (path_list.isEmpty()) {
        return;
    }

    for (const QString &file_path : path_list) {
        const QHash<QString, QVariant> data = [&]() {
            QFile file(file_path);
            file.open(QIODevice::ReadOnly);
            const QByteArray json_bytes = file.readAll();

            const QJsonDocument json_document = QJsonDocument::fromJson(json_bytes);

            if (json_document.isNull()) {
                const QString error_text = QString(QCoreApplication::translate("query.cpp", "Query file is corrupted."));
                message_box_warning(console, QCoreApplication::translate("query.cpp", "Error"), error_text);

                return QHash<QString, QVariant>();
            }

            const QHash<QString, QVariant> out = json_document.toVariant().toHash();

            return out;
        }();

        console_query_item_load_hash(console, data, parent_index);
    }

    console_query_tree_save(console);
}

void console_query_tree_init(ConsoleWidget *console) {
    const QList<QStandardItem *> root_row = console->add_scope_item(ItemType_QueryFolder, console->domain_info_index());
    auto root = root_row[0];
    root->setText(QCoreApplication::translate("query", "Saved Queries"));
    root->setIcon(g_icon_manager->get_object_icon(ADMC_CATEGORY_QUERY_FOLDER));
    root->setDragEnabled(false);
    root->setData(true, QueryItemRole_IsRoot);

    // Add rest of tree
    const QHash<QString, QVariant> folder_list = settings_get_variant(SETTING_query_folders).toHash();
    const QHash<QString, QVariant> item_list = settings_get_variant(SETTING_query_items).toHash();

    QStack<QPersistentModelIndex> folder_stack;
    folder_stack.append(root->index());
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
                console_query_item_load_hash(console, data, folder_index);
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
    const QModelIndex root = get_query_tree_root(console);
    if (!root.isValid()) {
        return;
    }

    QHash<QString, QVariant> folder_list;
    QHash<QString, QVariant> item_list;

    QStack<QModelIndex> stack;
    stack.append(root);

    const QAbstractItemModel *model = root.model();

    while (!stack.isEmpty()) {
        const QModelIndex index = stack.pop();

        // Add children to stack
        for (int i = 0; i < model->rowCount(index); i++) {
            const QModelIndex child = model->index(i, 0, index);

            stack.append(child);
        }

        const QString path = console_query_folder_path(index);
        const QString parent_path = console_query_folder_path(index.parent());
        const ItemType type = (ItemType) console_item_get_type(index);

        const QList<QString> child_list = [&]() {
            QList<QString> out;

            for (int i = 0; i < model->rowCount(index); i++) {
                const QModelIndex child = model->index(i, 0, index);
                const QString child_path = console_query_folder_path(child);

                out.append(child_path);
            }

            return out;
        }();

        if (type == ItemType_QueryFolder) {
            const bool is_root = !index.parent().isValid();
            if (is_root) {
                QHash<QString, QVariant> data;
                data["child_list"] = QVariant(child_list);

                folder_list[path] = data;
            } else {
                const QString name = index.data(Qt::DisplayRole).toString();
                const QString description = index.data(QueryItemRole_Description).toString();

                QHash<QString, QVariant> data;
                data["name"] = name;
                data["description"] = description;
                data["child_list"] = QVariant(child_list);

                folder_list[path] = data;
            }
        } else if (type == ItemType_QueryItem) {
            const QHash<QString, QVariant> data = console_query_item_save_hash(index);

            item_list[path] = data;
        }
    }

    const QVariant folder_variant = QVariant(folder_list);
    const QVariant item_variant = QVariant(item_list);

    settings_set_variant(SETTING_query_folders, folder_variant);
    settings_set_variant(SETTING_query_items, item_variant);
}

QModelIndex get_query_tree_root(ConsoleWidget *console) {
    const QModelIndex out = console->search_item(console->domain_info_index(), QueryItemRole_IsRoot, true, {ItemType_QueryFolder});

    return out;
}

QList<QString> QueryFolderImpl::column_labels() const {
    return {
        QCoreApplication::translate("query_folder.cpp", "Name"),
        QCoreApplication::translate("query_folder.cpp", "Description"),
    };
}

QList<int> QueryFolderImpl::default_columns() const {
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

QModelIndex console_query_folder_create(ConsoleWidget *console, const QString &name, const QString &description, const QModelIndex &parent) {
    const QList<QStandardItem *> row = console->add_scope_item(ItemType_QueryFolder, parent);
    console_query_folder_load(row, name, description);

    return row[0]->index();
}

void console_query_folder_load(const QList<QStandardItem *> &row, const QString &name, const QString &description) {
    QStandardItem *main_item = row[0];
    main_item->setData(description, QueryItemRole_Description);
    main_item->setIcon(g_icon_manager->get_object_icon(ADMC_CATEGORY_QUERY_FOLDER));
    main_item->setData(false, QueryItemRole_IsRoot);

    row[QueryColumn_Name]->setText(name);
    row[QueryColumn_Description]->setText(description);
}

bool console_query_or_folder_name_is_good(const QString &name, const QList<QString> &sibling_names, QWidget *parent_widget) {
    if (name.isEmpty()) {
        const QString error_text = QString(QCoreApplication::translate("query.cpp", "Name may not be empty"));
        message_box_warning(parent_widget, QCoreApplication::translate("query.cpp", "Error"), error_text);

        return false;
    }

    const bool name_conflict = sibling_names.contains(name);
    const bool name_contains_slash = name.contains("/");

    if (name_conflict) {
        const QString error_text = QString(QCoreApplication::translate("query.cpp", "There's already an item with this name."));
        message_box_warning(parent_widget, QCoreApplication::translate("query.cpp", "Error"), error_text);
    } else if (name_contains_slash) {
        const QString error_text = QString(QCoreApplication::translate("query.cpp", "Names cannot contain \"/\"."));
        message_box_warning(parent_widget, QCoreApplication::translate("query.cpp", "Error"), error_text);
    }

    const bool name_is_good = (!name_conflict && !name_contains_slash);

    return name_is_good;
}

bool console_query_or_folder_name_is_good(const QString &name, const QModelIndex &parent_index, QWidget *parent_widget, const QModelIndex &current_index) {
    const QList<QString> sibling_names = get_sibling_name_list(parent_index, current_index);

    return console_query_or_folder_name_is_good(name, sibling_names, parent_widget);
}

void query_action_delete(ConsoleWidget *console, const QList<QModelIndex> &index_list) {
    const bool confirmed = confirmation_dialog(QCoreApplication::translate("query_folder_impl.cpp", "Are you sure you want to delete this item?"), console);
    if (!confirmed) {
        return;
    }

    const QList<QPersistentModelIndex> persistent_list = persistent_index_list(index_list);

    for (const QPersistentModelIndex &index : persistent_list) {
        console->delete_item(index);
    }

    console_query_tree_save(console);
}

void console_query_move(ConsoleWidget *console, const QList<QPersistentModelIndex> &index_list, const QModelIndex &new_parent_index, const bool delete_old_branch) {
    // Check for name conflict
    for (const QPersistentModelIndex &index : index_list) {
        const QString name = index.data(Qt::DisplayRole).toString();
        if (!console_query_or_folder_name_is_good(name, new_parent_index, console, index)) {
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
        QList<QPersistentModelIndex> created_list;
        stack.append(old_index);
        while (!stack.isEmpty()) {
            const QPersistentModelIndex index = stack.pop();

            const QPersistentModelIndex new_parent = new_parent_map[index.parent()];

            const ItemType type = (ItemType) console_item_get_type(index);
            if (type == ItemType_QueryItem) {
                const QString description = index.data(QueryItemRole_Description).toString();
                const QString filter = index.data(QueryItemRole_Filter).toString();
                const QByteArray filter_state = index.data(QueryItemRole_FilterState).toByteArray();
                const QString base = index.data(QueryItemRole_Base).toString();
                const QString name = index.data(Qt::DisplayRole).toString();
                const bool scope_is_children = index.data(QueryItemRole_ScopeIsChildren).toBool();

                const QModelIndex item_index = console_query_item_create(console, name, description, filter, filter_state, base, scope_is_children, new_parent);

                created_list.append(item_index);
            } else if (type == ItemType_QueryFolder) {
                const QString name = index.data(Qt::DisplayRole).toString();
                const QString description = index.data(QueryItemRole_Description).toString();
                const QModelIndex folder_index = console_query_folder_create(console, name, description, new_parent);

                created_list.append(folder_index);

                new_parent_map[index] = QPersistentModelIndex(folder_index);
            }

            QAbstractItemModel *index_model = (QAbstractItemModel *) index.model();
            for (int row = 0; row < index_model->rowCount(index); row++) {
                const QModelIndex child = index_model->index(row, 0, index);

                // NOTE: don't add indexes that were
                // created during the move process, to
                // avoid infinite loop. This can happen
                // when copying and pasting a folder
                // into itself.
                const bool child_was_created = created_list.contains(QPersistentModelIndex(child));
                if (!child_was_created) {
                    stack.append(QPersistentModelIndex(child));
                }
            }
        }

        // Delete branch at old location
        if (delete_old_branch) {
            console->delete_item(old_index);
        }
    }

    console_query_tree_save(console);
}

QList<QString> get_sibling_name_list(const QModelIndex &parent_index, const QModelIndex &index_to_omit) {
    QList<QString> out;

    const QAbstractItemModel *model = parent_index.model();

    for (int row = 0; row < model->rowCount(parent_index); row++) {
        const QModelIndex sibling = model->index(row, 0, parent_index);
        const QString sibling_name = sibling.data(Qt::DisplayRole).toString();

        const bool this_is_query_itself = (sibling == index_to_omit);
        if (this_is_query_itself) {
            continue;
        }

        out.append(sibling_name);
    }

    return out;
}
