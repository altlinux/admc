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

enum ContainersColumn {
    ContainersColumn_Name,
    ContainersColumn_DN,
    ContainersColumn_COUNT,
};

void make_new_row(QStandardItem *parent, const QString &dn);
void load_row(QList<QStandardItem *> row, const QString &dn);

ContainersWidget::ContainersWidget(EntryContextMenu *entry_context_menu, QWidget *parent)
: QWidget(parent)
{
    model = new ContainersModel(this);
    const auto advanced_view_proxy = new AdvancedViewProxy(ContainersColumn_DN, this);
    const auto dn_column_proxy = new DnColumnProxy(ContainersColumn_DN, this);

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
};

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
    removeRows(0, rowCount());

    const QString head_dn = AD()->get_search_base();

    // Load head
    QStandardItem *invis_root = invisibleRootItem();
    make_new_row(invis_root, head_dn);
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
void make_new_row(QStandardItem *parent, const QString &dn) {
    const bool is_container = AD()->is_container(dn);
    const bool is_container_like = AD()->is_container_like(dn);
    const bool should_be_loaded = is_container || is_container_like;

    if (!should_be_loaded) {
        return;
    }

    auto row = QList<QStandardItem *>();
    for (int i = 0; i < ContainersColumn_COUNT; i++) {
        row.push_back(new QStandardItem());
    }

    load_row(row, dn);

    parent->appendRow(row);
}
