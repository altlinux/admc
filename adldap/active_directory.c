/**
 * Copyright (c) by: Mike Dawson mike _at_ no spam gp2x.org
 * Copyright (C) 2020 BaseALT Ltd.
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

#include "active_directory.h"
#include <ldap.h>
#include <sasl/sasl.h>
#include <lber.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <resolv.h>
#include <stdbool.h>

// NOTE: LDAP library char* inputs are non-const in the API but are
// actually const so we opt to discard const qualifiers rather
// than allocate copies

#ifdef __GNUC__
#  define UNUSED(x) x __attribute__((unused))
#else
#  define UNUSED(x) x
#endif

#define MAX_DN_LENGTH 1024
#define MAX_PASSWORD_LENGTH 255

typedef struct sasl_defaults_gssapi {
    char *mech;
    char *realm;
    char *authcid;
    char *passwd;
    char *authzid;
} sasl_defaults_gssapi;

int dn2domain(const char *dn, char **domain_out);
int sasl_interact_gssapi(LDAP *ld, unsigned flags, void *indefaults, void *in);
int query_server_for_hosts(const char *dname, char ***hosts_out);
int ad_get_all_attributes_internal(LDAP *ld, const char *dn, const char *attribute, LDAPMessage **res_out);

int ad_get_ldap_result(LDAP *ld) {
    int result;
    ldap_get_option(ld, LDAP_OPT_RESULT_CODE, &result);

    return result;
}

int ad_get_domain_hosts(const char *domain, const char *site, char ***hosts_out) {
    int result = AD_SUCCESS; 

    char **hosts = NULL;
    char **site_hosts = NULL;
    char **default_hosts = NULL;

    // TODO: confirm site query is formatted properly, currently getting no answer back (might be working as intended, since tested on domain without sites?)

    // Query site hosts
    if (site != NULL && strlen(site) > 0) {
        char dname[1000];
        snprintf(dname, sizeof(dname), "_ldap._tcp.%s._sites.%s", site, domain);

        int query_result = query_server_for_hosts(dname, &site_hosts);
        if (query_result != AD_SUCCESS) {
            result = AD_ERROR;

            goto end;
        }
    }

    const size_t site_hosts_size = ad_array_size(site_hosts);

    // Query default hosts
    char dname_default[1000];
    snprintf(dname_default, sizeof(dname_default), "_ldap._tcp.%s", domain);

    int query_result = query_server_for_hosts(dname_default, &default_hosts);
    if (query_result != AD_SUCCESS) {
        result = AD_ERROR;

        goto end;
    }

    const size_t default_hosts_size = ad_array_size(default_hosts);

    // Combine site and default hosts
    const int hosts_max_size = site_hosts_size + default_hosts_size + 1;
    hosts = calloc(hosts_max_size, sizeof(char *));
    size_t hosts_current_i = 0;
    
    // Load all site hosts first
    for (int i = 0; i < site_hosts_size; i++) {
        char *site_host = site_hosts[i];
        hosts[hosts_current_i] = strdup(site_host);
        hosts_current_i++;
    }

    // Add default hosts that aren't already in list
    for (int i = 0; i < default_hosts_size; i++) {
        char *default_host = default_hosts[i];

        bool already_in_list = false;
        for (int j = 0; j < hosts_current_i; j++) {
            char *other_host = hosts[j];

            if (strcmp(default_host, other_host) == 0) {
                already_in_list = true;
                break;
            }
        }

        if (!already_in_list) {
            hosts[hosts_current_i] = strdup(default_host);
            hosts_current_i++;
        }
    }

    end:
    {
        ad_array_free(site_hosts);
        ad_array_free(default_hosts);

        if (result == AD_SUCCESS) {
            *hosts_out = hosts;
        } else {
            *hosts_out = NULL;
            ad_array_free(hosts);
        }

        return result;
    }
}

int ad_login(const char* uri, LDAP **ld_out) {
    int result = AD_SUCCESS;

    LDAP *ld = NULL;

    const int result_init = ldap_initialize(&ld, uri);
    if (result_init != LDAP_SUCCESS) {
        result = AD_LDAP_ERROR;
        
        goto end;
    }

    // Set version
    const int version = LDAP_VERSION3;
    const int result_set_version = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &version);
    if (result_set_version != LDAP_OPT_SUCCESS) {
        result = AD_LDAP_ERROR;
        
        goto end;
    }

    // Disable referrals
    const int result_referral =ldap_set_option(ld, LDAP_OPT_REFERRALS, LDAP_OPT_OFF);
    if (result_referral != LDAP_OPT_SUCCESS) {
        result = AD_LDAP_ERROR;
        
        goto end;
    }

    // Set maxssf
    const char* sasl_secprops = "maxssf=56";
    const int result_secprops = ldap_set_option(ld, LDAP_OPT_X_SASL_SECPROPS, sasl_secprops);
    if (result_secprops != LDAP_SUCCESS) {
        result = AD_LDAP_ERROR;
        
        goto end;
    }

    // Setup sasl_defaults_gssapi 
    struct sasl_defaults_gssapi defaults;
    defaults.mech = "GSSAPI";
    ldap_get_option(ld, LDAP_OPT_X_SASL_REALM, &defaults.realm);
    ldap_get_option(ld, LDAP_OPT_X_SASL_AUTHCID, &defaults.authcid);
    ldap_get_option(ld, LDAP_OPT_X_SASL_AUTHZID, &defaults.authzid);
    defaults.passwd = NULL;

    // Perform bind operation
    unsigned sasl_flags = LDAP_SASL_QUIET;
    const int result_sasl = ldap_sasl_interactive_bind_s(ld, NULL,defaults.mech, NULL, NULL, sasl_flags, sasl_interact_gssapi, &defaults);

    ldap_memfree(defaults.realm);
    ldap_memfree(defaults.authcid);
    ldap_memfree(defaults.authzid);
    if (result_sasl != LDAP_SUCCESS) {
        result = AD_LDAP_ERROR;
        
        goto end;
    }

    // NOTE: not using this for now but might need later
    // The Man says: this function is used when an application needs to bind to another server in order to follow a referral or search continuation reference
    // ldap_set_rebind_proc(ld, sasl_rebind_gssapi, NULL);

    end:
    {
        if (result == AD_SUCCESS) {
            *ld_out = ld;
        } else {
            ldap_memfree(ld);
            *ld_out = NULL;
        }

        return result;
    }
}

size_t ad_array_size(char **array) {
    if (array == NULL) {
        return 0;
    } else {
        size_t count = 0;

        for (int i = 0; array[i] != NULL; i++) {
            count++;
        }

        return count;
    }
}

void ad_array_free(char **array) {
    if (array != NULL) {
        for (int i = 0; array[i] != NULL; i++) {
            free(array[i]);
        }

        free(array);
    }
}

void ad_2d_array_free(char ***array) {
    if (array != NULL) {
        for (int i = 0; array[i] != NULL; i++) {
            ad_array_free(array[i]);
        }

        free(array);
    }
}

int ad_search(LDAP *ld, const char *filter, const char* search_base, char ***list_out) {
    int result = AD_SUCCESS;

    char **list = NULL;
    LDAPMessage *res;

    char *attrs[] = {"1.1", NULL};
    const int result_search = ldap_search_ext_s(ld, search_base, LDAP_SCOPE_SUBTREE, filter, attrs, 1, NULL, NULL, NULL, LDAP_NO_LIMIT, &res);
    if (result_search != LDAP_SUCCESS) {
        result = AD_LDAP_ERROR;

        goto end;
    }

    const int entries_count = ldap_count_entries(ld, res);
    list = calloc(entries_count + 1, sizeof(char *));

    int i = 0;
    for (LDAPMessage *entry = ldap_first_entry(ld, res); entry != NULL; entry = ldap_next_entry(ld, entry), i++) {
        char *entry_dn = ldap_get_dn(ld, entry);
        list[i] = strdup(entry_dn);
        ldap_memfree(entry_dn);
    }

    end:
    {
        ldap_msgfree(res);

        if (result == AD_SUCCESS) {
            *list_out = list;
        } else {
            *list_out = NULL;
        }

        return result;
    }
}

int ad_list(LDAP *ld, const char *dn, char ***list_out) {
    int result = AD_SUCCESS;

    LDAPMessage *res;
    char **list = NULL;

    char *attrs[] = {"1.1", NULL};

    const int result_search = ldap_search_ext_s(ld, dn, LDAP_SCOPE_ONELEVEL, "(objectclass=*)", attrs, 0, NULL, NULL, NULL, LDAP_NO_LIMIT, &res);
    if (result_search != LDAP_SUCCESS) {
        result = AD_LDAP_ERROR;

        goto end;
    }

    const int entries_count = ldap_count_entries(ld, res);
    list = calloc(entries_count + 1, sizeof(char *));

    int i = 0;
    for (LDAPMessage *entry = ldap_first_entry(ld, res); entry != NULL; entry = ldap_next_entry(ld, entry), i++) {
        char *entry_dn = ldap_get_dn(ld, entry);
        list[i] = strdup(entry_dn);
        ldap_memfree(entry_dn);
    }

    end:
    {
        ldap_msgfree(res);

        if (result == AD_SUCCESS) {
            *list_out = list;
        } else {
            *list_out = NULL;
            ad_array_free(list);
        }

        return result;
    }
}

int ad_get_attribute(LDAP *ld, const char *dn, const char *attribute, char ***values_out) {
    int result = AD_SUCCESS;

    LDAPMessage *res;
    struct berval **values_ldap = NULL;
    char **values = NULL;

    const int result_get = ad_get_all_attributes_internal(ld, dn, attribute, &res);
    if (result_get != AD_SUCCESS) {
        result = result_get;

        goto end;
    }

    LDAPMessage *entry = ldap_first_entry(ld, res);

    values_ldap = ldap_get_values_len(ld, entry, attribute);
    if (values_ldap == NULL) {
        result = AD_LDAP_ERROR;

        goto end;
    }

    const int values_count = ldap_count_values_len(values_ldap);

    values = calloc(values_count + 1, sizeof(char *));
    for (int i = 0; i < values_count; i++) {
        struct berval value_berval = *values_ldap[i];
        values[i] = strdup(value_berval.bv_val);
    }

    end:
    {
        ldap_msgfree(res);
        ldap_value_free_len(values_ldap);

        if (result == AD_SUCCESS) {
            *values_out = values;
        } else {
            *values_out = NULL;
            ad_array_free(values);
        }

        return result;
    }
}

int ad_get_all_attributes(LDAP *ld, const char *dn, char ****attributes_out) {
    int result = AD_SUCCESS;

    char ***attributes = NULL;
    LDAPMessage *res;

    const int result_get = ad_get_all_attributes_internal(ld, dn, "*", &res);
    if (result_get != AD_SUCCESS) {
        result = result_get;

        goto end;
    }

    LDAPMessage *entry = ldap_first_entry(ld, res);

    // Count attributes
    int attributes_count = 0;
    {
        BerElement *berptr;
        for (char *attr = ldap_first_attribute(ld, entry, &berptr); attr != NULL; attr = ldap_next_attribute(ld, entry, berptr)) {
            ldap_memfree(attr);
            attributes_count++;
        }
        ber_free(berptr, 0);
    }

    const int attributes_size = attributes_count + 1;
    attributes = calloc(attributes_size, sizeof(char **));

    // Copy attribute values
    BerElement *berptr;
    int attributes_i = 0;
    for (char *attr = ldap_first_attribute(ld, entry, &berptr); attr != NULL; attr = ldap_next_attribute(ld, entry, berptr)) {
        struct berval **values_ldap = ldap_get_values_len(ld, entry, attr);
        if (values_ldap == NULL) {
            result = AD_LDAP_ERROR;

            ldap_value_free_len(values_ldap);

            goto end;
        }

        const int values_count = ldap_count_values_len(values_ldap);

        const int values_size = values_count + 2;
        char **values = calloc(values_size, sizeof(char *));
        values[0] = strdup(attr);
        for (int i = 0; i < values_count; i++) {
            struct berval value_berval = *values_ldap[i];
            values[i + 1] = strdup(value_berval.bv_val);
        }

        attributes[attributes_i] = values;
        attributes_i++;

        ldap_value_free_len(values_ldap);
        ldap_memfree(attr);
    }
    ber_free(berptr, 0);

    end:
    {
        ldap_msgfree(res);

        if (result == AD_SUCCESS) {
            *attributes_out = attributes;
        } else {
            ad_2d_array_free(attributes);
            *attributes_out = NULL;
        }

        return result;
    }
}

int ad_create_user(LDAP *ld, const char *username, const char *dn) {
    int result = AD_SUCCESS;

    char *upn = NULL;
    char *domain = NULL;

    LDAPMod attr1;
    const char *class_values[] = {"user", NULL};
    attr1.mod_op = LDAP_MOD_ADD;
    attr1.mod_type = "objectClass";
    attr1.mod_values = (char **)class_values;

    LDAPMod attr2;
    const char *name_values[] = {username, NULL};
    attr2.mod_op = LDAP_MOD_ADD;
    attr2.mod_type = "sAMAccountName";
    attr2.mod_values = (char **)name_values;
    
    LDAPMod attr3;
    const char *control_values[] = {"66050", NULL};
    attr3.mod_op = LDAP_MOD_ADD;
    attr3.mod_type = "userAccountControl";
    attr3.mod_values = (char **)control_values;

    LDAPMod attr4;
    // Construct userPrincipalName
    const int result_dn2domain = dn2domain(dn, &domain);
    if (result_dn2domain != AD_SUCCESS) {
        result = result_dn2domain;

        goto end;
    }
    upn = malloc(strlen(username) + strlen(domain) + 2);
    sprintf(upn, "%s@%s", username, domain);
    const char *upn_values[] = {upn, NULL};
    attr4.mod_op = LDAP_MOD_ADD;
    attr4.mod_type = "userPrincipalName";
    attr4.mod_values = (char **)upn_values;

    LDAPMod *attrs[] = {&attr1, &attr2, &attr3, &attr4, NULL};

    const int result_add_attributes = ldap_add_ext_s(ld, dn, attrs, NULL, NULL);
    if (result_add_attributes != LDAP_SUCCESS) {
        result = AD_LDAP_ERROR;
    }

    end:
    {
        free(domain);
        free(upn);

        return result;
    }
}

int ad_create_computer(LDAP *ld, const char *name, const char *dn) {
    int result = AD_SUCCESS;

    char *account_name = NULL;

    LDAPMod attr1;
    const char *class_values[] = {"top", "person", "organizationalPerson", "user", "computer", NULL};
    attr1.mod_op = LDAP_MOD_ADD;
    attr1.mod_type = "objectClass";
    attr1.mod_values = (char **)class_values;

    LDAPMod attr2;
    // Construct sAMAccountName
    account_name = malloc(strlen(name) + 2);
    sprintf(account_name, "%s$", name);
    for (int i = 0; i < strlen(account_name); i++) {
        account_name[i] = toupper(account_name[i]);
    } 
    const char *name_values[] = {account_name, NULL};
    attr2.mod_op = LDAP_MOD_ADD;
    attr2.mod_type = "sAMAccountName";
    attr2.mod_values = (char **)name_values;

    LDAPMod attr3;
    const char *control_values[] = {"4128", NULL};
    attr3.mod_op = LDAP_MOD_ADD;
    attr3.mod_type = "userAccountControl";
    attr3.mod_values = (char **)control_values;

    LDAPMod *attrs[] = {&attr1, &attr2, &attr3, NULL};

    const int result_add = ldap_add_ext_s(ld, dn, attrs, NULL, NULL);
    if (result_add != LDAP_SUCCESS) {
        result = AD_LDAP_ERROR;
    }

    free(account_name);

    return result;
}

int ad_create_ou(LDAP *ld, const char *ou_name, const char *dn) {
    int result = AD_SUCCESS;

    LDAPMod attr1;
    char *class_values[] = {"organizationalUnit", NULL};
    attr1.mod_op = LDAP_MOD_ADD;
    attr1.mod_type = "objectClass";
    attr1.mod_values = class_values;

    LDAPMod attr2;
    const char *name_values[] = {ou_name, NULL};
    attr2.mod_op = LDAP_MOD_ADD;
    attr2.mod_type = "name";
    attr2.mod_values = (char **)name_values;
    
    LDAPMod *attrs[] = {&attr1, &attr2, NULL};

    const int result_add = ldap_add_ext_s(ld, dn, attrs, NULL, NULL);
    if (result_add != LDAP_SUCCESS) {
        result = AD_LDAP_ERROR;
    }

    return result;
}

int ad_create_group(LDAP *ld, const char *group_name, const char *dn) {
    int result = AD_SUCCESS;

    LDAPMod attr1;
    const char *class_values[] = {"group", NULL};
    attr1.mod_op = LDAP_MOD_ADD;
    attr1.mod_type = "objectClass";
    attr1.mod_values = (char **)class_values;

    LDAPMod attr2;
    const char *name_values[] = {group_name, NULL};
    attr2.mod_op = LDAP_MOD_ADD;
    attr2.mod_type = "name";
    attr2.mod_values = (char **)name_values;
    
    LDAPMod attr3;
    const char *account_name_values[] = {group_name, NULL};
    attr3.mod_op = LDAP_MOD_ADD;
    attr3.mod_type = "sAMAccountName";
    attr3.mod_values = (char **)account_name_values;

    LDAPMod *attrs[] = {&attr1, &attr2, &attr3, NULL};

    const int result_add = ldap_add_ext_s(ld, dn, attrs, NULL, NULL);
    if (result_add != LDAP_SUCCESS) {
        result = AD_LDAP_ERROR;
    }

    return result;
}

int ad_delete(LDAP *ld, const char *dn) {
    int result = AD_SUCCESS;

    const int result_delete = ldap_delete_ext_s(ld, dn, NULL, NULL);
    if (result_delete != LDAP_SUCCESS) {
        result = AD_LDAP_ERROR;
    }

    return result;
}

int ad_user_lock(LDAP *ld, const char *dn) {
    int result = AD_SUCCESS;

    char **flags = NULL;
    
    const int result_get_flags = ad_get_attribute(ld, dn, "userAccountControl", &flags);
    if (result_get_flags != AD_SUCCESS || flags[0] == NULL) {
        result = result_get_flags;

        goto end;
    }

    char newflags[255];
    int iflags = atoi(flags[0]);
    iflags |= 2;
    snprintf(newflags, sizeof(newflags), "%d", iflags);

    const int result_replace = ad_attribute_replace(ld, dn, "userAccountControl", newflags);
    if (result_replace != AD_SUCCESS) {
        result = result_replace;

        goto end;
    }

    end:
    {
        ad_array_free(flags);
         
        return result;
    }
}

int ad_user_unlock(LDAP *ld, const char *dn) {
    int result = AD_SUCCESS;

    char **flags = NULL;

    const int result_get_flags = ad_get_attribute(ld, dn, "userAccountControl", &flags);
    if (result_get_flags != AD_SUCCESS || flags[0] == NULL) {
        result = result_get_flags;
        
        goto end;
    }

    int iflags = atoi(flags[0]);
    if (iflags & 2) {
        iflags ^= 2;

        char newflags[255];
        snprintf(newflags, sizeof(newflags), "%d", iflags);
        const int result_replace = ad_attribute_replace(ld, dn, "userAccountControl", newflags);
        if (result_replace != AD_SUCCESS) {
            // Failed to replace userAccountControl
            result = result_replace;

            goto end;
        }
    }

    end:
    {
        ad_array_free(flags);

        return result;
    }
}

int ad_user_set_pass(LDAP *ld, const char *dn, const char *password) {
    int result = AD_SUCCESS;
    
    char quoted_password[MAX_PASSWORD_LENGTH + 2];
    char unicode_password[(MAX_PASSWORD_LENGTH + 2) * 2];

    // Put quotes around the password
    snprintf(quoted_password, sizeof(quoted_password), "\"%s\"", password);
    // Unicode the password
    memset(unicode_password, 0, sizeof(unicode_password));
    for (int i = 0; i < strlen(quoted_password); i++) {
        unicode_password[i * 2] = quoted_password[i];
    }

    struct berval pw;
    pw.bv_val = unicode_password;
    pw.bv_len = strlen(quoted_password) * 2;

    struct berval *bervalues[] = {&pw, NULL};

    LDAPMod attr1;
    attr1.mod_type = "unicodePwd";
    attr1.mod_op = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    attr1.mod_bvalues = bervalues;

    LDAPMod *attrs[] = {&attr1, NULL};

    const int result_modify = ldap_modify_ext_s(ld, dn, attrs, NULL, NULL);
    if (result_modify != LDAP_SUCCESS) {
        result = AD_LDAP_ERROR;
    }

    return result;
}

int ad_attribute_add(LDAP *ld, const char *dn, const char *attribute, const char *value) {
    int result = AD_SUCCESS;

    LDAPMod attr;
    const char *values[] = {value, NULL};
    attr.mod_op = LDAP_MOD_ADD;
    attr.mod_type = (char *)attribute;
    attr.mod_values = (char **)values;
    
    LDAPMod *attrs[] = {&attr, NULL};

    const int result_modify = ldap_modify_ext_s(ld, dn, attrs, NULL, NULL);
    if (result_modify != LDAP_SUCCESS) {
        result = AD_LDAP_ERROR;
    }

    return result;
}

int ad_attribute_add_binary(LDAP *ld, const char *dn, const char *attribute, const char *data, int data_length) {
    int result = AD_SUCCESS;

    char *data_copy = strdup(data);

    struct berval ber_data;
    ber_data.bv_val = data_copy;
    ber_data.bv_len = data_length;

    struct berval *values[] = {&ber_data, NULL};

    LDAPMod attr;
    attr.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    attr.mod_type = (char *)attribute;
    attr.mod_bvalues = values;
    
    LDAPMod *attrs[] = {&attr, NULL};

    const int result_modify = ldap_modify_ext_s(ld, dn, attrs, NULL, NULL);

    if (result_modify != LDAP_SUCCESS) {
        result = AD_LDAP_ERROR;

        goto end;
    }

    end:
    {
        free(data_copy);

        return result;
    }
}

int ad_attribute_replace(LDAP *ld, const char *dn, const char *attribute, const char *value) {
    int result = AD_SUCCESS;
    
    LDAPMod attr;
    const char *values[] = {value, NULL};
    attr.mod_op = LDAP_MOD_REPLACE;
    attr.mod_type = (char *)attribute;
    attr.mod_values = (char **)values;
    
    LDAPMod *attrs[] = {&attr, NULL};

    const int result_modify = ldap_modify_ext_s(ld, dn, attrs, NULL, NULL);
    if (result_modify != LDAP_SUCCESS) {
        result = AD_LDAP_ERROR;
    }

    return result;
}

int ad_attribute_replace_binary(LDAP *ld, const char *dn, const char *attribute, const char *data, int data_length) {
    int result = AD_SUCCESS;

    char *data_copy = strdup(data);

    struct berval ber_data;
    ber_data.bv_val = data_copy;
    ber_data.bv_len = data_length;

    LDAPMod attr;
    struct berval *values[] = {&ber_data, NULL};
    attr.mod_op = LDAP_MOD_REPLACE|LDAP_MOD_BVALUES;
    attr.mod_type = (char *)attribute;
    attr.mod_bvalues = values;
    
    LDAPMod *attrs[] = {&attr, NULL};

    const int result_modify = ldap_modify_ext_s(ld, dn, attrs, NULL, NULL);
    if (result_modify != LDAP_SUCCESS) {
        result = AD_LDAP_ERROR;
    }

    free(data_copy);

    return result;
}

int ad_attribute_delete(LDAP *ld, const char *dn, const char *attribute, const char *value) {
    int result = AD_SUCCESS;
    
    LDAPMod attr;
    const char *values[] = {value, NULL};
    attr.mod_op = LDAP_MOD_DELETE;
    attr.mod_type = (char *)attribute;
    attr.mod_values = (char **)values;
    
    LDAPMod *attrs[] = {&attr, NULL};

    const int result_modify = ldap_modify_ext_s(ld, dn, attrs, NULL, NULL);
    if (result_modify != LDAP_SUCCESS) {
        result = AD_LDAP_ERROR;
    }

    return result;
}

int ad_rename(LDAP *ld, const char *dn, const char *new_rdn) {
    int result = AD_SUCCESS;

    const int result_rename = ldap_rename_s(ld, dn, new_rdn, NULL, 1, NULL, NULL);
    if (result_rename != LDAP_SUCCESS) {
        result = AD_LDAP_ERROR;
    }

    return result;
}

int ad_rename_user(LDAP *ld, const char *dn, const char *new_name) {
    int result = AD_SUCCESS;

    char* new_rdn = NULL;
    char* domain = NULL;
    char* upn = NULL;

    const int result_replace_name = ad_attribute_replace(ld, dn, "sAMAccountName", new_name);
    if (result_replace_name != AD_SUCCESS) {
        // Failed to change sAMAccountName
        result = result_replace_name;

        goto end;
    }

    // Construct userPrincipalName
    const int result_dn2domain = dn2domain(dn, &domain);
    if (result_dn2domain != AD_SUCCESS) {
        result = result_dn2domain;

        goto end;
    }
    upn = malloc(strlen(new_name) + strlen(domain) + 2);
    sprintf(upn, "%s@%s", new_name, domain);

    const int result_replace_upn = ad_attribute_replace(ld, dn, "userPrincipalName", upn);
    if (result_replace_upn != AD_SUCCESS) {
        // Failed to change userPrincipalName
        result = result_replace_upn;

        goto end;
    }

    new_rdn = malloc(strlen(new_name) + 4);
    sprintf(new_rdn, "cn=%s", new_name);

    const int result_rename = ad_rename(ld, dn, new_rdn);
    if (result_rename != AD_SUCCESS) {
        result = result_rename;

        goto end;
    }

    end:
    {
        free(domain);
        free(upn);
        free(new_rdn);

        return result;
    }
}

int ad_rename_group(LDAP *ld, const char *dn, const char *new_name) {
    int result = AD_SUCCESS;

    char *new_rdn = NULL;

    const int result_replace = ad_attribute_replace(ld, dn, "sAMAccountName", new_name);
    if (result_replace != AD_SUCCESS) {
        // Failed to change sAMAccountName
        result = result_replace;

        goto end;
    }

    new_rdn = malloc(strlen(new_name) + 4);
    sprintf(new_rdn, "cn=%s", new_name);

    const int result_rename = ldap_rename_s(ld, dn, new_rdn, NULL, 1, NULL, NULL);
    if (result_rename != LDAP_SUCCESS) {
        result = AD_LDAP_ERROR;

        goto end;
    }

    end:
    {
        free(new_rdn);

        return result;
    }
}

int ad_move_user(LDAP *ld, const char *current_dn, const char *new_container) {
    int result = AD_SUCCESS;

    char **username = NULL;
    char *domain = NULL;
    char *upn = NULL;

    const int result_get_username = ad_get_attribute(ld, current_dn, "sAMAccountName", &username);
    if (result_get_username != AD_SUCCESS || username[0] == NULL) {
        result = result_get_username;

        goto end;
    }

    // Construct userPrincipalName
    const int result_dn2domain = dn2domain(new_container, &domain);
    if (AD_SUCCESS != result_dn2domain) {
        result = result_dn2domain;

        goto end;
    }
    upn = malloc(strlen(username[0]) + strlen(domain) + 2);
    sprintf(upn, "%s@%s", username[0], domain);

    // Modify userPrincipalName in case of domain change
    const int result_replace = ad_attribute_replace(ld, current_dn, "userPrincipalName", upn);
    if (result_replace != AD_SUCCESS) {
        // Failed to change userPrincipalName
        result = result_replace;

        goto end;
    }

    const int result_move = ad_move(ld, current_dn, new_container);
    if (result_move != AD_SUCCESS) {
        result = result_move;

        goto end;
    }
    
    end:
    {
        ad_array_free(username);
        free(domain);
        free(upn);

        return result;
    }
}

int ad_move(LDAP *ld, const char *current_dn, const char *new_container) {
    int result = AD_SUCCESS;

    char *rdn = strdup(current_dn);

    char *comma_ptr = strchr(rdn, ',');
    if (comma_ptr == NULL) {
        // Failed to extract RDN from DN
        result = AD_INVALID_DN;

        goto end;
    }
    *comma_ptr = '\0';

    const int result_rename = ldap_rename_s(ld, current_dn, rdn, new_container, 1, NULL, NULL);
    if (result_rename != LDAP_SUCCESS) {
        result = AD_LDAP_ERROR;

        goto end;
    }

    end:
    {
        free(rdn);

        return result;
    }
}

int ad_group_add_user(LDAP *ld, const char *group_dn, const char *user_dn) {
    return ad_attribute_add(ld, group_dn, "member", user_dn);
}

int ad_group_remove_user(LDAP *ld, const char *group_dn, const char *user_dn) {
    return ad_attribute_delete(ld, group_dn, "member", user_dn);
}

/**
 * Convert a distinguished name into the domain controller
 * dns domain, eg: "ou=users,dc=example,dc=com" returns
 * "example.com".
 * domain should be freed using free()
 * Returns AD_SUCCESS, AD_INVALID_DN
 */
