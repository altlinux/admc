/*
 * GPGUI - Group Policy Editor GUI
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
#if !defined(__GPTBACKEND_PREG_WRITER)
#define __GPTBACKEND_PREG_WRITER 1

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include "preg_data.h"

namespace preg {

class preg_writer {
    std::ofstream preg_file;
    char preg_magic[4]{ 'P', 'R', 'e', 'g' };
    char preg_version[4]{ '\x01', '\x00', '\x00', '\x00' };

public:
    preg_writer(std::string &preg_file);
    ~preg_writer();

    void add_entry(preg::entry &pentry);

    void close();

private:
    void preg_type2buf(uint16_t type);
}; /* class preg_writer */

} /* namespace preg */

#endif /* namespace preg */

