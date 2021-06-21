/*
 * GPGUI - Group Policy Editor GUI
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
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
#include "preg_writer.h"
#include <catch2/catch.hpp>

TEST_CASE("Test if PReg file may be written to disk", "[preg_writer]" /*"[!hide]"*/) {
    preg::entry pe;
    pe.value_name = "Software\\BaseALT\\Policies\\Control";
    pe.key_name = "sudo";
    pe.type = 4;
    pe.size = 5;
    pe.value = new char[5]{'T', 'e', 's', 't', '1'};

    std::string preg_path = "test.pol";

    {
        preg::preg_writer pw(preg_path);
        pw.add_entry(pe);
    }
}
