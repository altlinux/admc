/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2025-2026 BaseALT Ltd.
 * Copyright (C) 2025 Semyon Knyazev
 * Copyright (C) 2026 Artyom V. Poptsov
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

#include "console_impls/object_impl/drag_n_drop.h"
#include <QModelIndex>
#include <core/globals.h>
#include "console_impls/object_impl/object_impl.h"

ObjectDragDrop::DropType ObjectDragDrop::console_object_get_drop_type(const QModelIndex &dropped, const QModelIndex &target) {
    const QString dropped_dn = dropped.data(ObjectRole_DN).toString();
    const QString target_dn = target.data(ObjectRole_DN).toString();
    const bool dropped_is_target = (dropped_dn == target_dn);

    const QList<QString> dropped_classes = dropped.data(ObjectRole_ObjectClasses).toStringList();
    const QList<QString> target_classes = target.data(ObjectRole_ObjectClasses).toStringList();

    const bool dropped_is_user = dropped_classes.contains(CLASS_USER);
    const bool dropped_is_group = dropped_classes.contains(CLASS_GROUP);
    const bool target_is_group = target_classes.contains(CLASS_GROUP);
    const bool target_is_fetching = target.data(ObjectRole_Fetching).toBool();

    if (dropped_is_target || target_is_fetching) {
        return DropType_None;
    } else if (dropped_is_user && target_is_group) {
        return DropType_AddToGroup;
    } else if (dropped_is_group && target_is_group) {
        return DropType_AddToGroup;
    } else {
        const QList<QString> dropped_superiors = g_adconfig->get_possible_superiors(dropped_classes);
        bool target_is_valid_superior = false;
        for (const auto &object_class : dropped_superiors) {
            if (target_classes.contains(object_class)) {
                target_is_valid_superior = true;
                break;
            }
        }

        if (target_is_valid_superior) {
            return DropType_Move;
        } else {
            return DropType_None;
        }
    }
}
