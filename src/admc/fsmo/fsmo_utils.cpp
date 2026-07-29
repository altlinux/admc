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
