
#include "ad_model.h"
#include "ad_interface.h"
#include "constants.h"

#include <QMimeData>
#include <QMap>
#include <QIcon>

QString get_dn_of_index(const QModelIndex &index) {
    QModelIndex dn_index = index.siblingAtColumn(AdModel::Column::DN);
    QString dn = dn_index.data().toString();

    return dn;
}

QMimeData *AdModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *data = QStandardItemModel::mimeData(indexes);

    if (indexes.size() > 0) {
        QModelIndex index = indexes[0];
        QString dn = get_dn_of_index(index);

        data->setText(dn);
    }

    return data;
}

bool AdModel::canDropMimeData(const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent) const {    
    QString dropped_dn = data->text();
    QString parent_dn = get_dn_of_index(parent);

    // TODO: complete filtering for valid parent
    if (parent_dn == "") {
        return false;
    }

    if (parent_dn == HEAD_DN) {
        return false;
    }

    // printf("canDropMimeData() parent_dn = %s\n", qPrintable(parent_dn));

    return true;
}

bool AdModel::dropMimeData(const QMimeData *data, Qt::DropAction, int row, int column, const QModelIndex &parent) {
    QString dropped_dn = data->text();
    QString parent_dn = get_dn_of_index(parent);

    // If row/column are defined, then item is dropped BEFORE
    // drop_index
    // TODO: using this, implement dropping at a specifix position in parent's children list
    QModelIndex drop_index;
    if (row == -1 && column == -1) {
        drop_index = parent;
    } else {
        drop_index = this->index(row, column, parent);
    }

    // TODO: if parent is group and dropped entry is user do
    // ad_group_add_user(dropped_dn, parent_dn); (create interface function for this)
    // else
    // move_user(dropped_dn, parent_dn);
    // (parent is group if it has objectClass "group")
    // TODO: need to save objectClass to role or column?
    // since there it's multi-valued maybe a single bool role IsGroup?

    printf("AdModel::dropMimeData(): dropped_dn = %s, parent_dn = %s\n", qPrintable(dropped_dn), qPrintable(parent_dn));

    return true;
}

