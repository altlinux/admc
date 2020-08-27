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

#if !defined(__ADLDAP_AD_CONNECTION_H)
#define __ADLDAP_AD_CONNECTION_H 1

#include "active_directory.h"

#include <string>

namespace adldap {

class AdConnection {
    std::string uri;
    std::string search_base;
    LDAP* ldap_connection = nullptr;

public:
    AdConnection();
    int connect(std::string uri_arg, std::string domain);
    bool is_connected();
    std::string get_search_base() const;
    std::string get_uri() const;
    int get_ldap_result() const;

    int add(const char *dn, const char **objectClass);
    int object_delete(const char *dn);
    int search(const char *filter, char ***dn_list);
    int attribute_add(const char *dn, const char *attribute, const char *value);
    int attribute_add_binary(const char *dn, const char *attribute, const char *data, int data_length);
    int attribute_replace(const char *dn, const char *attribute, const char *value);
    int attribute_replace_binary(const char *dn, const char *attribute, const char *data, int data_length);
    int attribute_delete(const char *dn, const char *attribute, const char *value);
    int get_all_attributes(const char *dn, char ****attributes);
    int rename(const char *dn, const char *new_name);
    int move(const char *current_dn, const char *new_container);
    int group_add_user(const char *group_dn, const char *user_dn);
    int group_remove_user(const char *group_dn, const char *user_dn);
    int list(const char *dn, char ***dn_list);
};

} /* namespace adldap */

#endif /* __ADLDAP_AD_CONNECTION_H */

