
#include "ad_model.h"
#include "constants.h"

#include <QMimeData>
#include <QMap>
#include <QIcon>

void make_new_row(QStandardItem *parent, const QString &dn);

AdModel::AdModel(QObject *parent)
: QStandardItemModel(0, Column::COUNT, parent)
{
    setHorizontalHeaderItem(Column::Name, new QStandardItem("Name"));
    setHorizontalHeaderItem(Column::Category, new QStandardItem("Category"));
    setHorizontalHeaderItem(Column::Description, new QStandardItem("Description"));
    setHorizontalHeaderItem(Column::DN, new QStandardItem("DN"));

    QObject::connect(
        &ad_interface, &AdInterface::delete_entry_complete,
        this, &AdModel::on_delete_entry_complete);
    QObject::connect(
        &ad_interface, &AdInterface::move_user_complete,
        this, &AdModel::on_move_user_complete);
    QObject::connect(
        &ad_interface, &AdInterface::create_entry_complete,
        this, &AdModel::on_create_entry_complete);
    QObject::connect(
        &ad_interface, &AdInterface::load_attributes_complete,
        this, &AdModel::on_load_attributes_complete);

    // Load head
    QStandardItem *invis_root = invisibleRootItem();
    auto head_dn = QString(HEAD_DN);
    make_new_row(invis_root, head_dn);
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

    QString dn = get_dn_of_index(parent);

    QStandardItem *parent_item = itemFromIndex(parent);

    // Add children
    QList<QString> children = load_children(dn);

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
        drop_index = index(row, column, parent);
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

void AdModel::on_load_attributes_complete(const QString &dn) {
    //
    // Compose row of items
    //
    auto row = QList<QStandardItem *>();
    {
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
        for (int i = 0; i < AdModel::Column::COUNT; i++) {
            QModelIndex index = indexes[i];
            QStandardItem *item = itemFromIndex(index);
            row.push_back(item);
        }
    }

    //
    // Load row based on attributes
    //
    QString name = get_attribute(dn, "name");

    // NOTE: this is given as raw DN and contains '-' where it should
    // have spaces, so convert it
    QString category = get_attribute(dn, "objectCategory");
    category = extract_name_from_dn(category);
    category = category.replace('-', ' ');

    QString description = get_attribute(dn, "description");

    bool showInAdvancedViewOnly = get_attribute(dn, "showInAdvancedViewOnly") == "TRUE";

    bool is_container = false;
    const QList<QString> container_objectClasses = {"container", "organizationalUnit", "builtinDomain", "domain"};
    for (auto c : container_objectClasses) {
        if (attribute_value_exists(dn, "objectClass", c)) {
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
        if (attribute_value_exists(dn, "objectClass", c)) {
            icon_name = class_to_icon[c];
            break;  
        }
    }

    QIcon icon = QIcon::fromTheme(icon_name);
    row[0]->setIcon(icon);
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

QString get_dn_of_index(const QModelIndex &index) {
    QModelIndex dn_index = index.siblingAtColumn(AdModel::Column::DN);
    QString dn = dn_index.data().toString();

    return dn;
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

    // Load attributes since this is the first time this entry is loaded
    load_attributes(dn);

    // NOTE: the row is loaded in on_load_attributes_complete() slot
}