int dn2domain(const char *dn, char **domain_out) {
    int result = AD_SUCCESS;
    
    LDAPDN exp_dn = NULL;
    char *domain = NULL;

    // Explode dn
    const int result_str2dn = ldap_str2dn(dn, &exp_dn, LDAP_DN_FORMAT_LDAPV3);
    if (result_str2dn != LDAP_SUCCESS) {
        result = AD_INVALID_DN;

        goto end;
    }

    // Iterate over RDNs, extracting the domain part
    domain = calloc(MAX_DN_LENGTH, sizeof(char));
    for (int i = 0; exp_dn[i] != NULL; i++) {
        // NOTE: rdn can be multi-valued but we only process the first value
        LDAPRDN rdns = exp_dn[i];
        LDAPAVA *rdn = rdns[0];

        if (rdn == NULL) {
            result = AD_INVALID_DN;

            goto end;
        }

        // NOTE: BER/DER encoded attributes are unsupported
        const bool string_encoding = (rdn->la_flags & LDAP_AVA_STRING);
        if (!string_encoding) {
            result = AD_INVALID_DN;

            goto end;
        }

        // Save rdn if it's part of domain string
        const bool rdn_is_part_of_domain = (strncasecmp("DC", rdn[0].la_attr.bv_val, 2) == 0);
        if (rdn_is_part_of_domain) {
            strncat(domain, rdn[0].la_value.bv_val, MAX_DN_LENGTH - strlen(domain) - 1);
            strncat(domain, ".", MAX_DN_LENGTH - strlen(domain) - 1);
        }
    }

    // Convert domain to lower case
    for (int i = 0; i < strlen(domain); i++) {
        domain[i] = tolower(domain[i]);
    }

    // Remove last '.'
    if (strlen(domain) > 0) {
        domain[strlen(domain) - 1] = '\0';
    }

    end:
    {
        ldap_dnfree(exp_dn);

        if (result == AD_SUCCESS) {
        *domain_out = domain;
        } else {
            *domain_out = NULL;
            free(domain);
        }

        return result;
    }
}

