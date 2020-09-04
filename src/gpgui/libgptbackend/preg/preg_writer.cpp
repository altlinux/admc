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
#include "preg_writer.h"
#include "iconv_wrapper.h"

preg::preg_writer::preg_writer(std::string &preg_file) {
    this->preg_file = std::ofstream(preg_file, std::ios::out | std::ios::binary);
    this->preg_file.write(this->preg_magic, 4);
    this->preg_file.write(this->preg_version, 4);
    // FIXME: Throw exception if not this->preg_file.good()
} /* preg::preg_writer::preg_writer() */

preg::preg_writer::~preg_writer() {
    this->close();
} /* preg::preg_writer::~preg_writer() */

void preg::preg_writer::close() {
    if (this->preg_file) {
        this->preg_file.close();
    }
} /* preg::preg_writer::close() */

void preg::preg_writer::add_entry(preg::entry &pentry) {
    char null_terminator[2]{ '\x00', '\x00' };
    char separator[2]{ ';', '\x00' };
    char range_start[2]{ '[', '\x00' };
    char range_end[2]{ ']', '\x00' };

    gptbackend::iconv_wrapper iw("UTF-8", "UTF-16LE");

    std::string conv_value_name = iw.convert(pentry.value_name);
    std::string conv_key_name = iw.convert(pentry.key_name);

    const char *value_name = conv_value_name.c_str();
    size_t value_name_size = conv_value_name.length() * sizeof(char);

    const char *key_name = conv_key_name.c_str();
    size_t key_name_size = conv_key_name.length() * sizeof(char);

    char type[2];
    type[0] = pentry.type & 0xFF;
    type[1] = pentry.type >> 8;

    char size[4];
    size[0] = pentry.size & 0xFF;
    size[1] = pentry.size >> 8;
    size[2] = pentry.size >> 16;
    size[3] = pentry.size >> 24;

    // FIXME: Make streambuf out of data and then write it at once.
    this->preg_file.write(range_start, 2);
    this->preg_file.write(value_name, value_name_size);
    this->preg_file.write(null_terminator, 2);
    this->preg_file.write(separator, 2);
    this->preg_file.write(key_name, key_name_size);
    this->preg_file.write(null_terminator, 2);
    this->preg_file.write(separator, 2);
    this->preg_file.write(type, 2);
    this->preg_file.write(null_terminator, 2);
    this->preg_file.write(separator, 2);
    this->preg_file.write(size, 4);
    this->preg_file.write(separator, 2);
    this->preg_file.write(pentry.value, pentry.size);
    this->preg_file.write(range_end, 2);
} /* void preg::preg_writer::add_entry() */

