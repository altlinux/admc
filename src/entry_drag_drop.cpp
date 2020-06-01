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

#include "ad_interface.h"
#include "constants.h"

#include <QString>

bool can_drop_entry(const QString &dn, const QString &parent_dn) {    
    const bool dropped_is_user = attribute_value_exists(dn, "objectClass", "user");

    const bool parent_is_group = attribute_value_exists(parent_dn, "objectClass", "group");
    const bool parent_is_ou = attribute_value_exists(parent_dn, "objectClass", "organizationalUnit");
    const bool parent_is_container = attribute_value_exists(parent_dn, "objectClass", "container");

    // TODO: support dropping non-users
    // TODO: support dropping policies
    if (parent_dn == "") {
        return false;
    } else if (parent_dn == HEAD_DN) {
        return false;
    } else if (dropped_is_user && (parent_is_group || parent_is_ou || parent_is_container)) {
        return true;
    } else {
        return false;
    }
}

void drop_entry(const QString &dn, const QString &parent_dn) {
    const bool dropped_is_user = attribute_value_exists(dn, "objectClass", "user");

    const bool parent_is_group = attribute_value_exists(parent_dn, "objectClass", "group");
    const bool parent_is_ou = attribute_value_exists(parent_dn, "objectClass", "organizationalUnit");
    const bool parent_is_container = attribute_value_exists(parent_dn, "objectClass", "container");

    if (dropped_is_user && (parent_is_ou || parent_is_container)) {
        move_user(dn, parent_dn);
    } else if (dropped_is_user && parent_is_group) {
        add_user_to_group(parent_dn, dn);
    }
}
