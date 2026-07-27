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

#include "site_dn_attrs_updater.h"
#include "ad_config.h"
#include "ad_defines.h"
#include "ad_filter.h"
#include "ad_interface.h"
#include "ad_object.h"
#include "core/globals.h"
#include "ad_utils.h"

SiteDnAttrsUpdater::SiteDnAttrsUpdater(const QString &site_dn) : dn(site_dn) {

}

void SiteDnAttrsUpdater::update_for_delete(AdInterface &ad) {
    update_site_links(ad, "");
    update_dns_records(ad, "");
}

void SiteDnAttrsUpdater::update_for_rename(AdInterface &ad, const QString &new_dn) {
    update_site_links(ad, new_dn);
    update_dns_records(ad, new_dn);
}

void SiteDnAttrsUpdater::update_site_links(AdInterface &ad, const QString &new_site_dn) {
    const QString filter1 = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_SITE_LINK);
    const QString filter2 = filter_CONDITION(Condition_Contains, ATTRIBUTE_SITE_LIST, dn);
    const QString filter = filter_AND({filter1, filter2});
    const QString intersite_transports_dn = "CN=Inter-Site Transports," + g_adconfig->sites_container_dn();

    auto res = ad.search(intersite_transports_dn, SearchScope_All, filter, {ATTRIBUTE_DN, ATTRIBUTE_SITE_LIST});
    for (const QString &sitelink_dn : res.keys()) {
        QList<QByteArray> values = res[sitelink_dn].get_values(ATTRIBUTE_SITE_LIST);
        values.removeAll(dn.toUtf8());

        if (!new_site_dn.isEmpty()) {
            values.append(new_site_dn.toUtf8());
        }

        ad.attribute_replace_values(sitelink_dn, ATTRIBUTE_SITE_LIST, values);
    }
}

void SiteDnAttrsUpdater::update_dns_records(AdInterface &ad, const QString &new_site_dn) {
    Q_UNUSED(new_site_dn) // May be useful arg later

    const QString domain_zone_dn = QString("DC=%1,CN=MicrosoftDNS,DC=DomainDnsZones,%2").
            arg(g_adconfig->domain().toLower()).arg(g_adconfig->domain_dn());
    const QString forest_zone_dn = QString("DC=%1,CN=MicrosoftDNS,DC=ForestDnsZones,%2").
            arg(QString("_msdcs.") + g_adconfig->domain().toLower()).arg(g_adconfig->domain_dn());

    const QString site_name = dn_get_name(dn);

    for (auto dns_zone_dn : {domain_zone_dn, forest_zone_dn}) {
        const QString filter1 = filter_CONDITION(Condition_Contains, ATTRIBUTE_DNS_RECORD_DC,
            QString(".%1._sites").arg(site_name));
        const QString filter2 = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_DNS_NODE);
        const QString res_filter = filter_AND({filter1, filter2});

        QHash<QString, AdObject> dns_nodes = ad.search(dns_zone_dn, SearchScope_Children, res_filter, {ATTRIBUTE_DN});
        for (auto dns_node_dn : dns_nodes.keys()) {
            ad.object_delete(dns_node_dn);
        }
    }
}
