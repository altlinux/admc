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

#include "move_query_dialog.h"

#include "adldap.h"
#include "globals.h"
#include "settings.h"
#include "utils.h"
#include "central_widget.h"
#include "console_widget/console_widget.h"
#include "console_types/console_query.h"

#include <QTreeView>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QStack>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

MoveQueryDialog::MoveQueryDialog(ConsoleWidget *console_arg)
: QDialog(console_arg) {
    setWindowTitle(tr("Select folder"));

    resize(400, 500);

    console = console_arg;

    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setExpandsOnDoubleClick(true);
    view->setSortingEnabled(true);
    view->sortByColumn(0, Qt::AscendingOrder);
    view->setHeaderHidden(true);

    model = new QStandardItemModel(this);

    proxy_model = new QSortFilterProxyModel(this);
    proxy_model->setSourceModel(model);
    proxy_model->setSortCaseSensitivity(Qt::CaseInsensitive);

    view->setModel(proxy_model);

    auto buttonbox = new QDialogButtonBox();
    buttonbox->addButton(QDialogButtonBox::Ok);
    buttonbox->addButton(QDialogButtonBox::Cancel);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(view);
    layout->addWidget(buttonbox);

    enable_widget_on_selection(buttonbox->button(QDialogButtonBox::Ok), view);

    connect(
        buttonbox, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
    connect(
        buttonbox, &QDialogButtonBox::rejected,
        this, &QDialog::reject);
}

// TODO: duplicating console_query_tree_init()
void MoveQueryDialog::open() {
    model->removeRows(0, model->rowCount());

    QStandardItem *head_item = new QStandardItem();
    head_item->setText(tr("Saved Queries"));
    head_item->setIcon(QIcon::fromTheme("folder"));
    head_item->setData(ItemType_QueryRoot, ConsoleRole_Type);
    model->appendRow(head_item);

    const QJsonObject folder_list = QJsonDocument::fromJson(g_settings->get_variant(VariantSetting_QueryFolders).toByteArray()).object();

    QStack<QModelIndex> query_stack;
    query_stack.append(head_item->index());
    while (!query_stack.isEmpty()) {
        const QModelIndex folder_index = query_stack.pop();
        QStandardItem *folder_item  = model->itemFromIndex(folder_index);

        const QJsonArray child_list =
        [&]() {
            const QString folder_path = console_query_folder_path(folder_index);
            const QJsonObject folder_data = folder_list[folder_path].toObject();
            
            return folder_data["child_list"].toArray();
        }();

        for (const QJsonValue &path_json : child_list) {
            const QString path = path_json.toString();

            if (folder_list.contains(path)) {
                const QJsonObject folder_data = folder_list[path].toObject();

                const QString name = folder_data["name"].toString();

                auto item = new QStandardItem(name);
                item->setIcon(QIcon::fromTheme("folder"));
                folder_item->appendRow(item);

                query_stack.append(item->index());
            }
        }
    }

    QDialog::open();
}

void MoveQueryDialog::accept() {
    // Search for new parent by getting it's path and then
    // looking for index with same path in console's scope
    // tree
    const QPersistentModelIndex new_parent_index =
    [&]() {
        const QModelIndex query_root_index = console_query_get_root_index(console);
        if (!query_root_index.isValid()) {
            return QPersistentModelIndex();
        }
        
        const QModelIndex selected_index = view->selectionModel()->currentIndex();
        const QString selected_path = console_query_folder_path(selected_index);

        QStack<QModelIndex> stack;
        stack.append(query_root_index);
        while (!stack.isEmpty()) {
            const QModelIndex index = stack.pop();
            const QStandardItem *item = console->get_scope_item(index);
            const QString path = console_query_folder_path(index);

            if (path == selected_path) {
                return QPersistentModelIndex(index);
            }

            for (int row = 0; row < item->rowCount(); row++) {
                const QStandardItem *child = item->child(row);
                const ItemType type = (ItemType) child->data(ConsoleRole_Type).toInt();

                if (type == ItemType_QueryFolder) {
                    stack.append(child->index());
                }
            }
        }

        return QPersistentModelIndex();
    }();

    if (!new_parent_index.isValid()) {
        qDebug() << "Failed to find new parent index in console";
        QDialog::accept();

        return;
    }

    const bool name_conflict =
    [&]() {
        const QList<QPersistentModelIndex> selected_indexes = persistent_index_list(console->get_selected_items());
        for (const QPersistentModelIndex &old_index : selected_indexes) {
            QAbstractItemModel *index_model = (QAbstractItemModel *) old_index.model();
            const QString moved_name = index_model->data(old_index, Qt::DisplayRole).toString();
            if (!console_query_or_folder_name_is_good({moved_name}, new_parent_index, this, old_index)) {
                return true;
            }
        }

        return false;
    }();

    if (name_conflict) {
        return;
    }

    const QList<QPersistentModelIndex> selected_indexes = persistent_index_list(console->get_selected_items());
    console_query_move(console, selected_indexes, new_parent_index);

    QDialog::accept();
}
