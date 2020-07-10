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
    int connect(std::string uri_arg, std::string search_base_arg);
    bool is_connected();
    const char *get_error();
    std::string get_search_base() const;
    std::string get_uri() const;

    int create_user(const char *username, const char *dn);
    int create_computer(const char *name, const char *dn);
    int user_lock(const char *dn);
    int user_unlock(const char *dn);
    int object_delete(const char *dn);
    int user_set_pass(const char *dn, const char *password);
    int search(const char *filter, char ***dn_list);
    int attribute_add(const char *dn, const char *attribute, const char *value);
    int attribute_add_binary(const char *dn, const char *attribute, const char *data, int data_length);
    int attribute_replace(const char *dn, const char *attribute, const char *value);
    int attribute_replace_binary(const char *dn, const char *attribute, const char *data, int data_length);
    int attribute_delete(const char *dn, const char *attribute, const char *value);
    int attribute_get(const char *dn, const char *attribute, char ***values);
    int rename(const char *dn, const char *new_name);
    int rename_user(const char *dn, const char *new_username);
    int rename_group(const char *dn, const char *new_name);
    int move(const char *current_dn, const char *new_container);
    int move_user(const char *current_dn, const char *new_container);
    int create_group(const char *group_name, const char *dn);
    int group_add_user(const char *group_dn, const char *user_dn);
    int group_remove_user(const char *group_dn, const char *user_dn);
    int create_ou(const char *ou_name, const char *dn);
    int list(const char *dn, char ***dn_list);
};

} /* namespace adldap */

#endif /* __ADLDAP_AD_CONNECTION_H */

