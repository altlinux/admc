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

void make_new_row(QStandardItem *parent, const QString &dn);
void load_row(QList<QStandardItem *> row, const QString &dn);

ContainersWidget::ContainersWidget(EntryContextMenu *entry_context_menu, QWidget *parent)
: QWidget(parent)
{
    model = new ContainersModel(this);
    const auto advanced_view_proxy = new AdvancedViewProxy(ContainersModel::Column::DN, this);
    const auto dn_column_proxy = new DnColumnProxy(ContainersModel::Column::DN, this);

    view = new QTreeView(this);
    view->setAcceptDrops(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setRootIsDecorated(true);
    view->setItemsExpandable(true);
    view->setExpandsOnDoubleClick(true);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setDragDropMode(QAbstractItemView::DragDrop);
    entry_context_menu->connect_view(view, ContainersModel::Column::DN);

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
            const QString dn = get_dn_from_index(index, ContainersModel::Column::DN);

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

    QModelIndex dn_index = index.siblingAtColumn(ContainersModel::Column::DN);
    QString dn = dn_index.data().toString();

    emit selected_changed(dn);
}

ContainersModel::ContainersModel(QObject *parent)
: EntryModel(Column::COUNT, Column::DN, parent)
{
    setHorizontalHeaderItem(Column::Name, new QStandardItem("Name"));
    setHorizontalHeaderItem(Column::DN, new QStandardItem("DN"));

    connect(
        AD(), &AdInterface::ad_interface_login_complete,
        this, &ContainersModel::on_ad_interface_login_complete);
    connect(
        AD(), &AdInterface::dn_changed,
        this, &ContainersModel::on_dn_changed);
    connect(
        AD(), &AdInterface::create_entry_complete,
        this, &ContainersModel::on_create_entry_complete);
    connect(
        AD(), &AdInterface::attributes_changed,
        this, &ContainersModel::on_attributes_changed);
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

    QString dn = get_dn_from_index(parent, Column::DN);

    QStandardItem *parent_item = itemFromIndex(parent);

    // Add children
    QList<QString> children = AD()->load_children(dn);

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

void ContainersModel::on_ad_interface_login_complete(const QString &search_base, const QString &head_dn) {
    removeRows(0, rowCount());

    // Load head
    QStandardItem *invis_root = invisibleRootItem();
    make_new_row(invis_root, head_dn);
}

void ContainersModel::on_dn_changed(const QString &old_dn, const QString &new_dn) {
    QStandardItem *old_dn_item = find_item(old_dn, ContainersModel::Column::DN);

    // Update DN
    if (old_dn_item != nullptr && new_dn != "") {
        old_dn_item->setText(new_dn);
    }

    const QString old_parent_dn = extract_parent_dn_from_dn(old_dn);
    const QString new_parent_dn = extract_parent_dn_from_dn(new_dn);

    QStandardItem *old_parent = find_item(old_parent_dn, 0);
    QStandardItem *new_parent = find_item(new_parent_dn, 0);

    // If parent of row is already new parent, don't need to move row
    // This happens when entry was moved together with it's parent
    // or ancestor
    // Also true if entry was only renamed
    if (old_dn_item->parent() == new_parent) {
        return;
    }

    // NOTE: only add to new parent if it can't fetch
    // if new parent can fetch, then it will load this row when
    // it does fetch along with all other children
    const bool remove_from_old_parent = (old_parent != nullptr);
    const bool add_to_new_parent = (new_parent != nullptr && !canFetchMore(new_parent->index()));

    if (remove_from_old_parent) {
        const int old_row_i = old_dn_item->row();

        if (add_to_new_parent) {
            // Transfer row from old to new parent
            const QList<QStandardItem *> row = old_parent->takeRow(old_row_i);
            new_parent->appendRow(row);
        } else {
            old_parent->removeRow(old_row_i);
        }
    } else {
        if (add_to_new_parent) {
            make_new_row(new_parent, new_dn);
        }
    }
}

void ContainersModel::on_create_entry_complete(const QString &dn, NewEntryType type) {
    // Load entry to model if it's parent has already been fetched
    QString parent_dn = extract_parent_dn_from_dn(dn);
    QStandardItem *parent = find_item(parent_dn, 0);

    if (parent != nullptr) {
        const QModelIndex parent_index = parent->index();

        if (!canFetchMore(parent_index)) {
            make_new_row(parent, dn);
        }
    }
}

void ContainersModel::on_attributes_changed(const QString &dn) {
    QList<QStandardItem *> row = find_row(dn);

    if (!row.isEmpty()) {
        load_row(row, dn);
    }
}

void load_row(QList<QStandardItem *> row, const QString &dn) {
    QString name = AD()->get_attribute(dn, "name");

    row[ContainersModel::Column::Name]->setText(name);
    row[ContainersModel::Column::DN]->setText(dn);

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
    for (int i = 0; i < ContainersModel::Column::COUNT; i++) {
        row.push_back(new QStandardItem());
    }

    load_row(row, dn);

    parent->appendRow(row);
}
