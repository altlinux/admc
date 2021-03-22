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

bool object_can_drop(const AdObject &dropped, const AdObject &target) {
    const DropType drop_type = get_drop_type(dropped, target);

    if (drop_type == DropType_None) {
        return false;
    } else {
        return true;
    }
}
