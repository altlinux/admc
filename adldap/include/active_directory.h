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

#ifndef ACTIVE_DIRECTORY_H
#define ACTIVE_DIRECTORY_H 1

/* Error codes */
#define AD_SUCCESS 1
#define AD_COULDNT_OPEN_CONFIG_FILE 2
#define AD_MISSING_CONFIG_PARAMETER 3
#define AD_SERVER_CONNECT_FAILURE 4
#define AD_LDAP_OPERATION_FAILURE 5
#define AD_OBJECT_NOT_FOUND 6
#define AD_ATTRIBUTE_ENTRY_NOT_FOUND 7
#define AD_INVALID_DN 8
#define AD_RESOLV_ERROR 9

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/**
 * Output a pointer to a string containing an explanation
 * of the last error that occured. If no error has previously occured
 * the string the contents are undefined. Subsequent active directory
 * library calls may over-write this string.
 */
const char *ad_get_error();

/**
 * Output a list of hosts that exist for given domain and site
 * list is NULL terminated
 * list should be freed by the caller using ad_array_free()
 * Returns AD_SUCCESS or error code
 */
int ad_get_domain_hosts(const char *domain, const char *site, char ***hosts_out);

/**
 * Connect and authenticate to Active Directory server
 * If connected succesfully saves connection handle into ds
 * Returns AD_SUCCESS or error code
 */
int ad_login(const char* uri, LDAP **ds);

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
 * Output a list of DN's which match the given filter and are
 * below the given search base
 * list is NULL terminated
 * list should be freed by the caller using ad_array_free()
 */
int ad_search(LDAP *ld, const char *filter, const char* search_base, char ***list_out);

/** 
 * Output a list of DN's that are one level below the given object
 * If none are found, list is allocated and is empty
 * list is NULL terminated
 * list should be freed by the caller using ad_array_free()
 * Returns AD_SUCCESS or error code
 */
int ad_list(LDAP *ld, const char *dn, char ***list_out);

/**
 * Output a NULL terminated array of values for the given attribute
 * Array should be freed by the caller using ad_array_free()
 * Returns AD_SUCCESS or error code
 */
int ad_get_attribute(LDAP *ld, const char *dn, const char *attribute, char ***values_out);

/**
 * Output a 2d NULL terminated array of attribute values for given
 * object
 * Each sub-array contains an attribute name followed by it's values
 * Output should be freed by the caller using ad_2d_array_free()
 * Returns AD_SUCCESS or error code
 */
int ad_get_all_attributes(LDAP *ld, const char *dn, char ****attributes_out);

/**
 * Create user object below given DN
 * New user object is locked by default
 * Returns AD_SUCCESS or error code
 */
int ad_create_user(LDAP *ld, const char *username, const char *dn);

/**
 * Create computer object below given DN
 * Returns AD_SUCCESS or error code
 */
int ad_create_computer(LDAP *ld, const char *name, const char *dn);

/**
 * Create an organizational unit
 * Returns AD_SUCCESS or error code
 */
int ad_create_ou(LDAP *ld, const char *ou_name, const char *dn);

/**
 * Create a group with given name below given DN
 * Returns AD_SUCCESS or error code
 */
int ad_create_group(LDAP *ld, const char *group_name, const char *dn);

/**
 * Delete object
 * Returns AD_SUCCESS or error code
 */
int ad_delete(LDAP *ld, const char *dn);

/**
 * Lock a user account
 * Returns AD_SUCCESS or error code
 */
int ad_user_lock(LDAP *ld, const char *dn);

/**
 * Unlock a disabled user account
 * Returns AD_SUCCESS or error code
 */
int ad_user_unlock(LDAP *ld, const char *dn);

/**
 * Set the user's password to the given password string
 * SSL connection is required
 * Returns AD_SUCCESS or error code
 */
int ad_user_set_pass(LDAP *ld, const char *dn, const char *password);

/**
 * Adds a value to given attribute
 * This function works only on multi-valued attributes
 * Returns AD_SUCCESS or error code
 */
int ad_attribute_add(LDAP *ld, const char *dn, const char *attribute, const char *value);

/**
 * Same as ad_attribute_add() but for binary data
 * Returns AD_SUCCESS or error code
 */
int ad_attribute_add_binary(LDAP *ld, const char *dn, const char *attribute, const char *data, int data_length);

/**
 * Replaces the value of given attribute with new value
 * If attributes has multiple values, all of them are replaced
 * Returns AD_SUCCESS or error code
 */
int ad_attribute_replace(LDAP *ld, const char *dn, const char *attribute, const char *value);

/**
 * Same as ad_mode_replace() but for binary data
 * Returns AD_SUCCESS or error code
 */
int ad_attribute_replace_binary(LDAP *ld, const char *dn, const char *attribute, const char *data, int data_length);

/**
 * Remove (attribute, value) mapping from object
 * If given value is NULL, remove all values of this attributes
 * Returns AD_SUCCESS or error code
 */
int ad_attribute_delete(LDAP *ld, const char *dn, const char *attribute, const char *value);

/**
 * Rename object
 * Use specialized functions to rename users and groups
 * Returns AD_SUCCESS or error code
 */
int ad_rename(LDAP *ld, const char *dn, const char *new_rdn);

/**
 * Rename user and update related attributes
 * Returns AD_SUCCESS or error code
 */
int ad_rename_user(LDAP *ld, const char *dn, const char *new_name);

/**
 * Rename group and update related attributes
 * Returns AD_SUCCESS or error code
 */
int ad_rename_group(LDAP *ld, const char *dn, const char *new_name);

/**
 * Move object
 * Use ad_move_user() for user objects
 * Returns AD_SUCCESS or error code
 */
int ad_move(LDAP *ld, const char *current_dn, const char *new_container);

/**
 * Move user and update attributes affected by move
 * Returns AD_SUCCESS or error code
 */
int ad_move_user(LDAP *ld, const char *current_dn, const char *new_container);

/**
 * Add user to a group
 * Returns AD_SUCCESS or error code
 */
int ad_group_add_user(LDAP *ld, const char *group_dn, const char *user_dn);

/**
 * Remove user from a group
 * Returns AD_SUCCESS or error code
 */
int ad_group_remove_user(LDAP *ld, const char *group_dn, const char *user_dn);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* ACTIVE_DIRECTORY_H */
