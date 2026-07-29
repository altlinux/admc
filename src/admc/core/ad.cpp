/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2026 BaseALT Ltd.
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

#include <QHash>
#include <QList>
#include <QString>

#include "core/globals.h"
#include "ad_config.h"
#include "ad_interface.h"
#include "ad_object.h"
#include "adldap.h"

/**
 * Search AD objects from the specified list.
 *
 * TODO: Move to adldap.
 *
 * @param ad An AD instance.
 * @param list A list of objects to search.
 * @return A QHash of DNs and AdObject pairs.
 */
QHash<QString, AdObject> ad_search_objects(AdInterface &ad,
                                           const QList<QString> list) {
    QHash<QString, AdObject> object_map;
    for (const QString &dn : list) {
        const AdObject object = ad.search_object(dn);
        object_map[dn] = object;
    }
    return object_map;
}

void ad_add_members_to_groups(AdInterface &ad,
                              const  QList<QString> &targets,
                              const QList<QString> &groups) {
    for (const QString &target : targets) {
        for (auto group : groups) {
            ad.group_add_member(group, target);
        }
    }
}

/**
 * Select only changed DNs in the specified hash map of old-to-new DNs.
 *
 * @param map A map of DNs.
 * @return A new QHash table of DNs that were actually changed.
 */
QHash<QString, QString> ad_select_changed_dn(QHash<QString, QString> map) {
    QHash<QString, QString> changed_dn;
    for (const QString &old_dn : map.keys()) {
        const QString new_dn = map[old_dn];
        const bool dn_changed = (new_dn != old_dn);
        if (dn_changed) {
            changed_dn[old_dn] = new_dn;
        }
    }
    return changed_dn;
}

/**
 * Check if the specified AD object is a person.
 *
 * @param object An AD object to check.
 * @return True if the object is a person, false otherwise.
 */
bool ad_object_is_person(const AdObject &object) {
    return (object.is_class(CLASS_USER) || object.is_class(CLASS_INET_ORG_PERSON));
}

QString ad_current_dc_dns_host_name(AdInterface &ad) {
    const AdObject rootDSE = ad.search_object("");
    const QString server_name = rootDSE.get_string(ATTRIBUTE_SERVER_NAME);
    const AdObject server = ad.search_object(server_name);
    const QString out = server.get_string(ATTRIBUTE_DNS_HOST_NAME);

    return out;
}

int ad_get_range_upper(const QString &attribute) {
    if (attribute == ATTRIBUTE_UPN_SUFFIXES) {
        const int upn_max_length =
            g_adconfig->get_attribute_range_upper(
                ATTRIBUTE_USER_PRINCIPAL_NAME);

        // NOTE: schema doesn't define a max length for
        // "upn suffixes", but we do need a limit. Use
        // half of total max length of upn as a good
        // estimate.
        const int upn_suffix_max_length = upn_max_length / 2;

        return upn_suffix_max_length;
    } else {
        const int out = g_adconfig->get_attribute_range_upper(attribute);

        return out;
    }
}

/**
 * Returns the DNS host name of role's master object (DC.)
 */
QString ad_current_master_for_role_dn(AdInterface &ad, QString role_dn) {
    const AdObject role_object = ad.search_object(role_dn);
    const QString master_settings_dn =
        role_object.get_string(ATTRIBUTE_FSMO_ROLE_OWNER);
    const QString master_dn = dn_get_parent(master_settings_dn);
    const AdObject master_object = ad.search_object(master_dn);
    const QString current_master =
        master_object.get_string(ATTRIBUTE_DNS_HOST_NAME);
    return current_master;
}
