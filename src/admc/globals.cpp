/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#include "globals.h"

#include "adldap.h"
#include "settings.h"
#include "status.h"
#include "managers/icon_manager.h"
#include "managers/gplink_manager.h"

#include <QLocale>


AdConfig *g_adconfig = new AdConfig();
Status *g_status = new Status();
IconManager *g_icon_manager = new IconManager();
GPLinkManager *g_gplink_manager = new GPLinkManager();

void load_g_adconfig(AdInterface &ad) {
    const QLocale locale = settings_get_variant(SETTING_locale).toLocale();
    g_adconfig->load(ad, locale);
    AdInterface::set_config(g_adconfig);
}
