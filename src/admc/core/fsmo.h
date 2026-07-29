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

#ifndef CORE_FSMO_H
#define CORE_FSMO_H

#include <QString>
#include <functional>

#include "ad_interface.h"

enum FSMORole {
    FSMORole_DomainDNS,
    FSMORole_ForestDNS,
    FSMORole_PDCEmulation,
    FSMORole_Schema,
    FSMORole_DomainNaming,
    FSMORole_Infrastructure,
    FSMORole_RidAllocation,

    FSMORole_COUNT,
};

QString fsmo_dn_from_role(FSMORole role);
QString fsmo_role_to_string(FSMORole role);
bool fsmo_is_current_dc_master_for_role(AdInterface &ad, FSMORole role);

#endif  /* ifndef CORE_FSMO_H */
