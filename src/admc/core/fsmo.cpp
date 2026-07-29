/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2023-2026 BaseALT Ltd.
 * Copyright (C) 2023 Semyon Knyazev
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
#include <QObject>
#include <QString>
#include <functional>

#include "ad_config.h"
#include "core/ad.h"
#include "core/settings.h"
#include "fsmo.h"
#include "globals.h"

// Returns the DN of the object that
// store's role's master in it's attributes
QString fsmo_dn_from_role(FSMORole role) {
    const QString domain_dn = g_adconfig->domain_dn();

    switch (role) {
    case FSMORole_DomainDNS:
        return QString("CN=Infrastructure,DC=DomainDnsZones,%1").arg(domain_dn);
    case FSMORole_ForestDNS:
        return QString("CN=Infrastructure,DC=ForestDnsZones,%1").arg(domain_dn);
    case FSMORole_PDCEmulation:
        return domain_dn;
    case FSMORole_Schema:
        return g_adconfig->schema_dn();
    case FSMORole_DomainNaming:
        return g_adconfig->partitions_dn();
    case FSMORole_Infrastructure:
        return QString("CN=Infrastructure,%1").arg(domain_dn);
    case FSMORole_RidAllocation:
        return QString("CN=RID Manager$,CN=System,%1").arg(domain_dn);

    case FSMORole_COUNT:
        break;
    };

    return QString();
}

QString fsmo_role_to_string(FSMORole role) {
    switch (role) {
    case FSMORole_DomainDNS:
        return "Domain DNS";
    case FSMORole_ForestDNS:
        return "Forest DNS";
    case FSMORole_PDCEmulation:
        return "PDC Emulator";
    case FSMORole_Schema:
        return "Schema master";
    case FSMORole_DomainNaming:
        return "Domain naming master";
    case FSMORole_Infrastructure:
        return "Infrastructure master";
    case FSMORole_RidAllocation:
        return "RID master";

    case FSMORole_COUNT:
        break;
    };

    return QString();
}

bool fsmo_is_current_dc_master_for_role(AdInterface &ad, FSMORole role) {
    QString role_dn = fsmo_dn_from_role(role);
    QString current_master = ad_current_master_for_role_dn(ad, role_dn);
    QString current_dc = ad_current_dc_dns_host_name(ad);
    return current_master == current_dc;
}

QString fsmo_current_master_for_role(AdInterface &ad, FSMORole role) {
    QString role_dn = fsmo_dn_from_role(role);
    return ad_current_master_for_role_dn(ad, role_dn);
}

void fsmo_connect_host_with_role(AdInterface &ad, FSMORole role) {
    QString current_master = fsmo_current_master_for_role(ad, role);
    settings_set_variant(SETTING_host, current_master);
    AdInterface::set_dc(current_master);
    ad.update_dc();
}

QString fsmo_string_from_dn(const QString &fsmo_role_dn) {
    for (int role = 0; role < FSMORole_COUNT; ++role) {
        if (fsmo_dn_from_role(FSMORole(role)) == fsmo_role_dn) {
            return fsmo_role_to_string(FSMORole(role));
        }
    }
    return QString();
}

bool fsmo_role_from_dn(const QString &role_dn, FSMORole &role_out) {
    const QString domain_dn = g_adconfig->domain_dn();
    const QString schema_dn = g_adconfig->schema_dn();
    const QString partitions_dn = g_adconfig->partitions_dn();
    const QString dn_lower = role_dn.toLower();

    if (dn_lower == domain_dn.toLower()) {
        role_out = FSMORole_PDCEmulation;
        return true;
    }

    if (dn_lower == schema_dn.toLower()) {
        role_out = FSMORole_Schema;
        return true;
    }

    if (dn_lower == partitions_dn.toLower()) {
        role_out = FSMORole_DomainNaming;
        return true;
    }

    const QString domain_dns = QString("cn=infrastructure,dc=domaindnszones,%1")
        .arg(domain_dn).toLower();
    if (dn_lower == domain_dns) {
        role_out = FSMORole_DomainDNS;
        return true;
    }

    const QString forest_dns = QString("cn=infrastructure,dc=forestdnszones,%1")
        .arg(domain_dn).toLower();
    if (dn_lower == forest_dns) {
        role_out = FSMORole_ForestDNS;
        return true;
    }

    if (dn_lower == QString("cn=infrastructure,%1").arg(domain_dn).toLower()) {
        role_out = FSMORole_Infrastructure;
        return true;
    }

    const QString rid_allocation = QString("cn=rid manager$,cn=system,%1")
        .arg(domain_dn).toLower();
    if (dn_lower == rid_allocation) {
        role_out = FSMORole_RidAllocation;
        return true;
    }

     return false;
}

bool fsmo_set_master(AdInterface& ad,
                     const AdObject &root_dse,
                     const QString &role_dn) {
    const QString new_master_service =
        root_dse.get_string(ATTRIBUTE_DS_SERVICE_NAME);
    return ad.attribute_replace_string(role_dn,
                                       ATTRIBUTE_FSMO_ROLE_OWNER,
                                       new_master_service);
}
