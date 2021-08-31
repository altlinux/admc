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
#include "console_types/object_impl.h"
#include "console_types/query_folder_impl.h"
#include "globals.h"
#include "settings.h"
#include "utils.h"
#include "create_query_item_dialog.h"
#include "edit_query_item_dialog.h"
#include "item_type.h"

#include <QCoreApplication>
#include <QFileDialog>
#include <QJsonDocument>
#include <QMenu>
#include <QStack>
#include <QStandardItem>
#include <QStandardPaths>

#define QUERY_ROOT "QUERY_ROOT"

void console_query_item_load(const QList<QStandardItem *> row, const QString &name, const QString &description, const QString &filter, const QByteArray &filter_state, const QString &base, const bool scope_is_children) {
    QStandardItem *main_item = row[0];
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
    const QList<QStandardItem *> row = console->add_scope_item(ItemType_QueryItem, parent);

    console_query_item_load(row, name, description, filter, filter_state, base, scope_is_children);
}

void ConsoleQueryItem::fetch(const QModelIndex &index) {
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

void ConsoleQueryItem::on_export() {
    const QModelIndex index = console->get_selected_item(ItemType_QueryItem);

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

    if (!console_query_or_folder_name_is_good(name, parent_index, console, QModelIndex())) {
        return;
    }

    console_query_item_create(console, name, description, filter, filter_state, base, scope_is_children, parent_index);
}

void ConsoleQueryItem::on_edit() {
    auto dialog = new EditQueryItemDialog(console);
    dialog->open();
}

ConsoleQueryItem::ConsoleQueryItem(ConsoleWidget *console_arg)
: ConsoleImpl(console_arg) {
    edit_action = new QAction(tr("Edit..."), this);
    export_action = new QAction(tr("Export query..."), this);

    connect(
        edit_action, &QAction::triggered,
        this, &ConsoleQueryItem::on_edit);
    connect(
        export_action, &QAction::triggered,
        this, &ConsoleQueryItem::on_export);
}

QString ConsoleQueryItem::get_description(const QModelIndex &index) const {
    const QString object_count_text = console_object_count_string(console, index);

    return object_count_text;
}

QList<QAction *> ConsoleQueryItem::get_all_custom_actions() const {
    QList<QAction *> out;

    out.append(edit_action);
    out.append(export_action);

    return out;
}

QSet<QAction *> ConsoleQueryItem::get_custom_actions(const QModelIndex &index, const bool single_selection) const {
    QSet<QAction *> out;

    if (single_selection) {
        out.insert(edit_action);
        out.insert(export_action);
    }

    return out;
}

QSet<StandardAction> ConsoleQueryItem::get_standard_actions(const QModelIndex &index, const bool single_selection) const {
    QSet<StandardAction> out;

    out.insert(StandardAction_Delete);

    // TODO: currently implementation only supports single
    // selection cut/copy but probably should be able to do
    // multi?
    if (single_selection) {
        out.insert(StandardAction_Cut);
        out.insert(StandardAction_Copy);
    }
    
    return out;
}

void ConsoleQueryItem::refresh(const QList<QModelIndex> &index_list) {
    if (index_list.size() != 1) {
        return;
    }

    const QModelIndex index = index_list[0];

    console->delete_children(index);
    fetch(index);
}

void ConsoleQueryItem::delete_action(const QList<QModelIndex> &index_list) {
    query_action_delete(console, index_list);
}

void ConsoleQueryItem::cut(const QList<QModelIndex> &index_list) {
    query_action_cut(index_list);
}

void ConsoleQueryItem::copy(const QList<QModelIndex> &index_list) {
    query_action_copy(index_list);
}
