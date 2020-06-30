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

#include <iostream>

namespace adldap {

AdConnection::AdConnection() {
}

void AdConnection::connect(std::string uri_, std::string search_base_) {
    this->uri = uri_;
    this->search_base = search_base_;
    this->ldap_connection = ad_login(this->uri.c_str());

    AdArray hosts = {0};
    int hosts_result = ad_get_domain_hosts("DOMAIN.ALT", "SITE", &hosts);
    for (size_t i = 0; i < hosts.size; i++) {
        const char *host = ad_array_get(&hosts, i);
        printf("%s\n", host);
    }
    ad_array_free(&hosts);
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

int AdConnection::get_errcode() {
    return ad_get_error_num();
}

int AdConnection::create_user(const char *username, const char *dn) {
    return ad_create_user(ldap_connection, username, dn);
}

int AdConnection::create_computer(const char *name, const char *dn) {
    return ad_create_computer(ldap_connection, name, dn);
}

int AdConnection::lock_user(const char *dn) {
    return ad_lock_user(ldap_connection, dn);
}

int AdConnection::unlock_user(const char *dn) {
    return ad_unlock_user(ldap_connection, dn);
}

int AdConnection::object_delete(const char *dn) {
    return ad_object_delete(ldap_connection, dn);
}

int AdConnection::setpass(const char *dn, const char *password) {
    return ad_setpass(ldap_connection, dn, password);
}

char** AdConnection::search(const char *attribute, const char *value) {
    return ad_search(ldap_connection, attribute, value, this->search_base.c_str());

}

int AdConnection::mod_add(const char *dn, const char *attribute, const char *value) {
    return ad_mod_add(ldap_connection, dn, attribute, value);
}

int AdConnection::mod_add_binary(const char *dn, const char *attribute, const char *data, int data_length) {
    return ad_mod_add_binary(ldap_connection, dn, attribute, data, data_length);
}

int AdConnection::mod_replace(const char *dn, const char *attribute, const char *value) {
    return ad_mod_replace(ldap_connection, dn, attribute, value);
}

int AdConnection::mod_replace_binary(const char *dn, const char *attribute, const char *data, int data_length) {
    return ad_mod_replace_binary(ldap_connection, dn, attribute, data, data_length);
}

int AdConnection::mod_delete(const char *dn, const char *attribute, const char *value) {
    return ad_mod_delete(ldap_connection, dn, attribute, value);
}

char** AdConnection::get_attribute(const char *dn, const char *attribute) {
    return ad_get_attribute(ldap_connection, dn, attribute);
}

int AdConnection::rename(const char *dn, const char *new_name) {
    return ad_mod_rename(ldap_connection, dn, new_name);
}

int AdConnection::rename_user(const char *dn, const char *new_username) {
    return ad_rename_user(ldap_connection, dn, new_username);
}

int AdConnection::rename_group(const char *dn, const char *new_name) {
    return ad_rename_group(ldap_connection, dn, new_name);
}

int AdConnection::move(const char *current_dn, const char *new_container) {
    return ad_move(ldap_connection, current_dn, new_container);
}

int AdConnection::move_user(const char *current_dn, const char *new_container) {
    return ad_move_user(ldap_connection, current_dn, new_container);
}

int AdConnection::group_create(const char *group_name, const char *dn) {
    return ad_group_create(ldap_connection, group_name, dn);
}

int AdConnection::group_add_user(const char *group_dn, const char *user_dn) {
    return ad_group_add_user(ldap_connection, group_dn, user_dn);
}

int AdConnection::group_remove_user(const char *group_dn, const char *user_dn) {
    return ad_group_remove_user(ldap_connection, group_dn, user_dn);
}

int AdConnection::group_subtree_remove_user(const char *container_dn, const char *user_dn) {
    return ad_group_subtree_remove_user(ldap_connection, container_dn, user_dn);
}

int AdConnection::ou_create(const char *ou_name, const char *dn) {
    return ad_ou_create(ldap_connection, ou_name, dn);
}

char** AdConnection::list(const char *dn) {
    return ad_list(ldap_connection, dn);
}

} /* namespace adldap */

