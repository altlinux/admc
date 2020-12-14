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

#include "object_model.h"
#include "ad_interface.h"
#include "ad_object.h"
#include "ad_config.h"
#include "filter.h"
#include "settings.h"
#include "utils.h"

#include <QMimeData>
#include <QString>

ObjectModel::ObjectModel(const int column_count, const int dn_column_arg, QObject *parent)
: QStandardItemModel(0, column_count, parent)
, dn_column(dn_column_arg)
{
    set_horizontal_header_labels_from_map(this, {
        {ObjectModel::Column::Name, tr("Name")},
        {ObjectModel::Column::DN, tr("DN")}
    });

    // Make row for head object
    QStandardItem *invis_root = invisibleRootItem();
    const QString head_dn = AD()->domain_head();
    const AdObject head_object = AD()->search_object(head_dn);

    make_row(invis_root, head_object);
}

bool ObjectModel::canFetchMore(const QModelIndex &parent) const {
    if (!parent.isValid()) {
        return false;
    }

    bool can_fetch = parent.data(ObjectModel::Roles::CanFetch).toBool();

    return can_fetch;
}

void ObjectModel::fetchMore(const QModelIndex &parent) {
    if (!parent.isValid() || !canFetchMore(parent)) {
        return;
    }

    const QString parent_dn = get_dn_from_index(parent, ObjectModel::Column::DN);

    QStandardItem *parent_item = itemFromIndex(parent);

    // Add children
    // TODO: move advanced view filtering to a proxy
    const QList<QString> search_attributes = {ATTRIBUTE_NAME, ATTRIBUTE_DISTINGUISHED_NAME, ATTRIBUTE_OBJECT_CLASS};
    const QString filter = current_advanced_view_filter();
    const QHash<QString, AdObject> search_results = AD()->search(filter, search_attributes, SearchScope_Children, parent_dn);

    for (const AdObject object : search_results.values()) {
        make_row(parent_item, object);
    }

    // Unset CanFetch flag
    parent_item->setData(false, ObjectModel::Roles::CanFetch);

    // In dev mode, load configuration and schema objects
    // NOTE: have to manually add configuration and schema objects because they aren't searchable
    const bool dev_mode = SETTINGS()->get_bool(BoolSetting_DevMode);
    if (dev_mode) {
        const QString search_base = AD()->domain_head();
        const QString configuration_dn = AD()->configuration_dn();
        const QString schema_dn = AD()->schema_dn();

        const auto load_manually =
        [this, parent_item](const QString &child) {
            const AdObject object = AD()->search_object(child);
            make_row(parent_item, object);
        };

        if (parent_dn == search_base) {
            load_manually(configuration_dn);
        } else if (parent_dn == configuration_dn) {
            load_manually(schema_dn);
        }
    }
}

// Override this so that unexpanded and unfetched items show the expander even though they technically don't have any children loaded
// NOTE: expander is show if hasChildren returns true
bool ObjectModel::hasChildren(const QModelIndex &parent = QModelIndex()) const {
    if (canFetchMore(parent)) {
        return true;
    } else {
        return QStandardItemModel::hasChildren(parent);
    }
}

QMimeData *ObjectModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *data = QStandardItemModel::mimeData(indexes);

    if (indexes.size() > 0) {
        QModelIndex index = indexes[0];
        QString dn = get_dn_from_index(index, dn_column);

        data->setText(dn);
    }

    return data;
}

bool ObjectModel::canDropMimeData(const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent) const {
    const QString dn = data->text();
    const QString target_dn = get_dn_from_index(parent, dn_column);

    const bool can_drop = AD()->object_can_drop(dn, target_dn);

    return can_drop;
}

bool ObjectModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) {
    if (row != -1 || column != -1) {
        return true;
    }

    if (!canDropMimeData(data, action, row, column, parent)) {
        return true;
    }

    const QString dn = data->text();
    const QString target_dn = get_dn_from_index(parent, dn_column);

    AD()->object_drop(dn, target_dn);

    return true;
}

// Make row in model at given parent based on object with given dn
QStandardItem *ObjectModel::make_row(QStandardItem *parent, const AdObject &object) {
    const bool is_container =
    [object]() {
        static const QList<QString> accepted_classes =
        []() {
            QList<QString> out = ADCONFIG()->get_filter_containers();

            // Make configuration and schema pass filter in dev mode so they are visible and can be fetched
            const bool dev_mode = SETTINGS()->get_bool(BoolSetting_DevMode);
            if (dev_mode) {
                out.append({CLASS_CONFIGURATION, CLASS_dMD});
            }

            // TODO: domain not included for some reason, so add it ourselves
            out.append(CLASS_DOMAIN);
            
            return out;
        }();

        // NOTE: compare against all classes of object, not just the most derived one
        const QList<QString> object_classes = object.get_strings(ATTRIBUTE_OBJECT_CLASS);
        for (const auto acceptable_class : accepted_classes) {
            if (object_classes.contains(acceptable_class)) {
                return true;
            }
        }

        return false;
    }();

    const QList<QStandardItem *> row = make_item_row(ObjectModel::Column::COUNT);
    
    const QString name = object.get_string(ATTRIBUTE_NAME);
    const QString dn = object.get_dn();
    row[ObjectModel::Column::Name]->setText(name);
    row[ObjectModel::Column::DN]->setText(dn);

    const QIcon icon = object.get_icon();
    row[0]->setIcon(icon);

    // Can fetch(expand) object if it's a container
    row[0]->setData(is_container, ObjectModel::Roles::CanFetch);
    row[0]->setData(is_container, ObjectModel::Roles::IsContainer);

    parent->appendRow(row);

    return row[0];
}
