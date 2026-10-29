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

/**
 * @file fsmo.cpp
 * @brief Procedures for working with Flexible Single Master Operation (FSMO).
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

/**
 * Return the distinguished name (DN) for the object that stores role master in
 * it's attributes.
 *
 * @param role A role to use.
 * @return A DN string.
 */
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

/**
 * Convert a role to a string.
 *
 * @param role A role to convert.
 * @return A role string.
 */
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

/**
 * Check if the current domain controller (DC) is a master for the given role.
 *
 * @param ad An AD interface instance.
 * @param role A role to check.
 * @return True if the DC is the master for the role, false otherwise.
 */
bool fsmo_is_current_dc_master_for_role(AdInterface &ad, FSMORole role) {
    QString role_dn = fsmo_dn_from_role(role);
    QString current_master = ad_current_master_for_role_dn(ad, role_dn);
    QString current_dc = ad_current_dc_dns_host_name(ad);
    return current_master == current_dc;
}

/**
 * Get the current master for the given role.
 *
 * @param ad An AD interface instance.
 * @param role A role to query.
 * @return DNS host name for the role master.
 */
QString fsmo_current_master_for_role(AdInterface &ad, FSMORole role) {
    QString role_dn = fsmo_dn_from_role(role);
    return ad_current_master_for_role_dn(ad, role_dn);
}

/**
 * Connect to a domain controller (DC) with the given role.
 * @param ad An AD interface instance.
 * @param role A role to use.
 */
void fsmo_connect_host_with_role(AdInterface &ad, FSMORole role) {
    QString current_master = fsmo_current_master_for_role(ad, role);
    settings_set_variant(SETTING_host, current_master);
    AdInterface::set_custom_dc(nullptr);
    AdInterface::set_dc(current_master);
    ad.update_dc();
}

/**
 * Get a FSMO role string for the given FSMO role distinguished name.
 *
 * @param fsmo_role_dn An FSMO role to convert.
 * @return A FSMO role string.
 */
QString fsmo_string_from_dn(const QString &fsmo_role_dn) {
    for (int role = 0; role < FSMORole_COUNT; ++role) {
        if (fsmo_dn_from_role(FSMORole(role)) == fsmo_role_dn) {
            return fsmo_role_to_string(FSMORole(role));
        }
    }
    return QString();
}

/**
 * Get a FSMO role from the given distinguished name (DN.)
 *
 * @param[in] role_dn A role distinguished name (DN) to convert.
 * @param[out] role_out A reference to the output FSMO role to store
 * the results.
 * @return True if the DN successfully converted, false otherwise.
 */
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

/**
 * Change the owner of the given FSMO role.
 *
 * @param ad An AD interface instance.
 * @param root_dse Root Directory Server Agent Service Entry (RootDSE.)
 * See <https://ldapwiki.com/wiki/Wiki.jsp?page=RootDSE>
 * @param role_dn A role distinguished name (DN.)
 */
bool fsmo_set_master(AdInterface& ad,
                     const AdObject &root_dse,
                     const QString &role_dn) {
    const QString new_master_service =
        root_dse.get_string(ATTRIBUTE_DS_SERVICE_NAME);
    return ad.attribute_replace_string(role_dn,
                                       ATTRIBUTE_FSMO_ROLE_OWNER,
                                       new_master_service);
}

/**
 * Get the mapping between FSMO roles and their names.
 *
 * @return A QHash table with the role mapping.
 */
QHash<FSMORole, QString> fsmo_role_mapping() {
    auto tr = [](const char *name) {
        return QCoreApplication::translate("FSMODialog", name);
    };
    return {
        { FSMORole_DomainDNS,      tr("Domain DNS") },
        { FSMORole_ForestDNS,      tr("Forest DNS") },
        { FSMORole_PDCEmulation,   tr("PDC Emulation") },
        { FSMORole_Schema,         tr("Schema") },
        { FSMORole_DomainNaming,   tr("Domain Naming") },
        { FSMORole_Infrastructure, tr("Infrastructure") },
        { FSMORole_RidAllocation,  tr("Rid Allocation") },
    };
}

/**
 * Iterate over FSMO roles (@see fsmo_role_mapping()), call a callback procedure
 * over each role.
 *
 * @param callback A procedure to call.
 */
void fsmo_role_for_each(
    const function<void(FSMORole, const QString&, const QString&)>& callback)
{
    const QHash<FSMORole, QString> role_mapping = fsmo_role_mapping();
    for (int role_i = 0; role_i < FSMORole_COUNT; role_i++) {
        const FSMORole role = (FSMORole) role_i;
        const QString title = role_mapping[role];
        const QString role_dn = fsmo_dn_from_role(role);
        callback(role, title, role_dn);
    }
}

/**
 * Setup a FSMO dialog geometry from the stored values.
 *
 * @param dialog An FSMO dialog to use.
 */
void fsmo_setup_dialog_geometry(QDialog *dialog) {
    settings_setup_dialog_geometry(SETTING_fsmo_dialog_geometry, dialog);
}
