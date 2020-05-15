
#include "ad_model.h"
#include "ad_interface.h"
#include "constants.h"

#include <QMap>

void populate_row(QList<QStandardItem*> row, QString &dn) {
    auto attributes = load_attributes(dn);

    // TODO: get rid of "if (x.contains(y))"
    
    // Name
    auto name = dn;
    if (attributes.contains("name")) {
        name = attributes["name"][0];
    }

    // Category
    auto category = dn;
    if (attributes.contains("objectCategory")) {
        // NOTE: raw category is given as DN
        // TODO: convert it completely (turn '-' into ' ')
        auto category_as_dn = attributes["objectCategory"][0];
        auto equals_index = category_as_dn.indexOf('=');
        auto comma_index = category_as_dn.indexOf(',');
        auto segment_length = comma_index - equals_index;
        // TODO: check what happens if equals is negative
        category = category_as_dn.mid(equals_index + 1, segment_length);
    }

    // Description
    QString description = "none";
    if (attributes.contains("description")) {
        description = attributes["description"][0];
    }

    // showInAdvancedViewOnly
    bool advanced_view = false;
    if (attributes.contains("showInAdvancedViewOnly")) {
        auto advanced_view_str = attributes["showInAdvancedViewOnly"][0];

        if (advanced_view_str == "TRUE") {
            advanced_view = true;

        }
    }

    // is container
    bool is_container = false;
    if (attributes.contains("objectClass")) {
        auto objectClasses = attributes["objectClass"];
        
        QList<QString> container_objectClasses = {"container", "organizationalUnit", "builtinDomain", "domain"};
        for (auto e : container_objectClasses) {
            if (objectClasses.contains(e)) {
                is_container = true;
                break;
            }
        }
    }

    auto name_item = row[AdModel::Column::Name];
    auto category_item = row[AdModel::Column::Category];
    auto description_item = row[AdModel::Column::Description];
    auto dn_item = row[AdModel::Column::DN];

    name_item->setText(name);
    category_item->setText(category);
    description_item->setText(description);
    dn_item->setText(dn);

    // NOTE: store special invisible attributes in Roles of first item
    // TODO: shouldn't store these in roles
    row[0]->setData(advanced_view, AdModel::Roles::AdvancedViewOnly);
    row[0]->setData(true, AdModel::Roles::CanFetch);
    row[0]->setData(is_container, AdModel::Roles::IsContainer);
}

QList<QStandardItem *> make_row(QString dn) {
    auto row = QList<QStandardItem *>();

    for (int i = 0; i < AdModel::Column::COUNT; i++) {
        row.push_back(new QStandardItem());
    }

    populate_row(row, dn);

    return row;
}

AdModel::AdModel(): QStandardItemModel(0, Column::COUNT) {
    this->setHorizontalHeaderItem(Column::Name, new QStandardItem("Name"));
    this->setHorizontalHeaderItem(Column::Category, new QStandardItem("Category"));
    this->setHorizontalHeaderItem(Column::Description, new QStandardItem("Description"));

    // Load head
    auto invis_root = this->invisibleRootItem();
    auto head_row = make_row(HEAD_DN);
    invis_root->appendRow(head_row);
}

bool AdModel::canFetchMore(const QModelIndex &parent) const {
    if (!parent.isValid()) {
        return false;
    }

    auto can_fetch = parent.data(AdModel::Roles::CanFetch).toBool();

    return can_fetch;
}

void AdModel::fetchMore(const QModelIndex &parent) {
    if (!parent.isValid() || !this->canFetchMore(parent)) {
        return;
    }

    auto dn_index = parent.siblingAtColumn(Column::DN);
    auto dn = dn_index.data().toString();

    auto parent_item = this->itemFromIndex(parent);

    // Add children
    auto children = load_children(dn);

    for (auto child : children) {
        auto child_row = make_row(child);

        parent_item->appendRow(child_row);
    }

    // Unset CanFetch flag
    parent_item->setData(false, AdModel::Roles::CanFetch);
}

// Override this so that unexpanded and unfetched items show the expander even though they technically don't have any children loaded
// NOTE: expander is show if hasChildren returns true
bool AdModel::hasChildren(const QModelIndex &parent = QModelIndex()) const {
    if (this->canFetchMore(parent)) {
        return true;
    } else {
        return QStandardItemModel::hasChildren(parent);
    }
}

void AdModel::on_entry_changed(QString &dn) {
    // TODO: confirm what kind of search is this, linear?
    auto items = this->findItems(dn, Qt::MatchExactly | Qt::MatchRecursive, AdModel::Column::DN);

    // TODO: not sure if any bad matches can happen, maybe?
    if (items.size() > 0) {
        auto dn_item = items[0];
        auto dn_index = dn_item->index();

        // Compose indexes for all columns
        auto indexes = QList<QModelIndex>(); 
        for (int i = 0; i < AdModel::Column::COUNT; i++) {
            auto index = dn_index.siblingAtColumn(i);
            indexes.push_back(index);
        }

        // Compose the row of items from indexes
        auto row = QList<QStandardItem *>();
        for (int i = 0; i < AdModel::Column::COUNT; i++) {
            auto index = indexes[i];
            auto item = this->itemFromIndex(index);
            row.push_back(item);
        }

        // Repopulate row
        populate_row(row, dn);
    }
}
