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

#include "admc.h"
#include "ad_interface.h"
#include "settings.h"

ADMC::ADMC(int& argc, char** argv)
: QApplication(argc, argv)
{
    m_ad_interface = new AdInterface(this);
    m_settings = new Settings(this);
}

AdInterface *ADMC::ad_interface() {
    return m_ad_interface;
}

Settings *ADMC::settings() {
    return m_settings;
}
