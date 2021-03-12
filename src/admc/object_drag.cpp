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

#include "object_drag.h"

#include "ad_interface.h"
#include "ad_config.h"
#include "ad_object.h"
#include "object_model.h"

#include <QMimeData>

enum DropType {
    DropType_Move,
    DropType_AddToGroup,
    DropType_None
};

// Determine what kind of drop type is dropping this object
// onto target. If drop type is none, then can't drop this
// object on this target.
DropType get_drop_type(const AdObject &dropped, const AdObject &target) {
    if (dropped.get_dn() == target.get_dn()) {
        return DropType_None;
    }

    const bool dropped_is_user = dropped.contains_class(CLASS_USER);
    const bool dropped_is_group = dropped.contains_class(CLASS_GROUP);
    const bool target_is_group = target.contains_class(CLASS_GROUP);

    if (dropped_is_user && target_is_group) {
        return DropType_AddToGroup;
    } else if (dropped_is_group && target_is_group) {
        return DropType_AddToGroup;
    } else {
        const bool system_flags_forbid_move = dropped.get_system_flag(SystemFlagsBit_CannotMove);

        const QList<QString> dropped_classes = dropped.get_strings(ATTRIBUTE_OBJECT_CLASS);
        const QList<QString> dropped_superiors = ADCONFIG()->get_possible_superiors(dropped_classes);

        const bool target_is_valid_superior =
        [target, dropped_superiors]() {
            const QList<QString> target_classes = target.get_strings(ATTRIBUTE_OBJECT_CLASS);
            for (const auto &object_class : dropped_superiors) {
                if (target_classes.contains(object_class)) {
                    return true;
                }
            }

            return false;
        }();

        if (system_flags_forbid_move) {
            return DropType_None;
        } else if (target_is_valid_superior) {
            return DropType_Move;
        } else {
            return DropType_None;
        }
    }
}

// General "drop" operation that can either move, link or change membership depending on which types of objects are involved
void object_drop(AdInterface &ad, const AdObject &dropped, const AdObject &target) {
    DropType drop_type = get_drop_type(dropped, target);

    switch (drop_type) {
        case DropType_Move: {
            ad.object_move(dropped.get_dn(), target.get_dn());
            break;
        }
        case DropType_AddToGroup: {
            ad.group_add_member(target.get_dn(), dropped.get_dn());
            break;
        }
        case DropType_None: {
            break;
        }
    }
}

QMimeData *object_mime_data(const QList<QModelIndex> &indexes) {
    auto data = new QMimeData();

    // Get adobject's from item data and convert the list to
    // a variant
    const QList<QVariant> objects =
    [=]() {
        QList<QVariant> out;
        for (const QModelIndex index : indexes) {
            if (index.column() == 0) {
                const QVariant object_variant = index.data(ObjectRole_AdObject);
                out.append(object_variant);
            }
        }

        return out;
    }();
    const QVariant objects_as_variant(objects);

    // Convert objects variant to bytes
    QByteArray objects_as_bytes;
    QDataStream stream(&objects_as_bytes, QIODevice::WriteOnly);
    stream << objects_as_variant;
    
    data->setData(MIME_TYPE_OBJECT, objects_as_bytes);

    return data;
}

bool object_can_drop(const QMimeData *data, const QModelIndex &parent) {
    if (!data->hasFormat(MIME_TYPE_OBJECT)) {
        return false;
    }

    const QList<AdObject> dropped_list = mimedata_to_object_list(data);
    const AdObject target = parent.siblingAtColumn(0).data(ObjectRole_AdObject).value<AdObject>();

    // NOTE: only check if object can be dropped if dropping a single object, because when dropping multiple objects it is ok for some objects to successfully drop and some to fail. For example, if you drop users together with OU's onto a group, users will be added to that group while OU will fail to drop.
    if (dropped_list.size() == 1) {
        const AdObject dropped = dropped_list[0];
        const DropType drop_type = get_drop_type(dropped, target);
        const bool can_drop = (drop_type != DropType_None);

        return can_drop;
    } else {
        return true;
    }
}

QList<AdObject> mimedata_to_object_list(const QMimeData *data) {
    // Convert from bytes to variant
    QByteArray objects_as_bytes = data->data(MIME_TYPE_OBJECT);
    QDataStream stream(&objects_as_bytes, QIODevice::ReadOnly);
    QVariant objects_as_variant;
    stream >> objects_as_variant;

    // Convert from variant to list of objects
    const QList<QVariant> objects_as_variant_list = objects_as_variant.toList();

    QList<AdObject> out;

    for (const QVariant &e : objects_as_variant_list) {
        const AdObject object = e.value<AdObject>();
        out.append(object);
    }

    return out;
}
