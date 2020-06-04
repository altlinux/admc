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

const std::string &AdConnection::get_search_base() {
    return search_base;
}

int AdConnection::create_user(const char *username, const char *dn) {
    return ad_create_user(username, dn, this->uri.c_str());
}

int AdConnection::create_computer(const char *name, const char *dn) {
    return ad_create_computer(name, dn, this->uri.c_str());
}

int AdConnection::lock_user(const char *dn) {
    return ad_lock_user(dn, this->uri.c_str());
}

int AdConnection::unlock_user(const char *dn) {
    return ad_unlock_user(dn, this->uri.c_str());
}

int AdConnection::object_delete(const char *dn) {
    return ad_object_delete(dn, this->uri.c_str());
}

int AdConnection::setpass(const char *dn, const char *password) {
    return ad_setpass(dn, password, this->uri.c_str());
}

char** AdConnection::search(const char *attribute, const char *value) {
    return ad_search(attribute, value, this->search_base.c_str(), this->uri.c_str());
}

int AdConnection::mod_add(const char *dn, const char *attribute, const char *value) {
    return ad_mod_add(dn, attribute, value, this->uri.c_str());
}

int AdConnection::mod_add_binary(const char *dn, const char *attribute, const char *data, int data_length) {
    return ad_mod_add_binary(dn, attribute, data, data_length, this->uri.c_str());
}

int AdConnection::mod_replace(const char *dn, const char *attribute, const char *value) {
    return ad_mod_replace(dn, attribute, value, this->uri.c_str());
}

int AdConnection::mod_replace_binary(const char *dn, const char *attribute, const char *data, int data_length) {
    return ad_mod_replace_binary(dn, attribute, data, data_length, this->uri.c_str());
}

int AdConnection::mod_delete(const char *dn, const char *attribute, const char *value) {
    return ad_mod_delete(dn, attribute, value, this->uri.c_str());
}

char** AdConnection::get_attribute(const char *dn, const char *attribute) {
    return ad_get_attribute(dn, attribute, this->uri.c_str());
}

int AdConnection::rename_user(const char *dn, const char *new_username) {
    return ad_rename_user(dn, new_username, this->uri.c_str());
}

int AdConnection::move_user(const char *current_dn, const char *new_container) {
    return ad_move_user(current_dn, new_container, this->uri.c_str());
}

int AdConnection::group_create(const char *group_name, const char *dn) {
    return ad_group_create(group_name, dn, this->uri.c_str());
}

int AdConnection::group_add_user(const char *group_dn, const char *user_dn) {
    return ad_group_add_user(group_dn, user_dn, this->uri.c_str());
}

int AdConnection::group_remove_user(const char *group_dn, const char *user_dn) {
    return ad_group_remove_user(group_dn, user_dn, this->uri.c_str());
}

int AdConnection::group_subtree_remove_user(const char *container_dn, const char *user_dn) {
    return ad_group_subtree_remove_user(container_dn, user_dn, this->uri.c_str());
}

int AdConnection::ou_create(const char *ou_name, const char *dn) {
    return ad_ou_create(ou_name, dn, this->uri.c_str());
}

char** AdConnection::list(const char *dn) {
    return ad_list(dn, this->uri.c_str());
}

} /* namespace adldap */

