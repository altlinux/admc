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
#include "preg_data.h"

std::string preg::regtype2str(uint32_t &regtype) {
    std::string result = "UNKNOWN";

    switch (regtype) {
    case preg::REG_NONE: {
        result = "REG_NONE";
        break;
    }
    case preg::REG_SZ: {
        result = "REG_SZ";
        break;
    }
    case preg::REG_EXPAND_SZ: {
        result = "REG_EXPAND_SZ";
        break;
    }
    case preg::REG_BINARY: {
        result = "REG_BINARY";
        break;
    }
    case preg::REG_DWORD_LITTLE_ENDIAN: {
        result = "REG_DWORD_LITTLE_ENDIAN";
        break;
    }
    case preg::REG_DWORD_BIG_ENDIAN: {
        result = "REG_DWORD_BIG_ENDIAN";
        break;
    }
    case preg::REG_LINK: {
        result = "REG_LINK";
        break;
    }
    case preg::REG_MULTI_SZ: {
        result = "REG_MULTI_SZ";
        break;
    }
    case preg::REG_RESOURCE_LIST: {
        result = "REG_RESOURCE_LIST";
        break;
    }
    case preg::REG_FULL_RESOURCE_DESCRIPTOR: {
        result = "REG_FULL_RESOURCE_DESCRIPTOR";
        break;
    }
    case preg::REG_RESOURCE_REQUIREMENTS_LIST: {
        result = "REG_RESOURCE_REQUIREMENTS_LIST";
        break;
    }
    case preg::REG_QWORD: {
        result = "REG_QWORD";
        break;
    }
    case preg::REG_QWORD_LITTLE_ENDIAN: {
        result = "REG_QWORD_LITTLE_ENDIAN";
        break;
    }
    default: {
        result = "UNKNOWN";
        break;
    }
    } /* switch (regtype) */

    return result;
} /* std::string preg::regtype2str(uint16_t &regtype) */

uint32_t preg::str2regtype(std::string &regtype) {
    uint32_t result = 0;

    if ("REG_NONE" == regtype) {
        result = preg::REG_NONE;
    }
    if ("REG_SZ" == regtype) {
        result = preg::REG_SZ;
    }
    if ("REG_EXPAND_SZ" == regtype) {
        result = preg::REG_EXPAND_SZ;
    }
    if ("REG_BINARY" == regtype) {
        result = preg::REG_BINARY;
    }
    if ("REG_DWORD_LITTLE_ENDIAN" == regtype || "REG_DWORD" == regtype) {
        result = preg::REG_DWORD_LITTLE_ENDIAN;
    }
    if ("REG_DWORD_BIG_ENDIAN" == regtype) {
        result = preg::REG_DWORD_BIG_ENDIAN;
    }
    if ("REG_LINK" == regtype) {
        result = preg::REG_LINK;
    }
    if ("REG_MULTI_SZ" == regtype) {
        result = preg::REG_MULTI_SZ;
    }
    if ("REG_RESOURCE_LIST" == regtype) {
        result = preg::REG_RESOURCE_LIST;
    }
    if ("REG_FULL_RESOURCE_DESCRIPTOR" == regtype) {
        result = preg::REG_FULL_RESOURCE_DESCRIPTOR;
    }
    if ("REG_RESOURCE_REQUIREMENTS_LIST" == regtype) {
        result = preg::REG_RESOURCE_REQUIREMENTS_LIST;
    }
    if ("REG_QWORD" == regtype) {
        result = preg::REG_QWORD;
    }
    if ("REG_QWORD_LITTLE_ENDIAN" == regtype) {
        result = preg::REG_QWORD_LITTLE_ENDIAN;
    }

    return result;
} /* uint16_t preg::str2regtype(std::string &regtype) */

const char *preg::invalid_magic::what() const throw() {
    return "Invalid PReg file magic value";
}

const char *preg::invalid_version::what() const throw() {
    return "Invalid PReg file version";
}

const char *preg::no_more_entries::what() const throw() {
    return "No more PReg entries";
}

