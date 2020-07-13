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

#include "containers_widget.h"
#include "advanced_view_proxy.h"
#include "entry_context_menu.h"
#include "dn_column_proxy.h"
#include "entry_model.h"
#include "utils.h"

#include <QTreeView>
#include <QLabel>
#include <QLayout>
#include <QMimeData>
#include <QMap>
#include <QIcon>
#include <QSet>
#include <QStack>

enum ContainersColumn {
    ContainersColumn_Name,
    ContainersColumn_DN,
    ContainersColumn_COUNT,
};

QStandardItem *make_new_row(QStandardItem *parent, const QString &dn);
void load_row(QList<QStandardItem *> row, const QString &dn);

ContainersWidget::ContainersWidget(EntryContextMenu *entry_context_menu, QWidget *parent)
: QWidget(parent)
{
    model = new ContainersModel(this);
    advanced_view_proxy = new AdvancedViewProxy(ContainersColumn_DN, this);
    dn_column_proxy = new DnColumnProxy(ContainersColumn_DN, this);

    view = new QTreeView(this);
    view->setAcceptDrops(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setRootIsDecorated(true);
    view->setItemsExpandable(true);
    view->setExpandsOnDoubleClick(true);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setDragDropMode(QAbstractItemView::DragDrop);
    view->setAllColumnsShowFocus(true);
    entry_context_menu->connect_view(view, ContainersColumn_DN);

    setup_model_chain(view, model, {advanced_view_proxy, dn_column_proxy});

    // Insert label into layout
    const auto label = new QLabel("Containers");

    const auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(label);
    layout->addWidget(view);

    connect(
        view->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &ContainersWidget::on_selection_changed);

    connect(
        view, &QAbstractItemView::clicked,
        [this] (const QModelIndex &index) {
            const QString dn = get_dn_from_index(index, ContainersColumn_DN);

            emit clicked_dn(dn);
        });

    connect(
        AD(), &AdInterface::modified,
        this, &ContainersWidget::on_ad_modified);
};

void ContainersWidget::on_ad_modified() {
    const QString head_dn = AD()->get_search_base();

    // Save fetch state
    QSet<QString> was_fetched;
    {
        QModelIndex head = model->index(0, 0);

        QStack<QModelIndex> stack;
        stack.push(head);

        while (!stack.isEmpty()) {
            const QModelIndex index = stack.pop();

            if (!model->canFetchMore(index)) {
                const QString dn = get_dn_from_index(index, ContainersColumn_DN);
                was_fetched.insert(dn);

                // Add children of index
                int row = 0;
                while (true) {
                    const QModelIndex child = model->index(row, 0, index);

                    if (child.isValid()) {
                        stack.push(child);
                        row++;
                    } else {
                        break;
                    }
                }
            }
        }
    }

    // Save expand state
    QSet<QString> was_expanded;
    {
        QAbstractItemModel *v_model = view->model();
        QModelIndex head = v_model->index(0, 0);
        QStack<QModelIndex> stack;
        stack.push(head);

        while (!stack.isEmpty()) {
            QModelIndex index = stack.pop();

            if (view->isExpanded(index)) {
                const QString dn = get_dn_from_index(index, ContainersColumn_DN);
                was_expanded.insert(dn);

                int row = 0;
                while (true) {
                    QModelIndex child = v_model->index(row, 0, index);

                    if (child.isValid()) {
                        stack.push(child);
                        row++;
                    } else {
                        view->expand(index);
                        break;
                    }
                }
            }
        }
    }

    // Re-fetch
    {
        model->removeRows(0, model->rowCount());

        QStack<QModelIndex> to_fetch;

        QStandardItem *invis_root = model->invisibleRootItem();
        const QStandardItem *head_item = make_new_row(invis_root, head_dn);
        to_fetch.push(head_item->index());

        while (!to_fetch.isEmpty()) {
            QModelIndex index = to_fetch.pop();
            const QString dn = get_dn_from_index(index, ContainersColumn_DN);
            const bool fetch = was_fetched.contains(dn);

            if (fetch && model->canFetchMore(index)) {
                model->fetchMore(index);

                int row = 0;
                while (true) {
                    QModelIndex child = model->index(row, 0, index);

                    if (child.isValid()) {
                        to_fetch.push(child);
                        row++;
                    } else {
                        view->expand(index);
                        break;
                    }
                }
            }
        }
    }

    // Re-expand
    QAbstractItemModel *v_model = view->model();
    QModelIndex head = v_model->index(0, 0);
    QStack<QModelIndex> to_expand;
    to_expand.push(head);

    while (!to_expand.isEmpty()) {
        QModelIndex index = to_expand.pop();
        const QString dn = get_dn_from_index(index, ContainersColumn_DN);
        const bool expand = was_expanded.contains(dn);

        if (expand) {
            view->expand(index);

            int row = 0;
            while (true) {
                QModelIndex child = v_model->index(row, 0, index);

                if (child.isValid()) {
                    to_expand.push(child);
                    row++;
                } else {
                    view->expand(index);
                    break;
                }
            }
        }
    }
}

void ContainersWidget::on_selection_changed(const QItemSelection &selected, const QItemSelection &) {
    // Transform selected index into source index and pass it on
    // to selected_container_changed() signal
    const QList<QModelIndex> indexes = selected.indexes();

    if (indexes.isEmpty()) {
        return;
    }

    const QModelIndex index = convert_to_source(indexes[0]);

    QModelIndex dn_index = index.siblingAtColumn(ContainersColumn_DN);
    QString dn = dn_index.data().toString();

    emit selected_changed(dn);
}

ContainersModel::ContainersModel(QObject *parent)
: EntryModel(ContainersColumn_COUNT, ContainersColumn_DN, parent)
{
    setHorizontalHeaderItem(ContainersColumn_Name, new QStandardItem("Name"));
    setHorizontalHeaderItem(ContainersColumn_DN, new QStandardItem("DN"));

    connect(
        AD(), &AdInterface::logged_in,
        this, &ContainersModel::on_logged_in);
    connect(
        AD(), &AdInterface::modified,
        this, &ContainersModel::on_ad_modified);
}

bool ContainersModel::canFetchMore(const QModelIndex &parent) const {
    if (!parent.isValid()) {
        return false;
    }

    bool can_fetch = parent.data(ContainersModel::Roles::CanFetch).toBool();

    return can_fetch;
}

void ContainersModel::fetchMore(const QModelIndex &parent) {
    if (!parent.isValid() || !canFetchMore(parent)) {
        return;
    }

    QString dn = get_dn_from_index(parent, ContainersColumn_DN);

    QStandardItem *parent_item = itemFromIndex(parent);

    // Add children
    QList<QString> children = AD()->list(dn);

    for (auto child : children) {
        make_new_row(parent_item, child);
    }

    // Unset CanFetch flag
    parent_item->setData(false, ContainersModel::Roles::CanFetch);
}

// Override this so that unexpanded and unfetched items show the expander even though they technically don't have any children loaded
// NOTE: expander is show if hasChildren returns true
bool ContainersModel::hasChildren(const QModelIndex &parent = QModelIndex()) const {
    if (canFetchMore(parent)) {
        return true;
    } else {
        return QStandardItemModel::hasChildren(parent);
    }
}

void ContainersModel::on_logged_in() {
    removeRows(0, rowCount());

    // Load head
    const QString head_dn = AD()->get_search_base();
    QStandardItem *invis_root = invisibleRootItem();
    make_new_row(invis_root, head_dn);
}

void ContainersModel::on_ad_modified() {
    // removeRows(0, rowCount());

    // const QString head_dn = AD()->get_search_base();

    // // Load head
    // QStandardItem *invis_root = invisibleRootItem();
    // make_new_row(invis_root, head_dn);
}

void load_row(QList<QStandardItem *> row, const QString &dn) {
    QString name = AD()->attribute_get(dn, "name");

    row[ContainersColumn_Name]->setText(name);
    row[ContainersColumn_DN]->setText(dn);

    QIcon icon = get_entry_icon(dn);
    row[0]->setIcon(icon);

    // Set fetch flag because row is new and can be fetched
    row[0]->setData(true, ContainersModel::Roles::CanFetch);
}

// Make new row in model at given parent based on entry with given dn
QStandardItem *make_new_row(QStandardItem *parent, const QString &dn) {
    const bool is_container = AD()->is_container(dn);
    const bool is_container_like = AD()->is_container_like(dn);
    const bool should_be_loaded = is_container || is_container_like;

    if (!should_be_loaded) {
        return nullptr;
    }

    auto row = QList<QStandardItem *>();
    for (int i = 0; i < ContainersColumn_COUNT; i++) {
        row.push_back(new QStandardItem());
    }

    load_row(row, dn);

    parent->appendRow(row);

    return row[0];
}