void load_row(QList<QStandardItem*> row, const QString &dn) {
    load_attributes(dn);
    auto attributes = get_attributes(dn);

    // TODO: get rid of "if (x.contains(y))"
    
    // Name
    QString name = dn;
    if (attributes.contains("name")) {
        name = attributes["name"][0];
    }

    // Category
    QString category = "none";
    if (attributes.contains("objectCategory")) {
        // TODO: convert it completely (turn '-' into ' ')
        // NOTE: raw category is given as DN
        QString category_as_dn = attributes["objectCategory"][0];
        category = extract_name_from_dn(category_as_dn);
    }

    // Description
    QString description = "none";
    if (attributes.contains("description")) {
        description = attributes["description"][0];
    }

    // showInAdvancedViewOnly
    bool advanced_view = false;
    if (attributes.contains("showInAdvancedViewOnly")) {
        QString advanced_view_str = attributes["showInAdvancedViewOnly"][0];

        if (advanced_view_str == "TRUE") {
            advanced_view = true;

        }
    }

    // is container
    bool is_container = false;
    if (attributes.contains("objectClass")) {
        QList<QString> objectClasses = attributes["objectClass"];
        
        QList<QString> container_objectClasses = {"container", "organizationalUnit", "builtinDomain", "domain"};
        for (auto e : container_objectClasses) {
            if (objectClasses.contains(e)) {
                is_container = true;
                break;
            }
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
    row[0]->setData(advanced_view, AdModel::Roles::AdvancedViewOnly);
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
    QList<QString> objectClasses = attributes["objectClass"];
    QString icon_name = "dialog-question";
    for (auto c : objectClasses) {
        if (class_to_icon.contains(c)) {
            icon_name = class_to_icon[c];
            break;    
        }
    }

    QIcon icon = QIcon::fromTheme(icon_name);
    row[0]->setIcon(icon);
}

void load_and_add_row(QStandardItem *parent, const QString &dn) {
    auto row = QList<QStandardItem *>();

    for (int i = 0; i < AdModel::Column::COUNT; i++) {
        row.push_back(new QStandardItem());
    }

    load_row(row, dn);

    // Set fetch flag because row is new and can be fetched
    row[0]->setData(true, AdModel::Roles::CanFetch);

    parent->appendRow(row);
}

AdModel::AdModel(): QStandardItemModel(0, Column::COUNT) {
    this->setHorizontalHeaderItem(Column::Name, new QStandardItem("Name"));
    this->setHorizontalHeaderItem(Column::Category, new QStandardItem("Category"));
    this->setHorizontalHeaderItem(Column::Description, new QStandardItem("Description"));
    this->setHorizontalHeaderItem(Column::DN, new QStandardItem("DN"));

    // Load head
    QStandardItem *invis_root = this->invisibleRootItem();
    auto head_dn = QString(HEAD_DN);
    load_and_add_row(invis_root, head_dn);

    QObject::connect(
        &ad_interface, &AdInterface::entry_deleted,
        this, &AdModel::on_entry_deleted);
    QObject::connect(
        &ad_interface, &AdInterface::entry_changed,
        this, &AdModel::on_entry_changed);
    QObject::connect(
        &ad_interface, &AdInterface::user_moved,
        this, &AdModel::on_user_moved);
    QObject::connect(
        &ad_interface, &AdInterface::entry_created,
        this, &AdModel::on_entry_created);
}

bool AdModel::canFetchMore(const QModelIndex &parent) const {
    if (!parent.isValid()) {
        return false;
    }

    bool can_fetch = parent.data(AdModel::Roles::CanFetch).toBool();

    return can_fetch;
}

void AdModel::fetchMore(const QModelIndex &parent) {
    if (!parent.isValid() || !this->canFetchMore(parent)) {
        return;
    }

    QString dn = get_dn_of_index(parent);

    QStandardItem *parent_item = this->itemFromIndex(parent);

    // Add children
    QList<QString> children = load_children(dn);

    for (auto child : children) {
        load_and_add_row(parent_item, child);
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

void AdModel::on_entry_changed(const QString &dn) {
    // TODO: confirm what kind of search is this, linear?
    QList<QStandardItem *> items = this->findItems(dn, Qt::MatchExactly | Qt::MatchRecursive, AdModel::Column::DN);

    // TODO: not sure if any bad matches can happen, maybe?
    if (items.size() > 0) {
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
            QStandardItem *item = this->itemFromIndex(index);
            row.push_back(item);
        }

        // Reload row
        load_row(row, dn);
    }
}

void AdModel::on_entry_deleted(const QString &dn) {
    QList<QStandardItem *> items = this->findItems(dn, Qt::MatchExactly | Qt::MatchRecursive, AdModel::Column::DN);

    if (items.size() > 0) {
        QStandardItem *dn_item = items[0];
        QModelIndex dn_index = dn_item->index();
        
        this->removeRow(dn_index.row(), dn_index.parent());
    }
}

void AdModel::on_user_moved(const QString &old_dn, const QString &new_dn, const QString &new_parent_dn) {
    // Remove old entry from model
    QList<QStandardItem *> old_items = this->findItems(old_dn, Qt::MatchExactly | Qt::MatchRecursive, AdModel::Column::DN);
    if (old_items.size() > 0) {
        QStandardItem *dn_item = old_items[0];
        QModelIndex dn_index = dn_item->index();
        
        this->removeRow(dn_index.row(), dn_index.parent());
    }

    printf("on_user_moved: %s %s\n", qPrintable(new_dn), qPrintable(new_parent_dn));

    // Need to load entry at new parent if the parent has already
    // been expanded/fetched
    // NOTE: loading if parent has already been fetched will
    // create a duplicate
    QList<QStandardItem *> parent_items = this->findItems(new_parent_dn, Qt::MatchExactly | Qt::MatchRecursive, AdModel::Column::DN);
    if (parent_items.size() > 0) {
        QStandardItem *parent_dn_item = parent_items[0];
        QModelIndex parent_dn_index = parent_dn_item->index();
        QModelIndex parent_index = parent_dn_index.siblingAtColumn(Column::Name);

        QStandardItem *parent_item = this->itemFromIndex(parent_index);

        if (!this->canFetchMore(parent_index)) {
            load_and_add_row(parent_item, new_dn);
        }
    }
}

void AdModel::on_entry_created(const QString &dn) {
    // Load entry to model if it's parent has already been fetched
    // If it hasn't been fetched, then this new entry will be loaded with all other children when the parent is fetched
    QString parent_dn = extract_parent_dn_from_dn(dn);
    QList<QStandardItem *> items = this->findItems(parent_dn, Qt::MatchExactly | Qt::MatchRecursive, Column::DN);

    if (items.size() > 0) {
        QStandardItem *dn_item = items[0];
        QModelIndex dn_index = dn_item->index();
        QModelIndex parent_index = dn_index.siblingAtColumn(0);
        QStandardItem *parent = this->itemFromIndex(parent_index);

        bool fetched_already = !this->canFetchMore(parent_index);
        if (fetched_already) {
            load_and_add_row(parent, dn);
        }
    }
}