/**
 * Callback for ldap_sasl_interactive_bind_s
 */
int sasl_interact_gssapi(LDAP *ld, unsigned flags, void *indefaults, void *in) {
    sasl_defaults_gssapi *defaults = indefaults;
    sasl_interact_t *interact = (sasl_interact_t*)in;

    if (ld == NULL) {
        return LDAP_PARAM_ERROR;
    }

    while (interact->id != SASL_CB_LIST_END) {
        const char *dflt = interact->defresult;

        switch (interact->id) {
            case SASL_CB_GETREALM:
            if (defaults)
                dflt = defaults->realm;
            break;
            case SASL_CB_AUTHNAME:
            if (defaults)
                dflt = defaults->authcid;
            break;
            case SASL_CB_PASS:
            if (defaults)
                dflt = defaults->passwd;
            break;
            case SASL_CB_USER:
            if (defaults)
                dflt = defaults->authzid;
            break;
            case SASL_CB_NOECHOPROMPT:
            break;
            case SASL_CB_ECHOPROMPT:
            break;
        }

        if (dflt && !*dflt) {
            dflt = NULL;
        }

        /* input must be empty */
        interact->result = (dflt && *dflt) ? dflt : "";
        interact->len = strlen(interact->result);
        interact++;
    }

    return LDAP_SUCCESS;
}

