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

//#include "config.h"
#include "ad_connection.h"

namespace adldap {

AdConnection::AdConnection() {
}

void AdConnection::connect(std::string uri, std::string search_base) {
    this->uri = uri;
    this->search_base = search_base;
    this->ldap_connection = ad_login(this->uri.c_str());
}

bool AdConnection::is_connected() {
    if (nullptr != this->ldap_connection) {
        return true;
    }
    return false;
}

char* AdConnection::get_errstr() {
    return ad_get_error();
}

} /* namespace adldap */

