/**
 * Copyright (c) by: Mike Dawson mike _at_ no spam gp2x.org
 *
 * This file may be used subject to the terms and conditions of the
 * GNU Library General Public License Version 2, or any later version
 * at your option, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
**/

#include <ldap.h>
#include <stdbool.h>

#ifndef ACTIVE_DIRECTORY_H
#define ACTIVE_DIRECTORY_H 1

// Result codes
#define AD_SUCCESS 0
#define AD_ERROR 1
#define AD_LDAP_ERROR 2
#define AD_INVALID_DN 3

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/**
 * Return a result code from last LDAP operation
 * When a library function returns AD_LDAP_ERROR, use this to get
 * the ldap error code
 */
int ad_get_ldap_result(LDAP *ld);

/**
 * Output a list of hosts that exist for given domain and site
 * list is NULL terminated
 * list should be freed by the caller using ad_array_free()
 * Returns AD_SUCCESS, AD_ERROR
 */
int ad_get_domain_hosts(const char *domain, const char *site, char ***hosts_out);

/**
 * Connect and authenticate to Active Directory server
 * If connected succesfully saves connection handle into ds
 * Returns AD_SUCCESS, AD_LDAP_ERROR
 */
int ad_login(const char* uri, LDAP **ld_out);

/**
 * Calculate size of null-terminated array by iterating through it
 */
size_t ad_array_size(char **array);

/**
 * Free a null-terminated array that was returned by one of
 * the functions in this library
 * If array is NULL, nothing is done
 */
void ad_array_free(char **array);

/**
 * Free a null-terminated array that was returned by one of
 * the functions in this library
 * If array is NULL, nothing is done
 */
void ad_2d_array_free(char ***array);

/**
 * Adds an object with given DN and objectClass
 * objectClass is a NULL terminated array of objectClass values
 * All ancestors of object must already exist
 * Returns AD_SUCCESS, AD_LDAP_ERROR
 */
int ad_add(LDAP *ld, const char *dn, const char **objectClass);

/**
 * Delete object
 * Returns AD_SUCCESS, AD_LDAP_ERROR
 */
int ad_delete(LDAP *ld, const char *dn);

/**
 * Adds a value to given attribute
 * This function works only on multi-valued attributes
 * Returns AD_SUCCESS, AD_LDAP_ERROR
 */
int ad_attribute_add_string(LDAP *ld, const char *dn, const char *attribute, const char *data, int data_length);

/**
 * Replaces the value of given attribute with new value
 * If attributes has multiple values, all of them are replaced
 * Returns AD_SUCCESS, AD_LDAP_ERROR
 */
int ad_attribute_replace_string(LDAP *ld, const char *dn, const char *attribute, const char *data, int data_length);

/**
 * Remove (attribute, value) mapping from object
 * If given value is NULL, remove all values of this attributes
 * Returns AD_SUCCESS, AD_LDAP_ERROR
 */
int ad_attribute_delete_string(LDAP *ld, const char *dn, const char *attribute, const char *data, const int data_length);

/**
 * Rename object
 * Returns AD_SUCCESS, AD_LDAP_ERROR
 */
int ad_rename(LDAP *ld, const char *dn, const char *new_rdn);

/**
 * Move object
 * Returns AD_SUCCESS, AD_LDAP_ERROR
 */
int ad_move(LDAP *ld, const char *current_dn, const char *new_container);

int ad_get_all_attributes_internal(LDAP *ld, const char *dn, const char *attribute, LDAPMessage **res_out);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* ACTIVE_DIRECTORY_H */
