
#include "ad_model.h"
#include "ad_interface.h"
#include "constants.h"

#include <QMap>

QList<QStandardItem*> make_row(QString dn) {
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

    QList<QStandardItem*> row = {
        new QStandardItem(name),
        new QStandardItem(category),
        new QStandardItem(description),
    };

    // NOTE: store special invisible attributes in Roles of first item
    // TODO: shouldn't store these in roles
    row[0]->setData(dn, AdModel::Roles::DN);
    row[0]->setData(advanced_view, AdModel::Roles::AdvancedViewOnly);
    row[0]->setData(true, AdModel::Roles::CanFetch);
    row[0]->setData(is_container, AdModel::Roles::IsContainer);

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

    auto dn = parent.data(Roles::DN).toString();
    // printf("canFetchMore: %s\n", qPrintable(dn));

    auto can_fetch = parent.data(AdModel::Roles::CanFetch).toBool();

    return can_fetch;
}

void AdModel::fetchMore(const QModelIndex &parent) {
    if (!parent.isValid() || !this->canFetchMore(parent)) {
        return;
    }

    auto dn = parent.data(Roles::DN).toString();

    auto parent_item = this->itemFromIndex(parent);

    // Add children
    auto children = load_children(dn);

    for (auto child : children) {
        auto child_row = make_row(child);

        parent_item->appendRow(child_row);
    }

    // Unset CanFetch flag
    parent_item->setData(false, AdModel::Roles::CanFetch);
    
    // printf("fetchMore: %s\n", qPrintable(dn));
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
