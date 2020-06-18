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
        AD(), &AdInterface::dn_changed,
        this, &AdModel::on_dn_changed);
    connect(
        AD(), &AdInterface::create_entry_complete,
        this, &AdModel::on_create_entry_complete);
    connect(
        AD(), &AdInterface::attributes_changed,
        this, &AdModel::on_attributes_changed);
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

void AdModel::on_create_entry_complete(const QString &dn, NewEntryType type) {
    // Load entry to model if it's parent has already been fetched
    QString parent_dn = extract_parent_dn_from_dn(dn);
    QStandardItem *parent = find_first_row_item(parent_dn);

    if (parent != nullptr) {
        const QModelIndex parent_index = parent->index();

        if (!canFetchMore(parent_index)) {
            make_new_row(parent, dn);
        }
    }
}

void AdModel::on_attributes_changed(const QString &dn) {
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

// Update DN column to new DN and move row if necessary
// If row has children, they are moved together with parent row
// Then, when dn_changed() is called on children, they are only renamed
void AdModel::on_dn_changed(const QString &old_dn, const QString &new_dn) {
    const QStandardItem *old_item = find_first_row_item(old_dn);

    // Update DN
    if (old_item != nullptr && new_dn != "") {
        const QModelIndex old_item_index = old_item->index();
        const QModelIndex dn_index = old_item_index.siblingAtColumn(Column::DN);
        QStandardItem *dn_item = itemFromIndex(dn_index);

        dn_item->setText(new_dn);
    }

    // NOTE: row operations do nothing if row index is -1

    const QString old_parent_dn = extract_parent_dn_from_dn(old_dn);
    const QString new_parent_dn = extract_parent_dn_from_dn(new_dn);

    QStandardItem *old_parent = find_first_row_item(old_parent_dn);
    QStandardItem *new_parent = find_first_row_item(new_parent_dn);

    // If parent of row is already new parent, don't need to move row
    // This happens when entry was moved together with it's parent
    // or ancestor
    // Also true if entry was only renamed
    if (old_item->parent() == new_parent) {
        return;
    }

    // NOTE: only add to new parent if it can't fetch
    // if new parent can fetch, then it will load this row when
    // it does fetch along with all other children
    const bool remove_from_old_parent = (old_parent != nullptr);
    const bool add_to_new_parent = (new_parent != nullptr && !canFetchMore(new_parent->index()));

    if (remove_from_old_parent) {
        const int old_row_i = old_item->row();

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

QStandardItem *AdModel::find_first_row_item(const QString &dn) {
    if (dn == "") {
        return nullptr;
    }

    // Find dn item (findItems returns as list)
    const QList<QStandardItem *> dn_items = findItems(dn, Qt::MatchExactly | Qt::MatchRecursive, AdModel::Column::DN);
    if (dn_items.size() == 0) {
        return nullptr;
    }

    // Get first item in row
    const QStandardItem *dn_item = dn_items[0];
    const QModelIndex dn_index = dn_item->index();
    const QModelIndex first_item_index = dn_index.siblingAtColumn(0);
    QStandardItem *first_item = itemFromIndex(first_item_index);

    return first_item;
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

    bool is_container = AD()->is_container_like(dn) || AD()->is_container(dn);

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
