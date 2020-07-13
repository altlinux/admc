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

int AdConnection::connect(std::string uri_arg, std::string search_base_arg) {
    uri = uri_arg;
    search_base = search_base_arg;

    ldap_connection = NULL;
    const int result = ad_login(this->uri.c_str(), &ldap_connection);
    return result;
}

bool AdConnection::is_connected() {
    if (nullptr != this->ldap_connection) {
        return true;
    }
    return false;
}

const char *AdConnection::get_error() {
    return ad_get_error();
}

std::string AdConnection::get_search_base() const {
    return search_base;
}

std::string AdConnection::get_uri() const {
    return uri;
}

int AdConnection::create_user(const char *username, const char *dn) {
    return ad_create_user(ldap_connection, username, dn);
}

int AdConnection::create_computer(const char *name, const char *dn) {
    return ad_create_computer(ldap_connection, name, dn);
}

int AdConnection::user_lock(const char *dn) {
    return ad_user_lock(ldap_connection, dn);
}

int AdConnection::user_unlock(const char *dn) {
    return ad_user_unlock(ldap_connection, dn);
}

int AdConnection::object_delete(const char *dn) {
    return ad_delete(ldap_connection, dn);
}

int AdConnection::user_set_pass(const char *dn, const char *password) {
    return ad_user_set_pass(ldap_connection, dn, password);
}

int AdConnection::search(const char *filter, char ***dn_list) {
    return ad_search(ldap_connection, filter, this->search_base.c_str(), dn_list);
}

int AdConnection::attribute_add(const char *dn, const char *attribute, const char *value) {
    return ad_attribute_add(ldap_connection, dn, attribute, value);
}

int AdConnection::attribute_add_binary(const char *dn, const char *attribute, const char *data, int data_length) {
    return ad_attribute_add_binary(ldap_connection, dn, attribute, data, data_length);
}

int AdConnection::attribute_replace(const char *dn, const char *attribute, const char *value) {
    return ad_attribute_replace(ldap_connection, dn, attribute, value);
}

int AdConnection::attribute_replace_binary(const char *dn, const char *attribute, const char *data, int data_length) {
    return ad_attribute_replace_binary(ldap_connection, dn, attribute, data, data_length);
}

int AdConnection::attribute_delete(const char *dn, const char *attribute, const char *value) {
    return ad_attribute_delete(ldap_connection, dn, attribute, value);
}

int AdConnection::attribute_get(const char *dn, const char *attribute, char ***values) {
    return ad_attribute_get(ldap_connection, dn, attribute, values);
}

int AdConnection::rename(const char *dn, const char *new_name) {
    return ad_rename(ldap_connection, dn, new_name);
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

int AdConnection::create_group(const char *group_name, const char *dn) {
    return ad_create_group(ldap_connection, group_name, dn);
}

int AdConnection::group_add_user(const char *group_dn, const char *user_dn) {
    return ad_group_add_user(ldap_connection, group_dn, user_dn);
}

int AdConnection::group_remove_user(const char *group_dn, const char *user_dn) {
    return ad_group_remove_user(ldap_connection, group_dn, user_dn);
}

int AdConnection::create_ou(const char *ou_name, const char *dn) {
    return ad_create_ou(ldap_connection, ou_name, dn);
}

int AdConnection::list(const char *dn, char ***dn_list) {
    return ad_list(ldap_connection, dn, dn_list);
}

} /* namespace adldap */