/**
 * Perform a query for dname and output hosts
 * dname is a combination of protocols (ldap, tcp), domain and site
 * NOTE: this is rewritten from
 * https://github.com/paleg/libadclient/blob/master/adclient.cpp
 * which itself is copied from
 * https://www.ccnx.org/releases/latest/doc/ccode/html/ccndc-srv_8c_source.html
 * Another example of similar procedure:
 * https://www.gnu.org/software/shishi/coverage/shishi/lib/resolv.c.gcov.html
 */
int query_server_for_hosts(const char *dname, char ***hosts_out) {
    union dns_msg {
        HEADER header;
        unsigned char buf[NS_MAXMSG];
    } msg;

    int result = AD_SUCCESS;

    char **hosts = NULL;

    const int msg_len = res_search(dname, ns_c_in, ns_t_srv, msg.buf, sizeof(msg.buf));

    if (msg_len < 0 || msg_len < sizeof(HEADER)) {
        result = AD_ERROR;

        goto end;
    }

    const int packet_count = ntohs(msg.header.qdcount);
    const int answer_count = ntohs(msg.header.ancount);

    unsigned char *curr = msg.buf + sizeof(msg.header);
    const unsigned char *eom = msg.buf + msg_len;

    // Skip over packet records
    for (int i = packet_count; i > 0 && curr < eom; i--) {
        const int packet_len = dn_skipname(curr, eom);

        if (packet_len < 0) {
            result = AD_ERROR;

            goto end;
        }

        curr = curr + packet_len + QFIXEDSZ;
    }

    // Init hosts list
    const size_t hosts_size = answer_count + 1;
    hosts = calloc(hosts_size, sizeof(char *));

    // Process answers by collecting hosts into list
    size_t hosts_current_i = 0;
    for (int i = 0; i < answer_count; i++) {
        // Get server
        char server[NS_MAXDNAME];
        const int server_len = dn_expand(msg.buf, eom, curr, server, sizeof(server));
        if (server_len < 0) {
            result = AD_ERROR;

            goto end;
        }
        curr = curr + server_len;

        int record_type;
        int UNUSED(record_class);
        int UNUSED(ttl);
        int record_len;
        GETSHORT(record_type, curr);
        GETSHORT(record_class, curr);
        GETLONG(ttl, curr);
        GETSHORT(record_len, curr);
        
        unsigned char *record_end = curr + record_len;
        if (record_end > eom) {
            result = AD_ERROR;

            goto end;
        }

        // Skip non-server records
        if (record_type != ns_t_srv) {
            curr = record_end;

            continue;
        }

        int UNUSED(priority);
        int UNUSED(weight);
        int UNUSED(port);
        GETSHORT(priority, curr);
        GETSHORT(weight, curr);
        GETSHORT(port, curr);
        // TODO: need to save port field? maybe to incorporate into uri

        // Get host
        char host[NS_MAXDNAME];
        const int host_len = dn_expand(msg.buf, eom, curr, host, sizeof(host));
        if (host_len < 0) {
            result = AD_ERROR;

            goto end;
        }

        hosts[hosts_current_i] = strdup(host);
        hosts_current_i++;

        curr = record_end;
    }

    end:
    {
        if (result == AD_SUCCESS) {
            *hosts_out = hosts;
        } else {
            *hosts_out = NULL;
            ad_array_free(hosts);
        }

        return result;
    }
}

/**
 * Perform an attribute search on object
 * Outputs LDAPMessage res which contains results of the search
 * res should bbe freed by the caller using ldap_msgfree()
 */
int ad_get_all_attributes_internal(LDAP *ld, const char *dn, const char *attribute, LDAPMessage **res_out) {
    int result = AD_SUCCESS;

    LDAPMessage *res;

    // TODO: use paged search
    char *attrs[] = {(char *)attribute, NULL};
    const int result_search = ldap_search_ext_s(ld, dn, LDAP_SCOPE_BASE, "(objectclass=*)", attrs, 0, NULL, NULL, NULL, LDAP_NO_LIMIT, &res);
    if (result_search != LDAP_SUCCESS) {
        result = AD_LDAP_ERROR;
        
        goto end;
    }

    const int entries_count = ldap_count_entries(ld, res);
    if (entries_count == 0) {
        // No attribute entries found
        result = AD_ERROR;
        
        goto end;
    } else if (entries_count > 1) {
        // Multiple attribute entries found
        result = AD_ERROR;
        
        goto end;
    }

    end:
    {
        if (result == AD_SUCCESS) {
            *res_out = res;
        } else {
            *res_out = NULL;
            ldap_msgfree(res);
        }

        return result;
    }
}
