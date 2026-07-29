/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2023-2026 BaseALT Ltd.
 * Copyright (C) 2023-2025 Semyon Knyazev
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

#include "fsmo_utils.h"

#include "ad_config.h"
#include "core/ad.h"
#include "core/fsmo.h"
#include "core/globals.h"
#include "adldap.h"
#include "utils.h"
#include "core/settings.h"
#include "console_widget/console_widget.h"
#include "status.h"

#include <QString>
#include <QModelIndex>

bool gpo_edit_without_PDC_disabled = true;

void connect_to_PDC_emulator(AdInterface &ad, ConsoleWidget *console)
{
    fsmo_connect_host_with_role(ad, FSMORole_PDCEmulation);
    console->refresh_scope(console->domain_info_index());
    g_status->add_message(QObject::tr("PDC-Emulator is connected"),
                          StatusType_Success);
}

QString fsmo_string_from_dn(const QString &fsmo_role_dn)
{
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

    if (dn_lower == QString("cn=infrastructure,dc=domaindnszones,%1").arg(domain_dn).toLower()) {
        role_out = FSMORole_DomainDNS;
        return true;
    }

    if (dn_lower == QString("cn=infrastructure,dc=forestdnszones,%1").arg(domain_dn).toLower()) {
        role_out = FSMORole_ForestDNS;
        return true;
    }

    if (dn_lower == QString("cn=infrastructure,%1").arg(domain_dn).toLower()) {
        role_out = FSMORole_Infrastructure;
        return true;
    }

    if (dn_lower == QString("cn=rid manager$,cn=system,%1").arg(domain_dn).toLower()) {
        role_out = FSMORole_RidAllocation;
        return true;
    }

     return false;
}
