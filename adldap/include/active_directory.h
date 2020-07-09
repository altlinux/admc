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

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/* ad_get_error() returns a pointer to a string containing an
| explanation of the last error that occured.
|  If no error has previously occured the string the contents are 
| undefined.
|  Subsequent active directory library calls may over-write this
| string.
*/
char *ad_get_error();

/* ad_get_error_num() returns the integer code for the last error
| that occured.
|  If no function calls have previously failed the result is undefined.
|  See the end of this header file for the list of error codes.
*/
int ad_get_error_num();

/**
 * Free a null-terminated array that was returned by one of
 * the functions in this library
 * NULL check of array pointer is performed inside
 */
void ad_array_free(char **array);

/**
 * Calculate size of null-terminated array by iterating through it
 */
size_t ad_array_size(char **array);

/**
 * Return a null-terminated array of hosts that exist for given
 * domain and shit
 */
int ad_get_domain_hosts(const char *domain, const char *site, char ***hosts);

/* ad_create_user() creates a new, locked user account
| with the given user name and distinguished name
|  Example usage: 
| ad_create_user("nobody", "cn=nobody,ou=users,dc=example,dc=com");
|  Returns AD_SUCCESS on success, or
| AD_LDAP_OPERATION_FAILURE.
|  Attributes set:
|   objectClass=user
|   sAMAccountName=username
|   userPrincipalName=username@<domain> (derived from dn)
|   userAccountControl=66050
| (ACCOUNTDISABLE|NORMAL_ACCOUNT|DONT_EXPIRE_PASSWORD)
| see http://msdn.microsoft.com/library/default.asp?url=/library/en-us/netdir/adsi/ads_user_flag_enum.asp for flags.
|  Attributes set automatically by the directory:
|   objectclass=top,person,organizationalPerson
|   accountExpires,instanceType,objectCategory,objectGUID,
|   objectSid,
|   primaryGroupID=513
|   name=username
|   sAMAccountType=805306368
|   uSNChanged,uSNCreated,whenChanged,whenCreated

ad_create_user("new-user", "CN=new-user,CN=Users,DC=domain,DC=alt");
*/
int ad_create_user(LDAP *ds, const char *username, const char *dn);

/* create computer object */
int ad_create_computer(LDAP *ds, const char *name, const char *dn);

/* ad_lock_user() disables a user account
|  Returns AD_SUCCESS, AD_OBJECT_NOT_FOUND or AD_LDAP_OPERATIONS_FAILURE.
*/
int ad_lock_user(LDAP *ds, const char *dn);

/* ad_unlock_user() unlocks a disabled user account
|  Returns AD_SUCCESS, AD_OBJECT_NOT_FOUND or AD_LDAP_OPERATIONS_FAILURE.
*/
int ad_unlock_user(LDAP *ds, const char *dn);

/* ad_object_delete() deletes the given dn
|  Returns AD_SUCCESS or AD_LDAP_OPERATION_FAILURE.
*/
int ad_object_delete(LDAP *ds, const char *dn);

/* ad_setpass() sets the user's password to the password string given
|  This requires an ssl connection to work 
| (use a uri of ldaps:// rather than ldap:// in the configuration file)
|  Returns AD_SUCCESS or AD_LDAP_OPERATION_FAILURE.
*/
int ad_setpass(LDAP *ds, const char *dn, const char *password);

/* ad_search() is a general search function
|  Returns a NULL terminated array of dns which match the given filter
|  or NULL if no results are found.  
|  Returns -1 on error.
|  Sets error code to AD_SUCCESS, AD_OBJECT_NOT_FOUND 
| or AD_LDAP_OPERATION_FAILURE.
|  Searching is done from the searchbase specified in the configuration
| file.
*/
char **ad_search(LDAP *ds, const char *filter, const char* search_base);

/* ad_mod_add() adds a value to the given attribute.
| Example ad_mod_add("cn=nobody,ou=users,dc=example,dc=com",
|       "mail", "nobody@nowhere");
|  This function works only on multi-valued attributes.
|  Returns AD_SUCCESS on success.
*/
int ad_mod_add(LDAP *ds, const char *dn, const char *attribute, const char *value);

/* ad_mod_add_binary()
|  Works the same as ad_mod_add() except for binary data.
|  Returns AD_SUCCESS or AD_LDAP_OPERATION_FAILURE.
*/
int ad_mod_add_binary(LDAP *ds, const char *dn, const char *attribute, const char *data, int data_length);

