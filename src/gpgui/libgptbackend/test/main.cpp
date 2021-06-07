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
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "iconv_wrapper.h"
#include "preg_parser.h"

#include <string>

TEST_CASE("It is possible to convert UTF-8 string to UTF-16LE",
          "[iconv_wrapper]") {
    std::string test_str_utf8 = "Software\\BaseALT\\Policies\\Control";
    std::string test_str_utf16le{
        'S',  '\x00', 'o',  '\x00', 'f', '\x00', 't',  '\x00', 'w', '\x00',
        'a',  '\x00', 'r',  '\x00', 'e', '\x00', '\\', '\x00', 'B', '\x00',
        'a',  '\x00', 's',  '\x00', 'e', '\x00', 'A',  '\x00', 'L', '\x00',
        'T',  '\x00', '\\', '\x00', 'P', '\x00', 'o',  '\x00', 'l', '\x00',
        'i',  '\x00', 'c',  '\x00', 'i', '\x00', 'e',  '\x00', 's', '\x00',
        '\\', '\x00', 'C',  '\x00', 'o', '\x00', 'n',  '\x00', 't', '\x00',
        'r',  '\x00', 'o',  '\x00', 'l', '\x00'};
    {
        gptbackend::iconv_wrapper iwrapper("UTF-8", "UTF-16LE");
        REQUIRE(test_str_utf16le == iwrapper.convert(test_str_utf8));
    }
}

TEST_CASE("It is possile to convert UTF-16LE string to UTF-8",
          "[iconv_wrapper]") {
    std::string test_str_utf8 = "Software\\BaseALT\\Policies\\Control";
    std::string test_str_utf16le{
        'S',  '\x00', 'o',  '\x00', 'f', '\x00', 't',  '\x00', 'w', '\x00',
        'a',  '\x00', 'r',  '\x00', 'e', '\x00', '\\', '\x00', 'B', '\x00',
        'a',  '\x00', 's',  '\x00', 'e', '\x00', 'A',  '\x00', 'L', '\x00',
        'T',  '\x00', '\\', '\x00', 'P', '\x00', 'o',  '\x00', 'l', '\x00',
        'i',  '\x00', 'c',  '\x00', 'i', '\x00', 'e',  '\x00', 's', '\x00',
        '\\', '\x00', 'C',  '\x00', 'o', '\x00', 'n',  '\x00', 't', '\x00',
        'r',  '\x00', 'o',  '\x00', 'l', '\x00'};
    {
        gptbackend::iconv_wrapper iwrapper2("UTF-16LE", "UTF-8");
        REQUIRE(test_str_utf8 == iwrapper2.convert(test_str_utf16le));
    }
}
