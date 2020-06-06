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

#include "ad_model.h"
#include "ad_interface.h"

#include <QMimeData>
#include <QMap>
#include <QIcon>

void make_new_row(QStandardItem *parent, const QString &dn);
void load_row(QList<QStandardItem *> row, const QString &dn);

AdModel::AdModel(QObject *parent)
: EntryModel(Column::COUNT, Column::DN, parent)
{
    setHorizontalHeaderItem(Column::Name, new QStandardItem("Name"));
    setHorizontalHeaderItem(Column::Category, new QStandardItem("Category"));
    setHorizontalHeaderItem(Column::Description, new QStandardItem("Description"));
    setHorizontalHeaderItem(Column::DN, new QStandardItem("DN"));
    
    connect(
        AD(), &AdInterface::ad_interface_login_complete,
        this, &AdModel::on_ad_interface_login_complete);
    connect(
        AD(), &AdInterface::delete_entry_complete,
        this, &AdModel::on_delete_entry_complete);
    connect(
        AD(), &AdInterface::move_user_complete,
        this, &AdModel::on_move_user_complete);
    connect(
        AD(), &AdInterface::create_entry_complete,
        this, &AdModel::on_create_entry_complete);
    connect(
        AD(), &AdInterface::load_attributes_complete,
        this, &AdModel::on_load_attributes_complete);
    connect(
        AD(), &AdInterface::rename_complete,
        this, &AdModel::on_rename_complete);
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

void AdModel::on_ad_interface_login_complete(const QString &search_base, const QString &head_dn) {
    removeRows(0, rowCount());

    // Load head
    QStandardItem *invis_root = invisibleRootItem();
    make_new_row(invis_root, head_dn);
}

void AdModel::on_delete_entry_complete(const QString &dn) {
    QList<QStandardItem *> items = findItems(dn, Qt::MatchExactly | Qt::MatchRecursive, AdModel::Column::DN);

    if (items.size() > 0) {
        QStandardItem *dn_item = items[0];
        QModelIndex dn_index = dn_item->index();
        
        removeRow(dn_index.row(), dn_index.parent());
    }
}

void AdModel::on_move_user_complete(const QString &user_dn, const QString &container_dn, const QString &new_dn) {
    // Remove old entry from model
    QList<QStandardItem *> old_items = findItems(user_dn, Qt::MatchExactly | Qt::MatchRecursive, AdModel::Column::DN);
    if (old_items.size() > 0) {
        QStandardItem *dn_item = old_items[0];
        QModelIndex dn_index = dn_item->index();
        
        removeRow(dn_index.row(), dn_index.parent());
    }

    // Need to load entry at new parent if the parent has already
    // been expanded/fetched
    // NOTE: loading if parent has already been fetched will
    // create a duplicate
    QList<QStandardItem *> parent_items = findItems(container_dn, Qt::MatchExactly | Qt::MatchRecursive, AdModel::Column::DN);
    if (parent_items.size() > 0) {
        QStandardItem *parent_dn_item = parent_items[0];
        QModelIndex parent_dn_index = parent_dn_item->index();
        QModelIndex parent_index = parent_dn_index.siblingAtColumn(Column::Name);

        QStandardItem *parent_item = itemFromIndex(parent_index);

        if (!canFetchMore(parent_index)) {
            make_new_row(parent_item, new_dn);
        }
    }
}

void AdModel::on_create_entry_complete(const QString &dn, NewEntryType type) {
    // Load entry to model if it's parent has already been fetched
    // If it hasn't been fetched, then this new entry will be loaded with all other children when the parent is fetched
    QString parent_dn = extract_parent_dn_from_dn(dn);
    QList<QStandardItem *> items = findItems(parent_dn, Qt::MatchExactly | Qt::MatchRecursive, Column::DN);

    if (items.size() > 0) {
        QStandardItem *dn_item = items[0];
        QModelIndex dn_index = dn_item->index();
        QModelIndex parent_index = dn_index.siblingAtColumn(0);
        QStandardItem *parent = itemFromIndex(parent_index);

        bool fetched_already = !canFetchMore(parent_index);
        if (fetched_already) {
            make_new_row(parent, dn);
        }
    }
}

void AdModel::on_load_attributes_complete(const QString &dn) {
    // Compose row based on dn
    QList<QStandardItem *> items = findItems(dn, Qt::MatchExactly | Qt::MatchRecursive, AdModel::Column::DN);

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
        QStandardItem *item = itemFromIndex(index);
        row.push_back(item);
    }

    load_row(row, dn);
}

void AdModel::on_rename_complete(const QString &dn, const QString &new_name, const QString &new_dn) {
    // Find row still attached to old dn
    QList<QStandardItem *> items = findItems(dn, Qt::MatchExactly | Qt::MatchRecursive, AdModel::Column::DN);

    if (items.size() == 0) {
        return;
    }
    
    // Change dn of row to new dn
    QStandardItem *dn_item = items[0];
    dn_item->setText(new_dn);

    // Reload attributes
    on_load_attributes_complete(new_dn);
}

// Load data into row of items based on entry attributes
void load_row(QList<QStandardItem *> row, const QString &dn) {
    // Load row based on attributes
    QString name = AD()->get_attribute(dn, "name");

    // NOTE: this is given as raw DN and contains '-' where it should
    // have spaces, so convert it
    QString category = AD()->get_attribute(dn, "objectCategory");
    category = extract_name_from_dn(category);
    category = category.replace('-', ' ');

    QString description = AD()->get_attribute(dn, "description");

    bool showInAdvancedViewOnly = AD()->get_attribute(dn, "showInAdvancedViewOnly") == "TRUE";

    bool is_container = false;
    const QList<QString> container_objectClasses = {"container", "organizationalUnit", "builtinDomain", "domain"};
    for (auto c : container_objectClasses) {
        if (AD()->attribute_value_exists(dn, "objectClass", c)) {
            is_container = true;
            break;
        }
    }

    QStandardItem *name_item = row[AdModel::Column::Name];
    QStandardItem *category_item = row[AdModel::Column::Category];
    QStandardItem *description_item = row[AdModel::Column::Description];
    QStandardItem *dn_item = row[AdModel::Column::DN];

    name_item->setText(name);
    category_item->setText(category);
    description_item->setText(description);
    dn_item->setText(dn);
    row[0]->setData(showInAdvancedViewOnly, AdModel::Roles::AdvancedViewOnly);
    row[0]->setData(is_container, AdModel::Roles::IsContainer);

    // Set icon
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
