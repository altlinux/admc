/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2026 BaseALT Ltd.
 * Copyright (C) 2026 Semyon Knyazev
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

#include "server_dn_attrs_updater.h"
#include "core/fsmo.h"
#include "core/globals.h"
#include "ad_config.h"
#include "ad_defines.h"
#include "ad_filter.h"
#include "ad_interface.h"
#include "ad_object.h"
#include "fsmo/fsmo_utils.h"
#include "ad_utils.h"

ServerDnAttrsUpdater::ServerDnAttrsUpdater(const QString &server_dn) : dn(server_dn) {

}

void ServerDnAttrsUpdater::update_for_delete(AdInterface &ad) {
    update_replica_locations(ad, "");
    update_fsmo_role_owner(ad, "");
    update_inter_site_topology_generator(ad, "");
}

void ServerDnAttrsUpdater::update_for_move(AdInterface &ad, const QString &new_server_dn) {
    update_replica_locations(ad, new_server_dn);
    update_fsmo_role_owner(ad, new_server_dn);
    update_inter_site_topology_generator(ad, new_server_dn);
}

void ServerDnAttrsUpdater::update_replica_locations(AdInterface &ad, const QString &new_server_dn) {
    const QStringList dns_zone_nc_names = {
            QString("DC=DomainDnsZones,%1").arg(g_adconfig->domain_dn()),
            QString("DC=ForestDnsZones,%1").arg(g_adconfig->domain_dn()),
    };

    const QString partitions_dn = QString("CN=Partitions,CN=Configuration,%1").arg(g_adconfig->domain_dn());
    const QString filter_crossrefs = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_CROSS_REF);
    const QHash<QString, AdObject> crossrefs = ad.search(partitions_dn, SearchScope_Children, filter_crossrefs,
            {ATTRIBUTE_DN, ATTRIBUTE_NC_NAME, ATTRIBUTE_REPLICA_LOCATIONS});

    const QString filter_sites = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_SITE);
    const QHash<QString, AdObject> actual_site_dn_list = ad.search(g_adconfig->sites_container_dn(), SearchScope_Children, filter_sites,
            {ATTRIBUTE_DN});

    for (const QString &crossref_dn : crossrefs.keys()) {
        const QString nc_name = crossrefs[crossref_dn].get_string(ATTRIBUTE_NC_NAME);
        if (!dns_zone_nc_names.contains(nc_name)) {
            continue;
        }

        QList<QByteArray> values = crossrefs[crossref_dn].get_values(ATTRIBUTE_REPLICA_LOCATIONS);
        const QString old_server_setts_dn = QString("CN=NTDS Settings,") + dn;
        values.removeAll(old_server_setts_dn.toUtf8());
        cleanup_missing_site_dn_values(values, actual_site_dn_list.keys());


        const QString new_server_setts_dn = new_server_dn.isEmpty() ? QString() : QString("CN=NTDS Settings,") + new_server_dn;
        if (!new_server_setts_dn.isEmpty()) {
            values.append(new_server_setts_dn.toUtf8());
        }

        ad.attribute_replace_values(crossref_dn, ATTRIBUTE_REPLICA_LOCATIONS, values);
    }
}

void ServerDnAttrsUpdater::update_fsmo_role_owner(AdInterface &ad, const QString &new_server_dn) {
    if (new_server_dn.isEmpty()) {
        return;
    }

    QStringList server_fsmo_roles;
    for (int role = 0; role < (int)FSMORole_COUNT; ++role) {
        if (dn == current_master_for_role(ad, (FSMORole)role)) {
            server_fsmo_roles.append(fsmo_dn_from_role((FSMORole)role));
        }
    }

    for (auto role_dn : server_fsmo_roles) {
        ad.attribute_replace_string(role_dn, ATTRIBUTE_FSMO_ROLE_OWNER, new_server_dn);
    }
}

void ServerDnAttrsUpdater::update_inter_site_topology_generator(AdInterface &ad, const QString &new_server_dn) {
    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_NTDS_SITE_SETTINGS);
    const QHash<QString, AdObject> ntds_site_settings = ad.search(g_adconfig->sites_container_dn(),
                                                                  SearchScope_All, filter, {ATTRIBUTE_INTER_SITE_TOPOLOGY_GENERATOR});

    for (auto obj_dn : ntds_site_settings.keys()) {
        bool contains_old_dn = ntds_site_settings[obj_dn].get_string(ATTRIBUTE_INTER_SITE_TOPOLOGY_GENERATOR).contains(dn);
        if (contains_old_dn) {
            ad.attribute_replace_string(obj_dn, ATTRIBUTE_INTER_SITE_TOPOLOGY_GENERATOR, QString());
        }
    }

    QString parent_site_setts_dn = QString("CN=NTDS Site Settings,") + dn_get_parent(dn_get_parent(new_server_dn));
    QString new_server_setts_dn = new_server_dn.isEmpty() ? QString() : QString("CN=NTDS Settings,") + new_server_dn;
    ad.attribute_replace_string(parent_site_setts_dn, ATTRIBUTE_INTER_SITE_TOPOLOGY_GENERATOR, new_server_setts_dn);
}

void ServerDnAttrsUpdater::cleanup_missing_site_dn_values(QList<QByteArray> &server_setts_dn_list, const QStringList &actual_site_list) {
    QList<QByteArray> actual_ntds_server_setts_list;
    for (auto server_setts_dn : server_setts_dn_list) {
        QString server_setts_str = QString::fromUtf8(server_setts_dn);
        QString site_server_dn = dn_get_parent(dn_get_parent(dn_get_parent(server_setts_str)));
        if (actual_site_list.contains(site_server_dn)) {
            actual_ntds_server_setts_list.append(server_setts_dn);
        }
    }

    server_setts_dn_list = actual_ntds_server_setts_list;
}


