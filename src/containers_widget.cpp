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
#include "entry_proxy_model.h"

#include <QTreeView>
#include <QLabel>
#include <QLayout>
#include <QMimeData>
#include <QMap>
#include <QIcon>

void make_new_row(QStandardItem *parent, const QString &dn);
void load_row(QList<QStandardItem *> row, const QString &dn);

ContainersWidget::ContainersWidget(AdModel *model_arg, QWidget *parent)
: EntryWidget(model_arg, parent)
{
    model = model_arg;
    model->setHorizontalHeaderItem(AdModel::Column::Name, new QStandardItem("Name"));
    model->setHorizontalHeaderItem(AdModel::Column::DN, new QStandardItem("DN"));

    proxy = new EntryProxyModel(model, this);
    proxy->only_show_containers = true;

    view->setAcceptDrops(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setRootIsDecorated(true);
    view->setItemsExpandable(true);
    view->setExpandsOnDoubleClick(true);
    view->setModel(proxy);

    column_hidden[AdModel::Column::Name] = false;
    update_column_visibility();

    // Insert label into layout
    const auto label = new QLabel("Containers");
    layout()->removeWidget(view);
    layout()->addWidget(label);
    layout()->addWidget(view);

    connect(
        view->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &ContainersWidget::on_selection_changed);

    connect(
        AD(), &AdInterface::ad_interface_login_complete,
        this, &ContainersWidget::on_ad_interface_login_complete);
    connect(
        AD(), &AdInterface::delete_entry_complete,
        this, &ContainersWidget::on_delete_entry_complete);
    connect(
        AD(), &AdInterface::dn_changed,
        this, &ContainersWidget::on_dn_changed);
    connect(
        AD(), &AdInterface::create_entry_complete,
        this, &ContainersWidget::on_create_entry_complete);
    connect(
        AD(), &AdInterface::attributes_changed,
        this, &ContainersWidget::on_attributes_changed);
};

void ContainersWidget::on_selection_changed(const QItemSelection &selected, const QItemSelection &) {
    // Transform selected index into source index and pass it on
    // to selected_container_changed() signal
    const QList<QModelIndex> indexes = selected.indexes();

    if (indexes.size() > 0) {
        QModelIndex index = indexes[0];
        QModelIndex dn_index = index.siblingAtColumn(AdModel::Column::DN);
        QString dn = dn_index.data().toString();

        emit selected_changed(dn);
    }
}

AdModel::AdModel(QObject *parent)
: EntryModel(Column::COUNT, AdModel::Column::DN, parent)
{
    
}

bool AdModel::canFetchMore(const QModelIndex &parent) const {
    if (!parent.isValid()) {
        return false;
    }

    bool can_fetch = parent.data(AdModel::Roles::CanFetch).toBool();

    return can_fetch;
}

void AdModel::fetchMore(const QModelIndex &parent) {
    if (!parent.isValid() || !canFetchMore(parent)) {
        return;
    }

    QString dn = get_dn_from_index(parent);

    QStandardItem *parent_item = itemFromIndex(parent);

    // Add children
    QList<QString> children = AD()->load_children(dn);

    for (auto child : children) {
        make_new_row(parent_item, child);
    }

    // Unset CanFetch flag
    parent_item->setData(false, AdModel::Roles::CanFetch);
}

// Override this so that unexpanded and unfetched items show the expander even though they technically don't have any children loaded
// NOTE: expander is show if hasChildren returns true
bool AdModel::hasChildren(const QModelIndex &parent = QModelIndex()) const {
    if (canFetchMore(parent)) {
        return true;
    } else {
        return QStandardItemModel::hasChildren(parent);
    }
}

void ContainersWidget::on_ad_interface_login_complete(const QString &search_base, const QString &head_dn) {
    model->removeRows(0, model->rowCount());

    // Load head
    QStandardItem *invis_root = model->invisibleRootItem();
    make_new_row(invis_root, head_dn);
}

void ContainersWidget::on_delete_entry_complete(const QString &dn) {
    QList<QStandardItem *> items = model->findItems(dn, Qt::MatchExactly | Qt::MatchRecursive, AdModel::Column::DN);

    if (items.size() > 0) {
        QStandardItem *dn_item = items[0];
        QModelIndex dn_index = dn_item->index();
        
        model->removeRow(dn_index.row(), dn_index.parent());
    }
}

void ContainersWidget::on_dn_changed(const QString &old_dn, const QString &new_dn) {
    // Remove old entry from model
    QList<QStandardItem *> old_items = model->findItems(old_dn, Qt::MatchExactly | Qt::MatchRecursive, AdModel::Column::DN);
    if (old_items.size() > 0) {
        QStandardItem *dn_item = old_items[0];
        QModelIndex dn_index = dn_item->index();
        
        model->removeRow(dn_index.row(), dn_index.parent());
    }

    // Need to load entry at new parent if the parent has already
    // been expanded/fetched
    // NOTE: loading if parent hasn't been fetched will
    // create a duplicate
    const QString new_parent = extract_parent_dn_from_dn(new_dn);
    QList<QStandardItem *> parent_items = model->findItems(new_parent, Qt::MatchExactly | Qt::MatchRecursive, AdModel::Column::DN);
    if (parent_items.size() > 0) {
        QStandardItem *parent_dn_item = parent_items[0];
        QModelIndex parent_dn_index = parent_dn_item->index();
        QModelIndex parent_index = parent_dn_index.siblingAtColumn(AdModel::Column::Name);

        QStandardItem *parent_item = model->itemFromIndex(parent_index);

        if (!model->canFetchMore(parent_index)) {
            make_new_row(parent_item, new_dn);
        }
    }
}

void ContainersWidget::on_create_entry_complete(const QString &dn, NewEntryType type) {
    // Load entry to model if it's parent has already been fetched
    // If it hasn't been fetched, then this new entry will be loaded with all other children when the parent is fetched
    QString parent_dn = extract_parent_dn_from_dn(dn);
    QList<QStandardItem *> items = model->findItems(parent_dn, Qt::MatchExactly | Qt::MatchRecursive, AdModel::Column::DN);

    if (items.size() > 0) {
        QStandardItem *dn_item = items[0];
        QModelIndex dn_index = dn_item->index();
        QModelIndex parent_index = dn_index.siblingAtColumn(0);
        QStandardItem *parent = model->itemFromIndex(parent_index);

        bool fetched_already = !model->canFetchMore(parent_index);
        if (fetched_already) {
            make_new_row(parent, dn);
        }
    }
}

void ContainersWidget::on_attributes_changed(const QString &dn) {
    // Compose row based on dn
    QList<QStandardItem *> items = model->findItems(dn, Qt::MatchExactly | Qt::MatchRecursive, AdModel::Column::DN);

    if (items.size() == 0) {
        return;
    }

    QStandardItem *dn_item = items[0];
    QModelIndex dn_index = dn_item->index();

    // Compose indexes for all columns
    auto indexes = QList<QModelIndex>(); 
    for (int i = 0; i < AdModel::Column::COUNT; i++) {
        QModelIndex index = dn_index.siblingAtColumn(i);
        indexes.push_back(index);
    }

    // Compose the row of items from indexes
    auto row = QList<QStandardItem *>();
    for (int i = 0; i < AdModel::Column::COUNT; i++) {
        QModelIndex index = indexes[i];
        QStandardItem *item = model->itemFromIndex(index);
        row.push_back(item);
    }

    load_row(row, dn);
}

// Load data into row of items based on entry attributes
void load_row(QList<QStandardItem *> row, const QString &dn) {
    QString name = AD()->get_attribute(dn, "name");

    row[AdModel::Column::Name]->setText(name);
    row[AdModel::Column::DN]->setText(dn);

    QIcon icon = get_entry_icon(dn);
    row[0]->setIcon(icon);
}

// Make new row in model at given parent based on entry with given dn
void make_new_row(QStandardItem *parent, const QString &dn) {
    auto row = QList<QStandardItem *>();

    for (int i = 0; i < AdModel::Column::COUNT; i++) {
        row.push_back(new QStandardItem());
    }

    // Set fetch flag because row is new and can be fetched
    row[0]->setData(true, AdModel::Roles::CanFetch);

    // Load dn, so the row can be searched for later
    QStandardItem *dn_item = row[AdModel::Column::DN];
    dn_item->setText(dn);

    parent->appendRow(row);

    load_row(row, dn);
}

QIcon get_entry_icon(const QString &dn) {
    // TODO: change to custom, good icons, add those icons to installation?
    // TODO: are there cases where an entry can have multiple icons due to multiple objectClasses and one of them needs to be prioritized?
    QMap<QString, QString> class_to_icon = {
        {"groupPolicyContainer", "x-office-address-book"},
        {"container", "folder"},
        {"organizationalUnit", "network-workgroup"},
        {"person", "avatar-default"},
        {"group", "application-x-smb-workgroup"},
        {"builtinDomain", "emblem-system"},
    };
    QString icon_name = "dialog-question";
    for (auto c : class_to_icon.keys()) {
        if (AD()->attribute_value_exists(dn, "objectClass", c)) {
            icon_name = class_to_icon[c];
            break;  
        }
    }

    QIcon icon = QIcon::fromTheme(icon_name);

    return icon;
}
