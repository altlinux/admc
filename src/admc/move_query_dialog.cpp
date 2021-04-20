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
#include "console_types/query.h"

#include <QTreeView>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QStack>
#include <QDebug>

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

// TODO: duplicating query_tree_init()
void MoveQueryDialog::open() {
    auto path_to_name =
    [](const QString &path) {
        const int separator_i = path.lastIndexOf("/");
        const QString name = path.mid(separator_i + 1);

        return name;
    };

    model->removeRows(0, model->rowCount());

    QStandardItem *head_item = new QStandardItem();
    head_item->setText(tr("Saved Queries"));
    head_item->setIcon(QIcon::fromTheme("folder"));
    head_item->setData(ItemType_QueryRoot, ConsoleRole_Type);
    model->appendRow(head_item);

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

            if (!is_query_item) {
                const QString name = path_to_name(child_path);

                auto item = new QStandardItem(name);
                auto parent_item = model->itemFromIndex(index);
                parent_item->appendRow(item);

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
    const QModelIndex new_parent_index =
    [&]() {
        const QModelIndex query_root_index = get_query_root_index(console);
        if (!query_root_index.isValid()) {
            return QModelIndex();
        }
        
        const QModelIndex selected_index = view->selectionModel()->currentIndex();
        const QString selected_path = query_folder_path(selected_index);

        QStack<QModelIndex> stack;
        stack.append(query_root_index);
        while (!stack.isEmpty()) {
            const QModelIndex index = stack.pop();
            const QStandardItem *item = console->get_scope_item(index);
            const QString path = query_folder_path(index);

            if (path == selected_path) {
                return index;
            }

            for (int row = 0; row < item->rowCount(); row++) {
                const QStandardItem *child = item->child(row);
                const ItemType type = (ItemType) child->data(ConsoleRole_Type).toInt();

                if (type == ItemType_QueryFolder) {
                    stack.append(child->index());
                }
            }
        }

        return QModelIndex();
    }();

    if (!new_parent_index.isValid()) {
        qDebug() << "Failed to find new parent index in console";
        QDialog::accept();

        return;
    }

    const QModelIndex old_index = get_selected_scope_index(console);
    
    // Check that moved item doesn't have a name conflict at
    // new location
    QStandardItem *old_item = console->get_scope_item(old_index);
    const QString moved_name = old_item->text();
    if (!query_name_is_good(moved_name, new_parent_index, this, old_index)) {
        return;
    }

    // Create a copy of the tree branch at new location. Go
    // down the branch and replicate all of the children.

    QHash<QModelIndex, QModelIndex> old_to_new_index;
    old_to_new_index[old_index.parent()] = new_parent_index;

    QStack<QModelIndex> stack;
    stack.append(old_index);
    while (!stack.isEmpty()) {
        const QModelIndex index = stack.pop();
        QStandardItem *item = console->get_scope_item(index);

        const QModelIndex new_parent = old_to_new_index[item->parent()->index()];

        const ItemType type = (ItemType) item->data(ConsoleRole_Type).toInt();
        if (type == ItemType_QueryItem) {
            const QString description = item->data(QueryItemRole_Description).toString();
            const QString filter = item->data(QueryItemRole_Filter).toString();
            const QString search_base = item->data(QueryItemRole_SearchBase).toString();
            const QString name = item->text();

            query_item_create(console, name, description, filter, search_base, new_parent);
        } else if (type == ItemType_QueryFolder) {
            const QString name = item->text();
            const QString description = item->data(QueryItemRole_Description).toString();
            query_folder_create(console, name, description, new_parent);
        }

        for (int row = 0; row < item->rowCount(); row++) {
            QStandardItem *child = item->child(row);
            stack.append(child->index());
        }
    }

    // Delete branch at old location
    console->delete_item(old_index);

    console->sort_scope();

    query_tree_save(console);

    QDialog::accept();
}
