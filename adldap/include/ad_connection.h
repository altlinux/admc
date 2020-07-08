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
    void connect(std::string uri_arg, std::string domain);
    bool is_connected();
    char* get_errstr();
    int get_errcode();
    const std::string get_search_base() const;

    int create_user(const char *username, const char *dn);
    int create_computer(const char *name, const char *dn);
    int lock_user(const char *dn);
    int unlock_user(const char *dn);
    int object_delete(const char *dn);
    int setpass(const char *dn, const char *password);
    char **search(const char *filter);
    int mod_add(const char *dn, const char *attribute, const char *value);
    int mod_add_binary(const char *dn, const char *attribute, const char *data, int data_length);
    int mod_replace(const char *dn, const char *attribute, const char *value);
    int mod_replace_binary(const char *dn, const char *attribute, const char *data, int data_length);
    int mod_delete(const char *dn, const char *attribute, const char *value);
    char **get_attribute(const char *dn, const char *attribute);
    int rename(const char *dn, const char *new_name);
    int rename_user(const char *dn, const char *new_username);
    int rename_group(const char *dn, const char *new_name);
    int move(const char *current_dn, const char *new_container);
    int move_user(const char *current_dn, const char *new_container);
    int group_create(const char *group_name, const char *dn);
    int group_add_user(const char *group_dn, const char *user_dn);
    int group_remove_user(const char *group_dn, const char *user_dn);
    int group_subtree_remove_user(const char *container_dn, const char *user_dn);
    int ou_create(const char *ou_name, const char *dn);
    char **list(const char *dn);
};

} /* namespace adldap */

#endif /* __ADLDAP_AD_CONNECTION_H */