/* ad_mod_replace() overwrites the given attribute with a new value.
| Example ad_mod_replace("cn=nobody,ou=users,dc=example,dc=com",
|       "description", "some person");
|  On multi-valued attributes this replaces all values.
|  Returns AD_SUCCESS or AD_LDAP_OPERATION_FAILURE.
*/
int ad_mod_replace(LDAP *ds, const char *dn, const char *attribute, const char *value);

/* ad_mod_replace_binary()
|  Works the same as ad_mod_replace() except for binary data.
|  Returns AD_SUCCESS or AD_LDAP_OPERATION_FAILURE.
*/
int ad_mod_replace_binary(LDAP *ds, const char *dn, const char *attribute, const char *data, int data_length);

/* ad_mod_delete() removes attribute data from an object.
|  If user nobody has 'othertelephone' numbers
| '123' and '456' then ad_mod_delete(dn, "othertelephone", "123"
| will delete just the number '123', whereas:
| ad_mod_delete(dn, "othertelephone", NULL)
| will delete both numbers.
|  Returns AD_SUCCESS or AD_LDAP_OPERATION_FAILURE.
*/
int ad_mod_delete(LDAP *ds, const char *dn, const char *attribute, const char *value);

/* ad_get_attribute() returns a pointer to a NULL terminated
| array of strings containing values for the given attribute.
|  Returns NULL on failure or if nothing is found.
|  Sets error code to AD_SUCCESS, AD_OBJECT_NOT_FOUND, 
| AD_ATTRIBUTE_ENTRY_NOT_FOUND or AD_LDAP_OPERATION_FAILURE
| even if there are no values for the given attribute.
*/
char **ad_get_attribute(LDAP *ds, const char *dn, const char *attribute);

// Renames object at dn
// new_rdn has to have appropriate prefix and be of the form "CN=name"
// Modifies name
// Use specialized functions to rename users and groups
int ad_mod_rename(LDAP *ds, const char *dn, const char *new_rdn);

// Change given user's dn
// Modifies cn, name, sAMAccountName and userPrincipalName
// new_name should be without prefix "CN="
int ad_rename_user(LDAP *ds, const char *dn, const char *new_name);

// Change given group's dn
// Modifies cn, name and sAMAccountName
// new_name should be without prefix "CN="
int ad_rename_group(LDAP *ds, const char *dn, const char *new_name);

// Moves object to new container
// Use specialized functions to rename users and groups
int ad_move(LDAP *ds, const char *current_dn, const char *new_container);

// Moves user to new container
// Modifies userPrincipalName
int ad_move_user(LDAP *ds, const char *current_dn, const char *new_container);

/* ad_group_create() creates a new user group (of type global security)
|  Example ad_group_create("administrators",
|   "cn=administrators,ou=admin,dc=example,dc=com");
|  Sets objectclass=group,
| sAMAccountName=group name
|  The directory automatically sets:
| objectclass=top
| groupType,instanceType,objectCategory,objectGUID,objectSid,
| name,sAMAccountType,uSNChanged,uSNCreated,whenChanged,whenCreated
|  Returns AD_SUCCESS or AD_LDAP_OPERATION_FAILURE.
*/
int ad_group_create(LDAP *ds, const char *group_name, const char *dn);

/* ad_group_add_user()
| adds a user to a group
| Returns AD_SUCCESS or AD_LDAP_OPERATION_FAILURE.
*/
int ad_group_add_user(LDAP *ds, const char *group_dn, const char *user_dn);

/* ad_group_remove_user()
| removes a user from a group
| Returns AD_SUCCESS or AD_LDAP_OPERATION_FAILURE.
*/
int ad_group_remove_user(LDAP *ds, const char *group_dn, const char *user_dn);

/* ad_ou_create()
|  Create an organizational unit
|  Sets objectclass=organizationalUnit
|  Returns AD_SUCCESS or AD_LDAP_OPERATION_FAILURE.
*/
int ad_ou_create(LDAP *ds, const char *ou_name, const char *dn);

/* ad_list()
|  Return NULL terminated array of entries
*/
char **ad_list(LDAP *ds, const char *dn);

/**
 * Connect and authenticate to Active Directory server
 * If connected succesfully saves connection handle into ds
 */
int ad_login(const char* uri, LDAP **ds);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

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

#endif /* ACTIVE_DIRECTORY_H */
