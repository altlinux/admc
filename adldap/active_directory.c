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

/* active_directory.c
 * generic directory management functions */

// TODO: ldap_add_s is deprecated but works, not a big deal

#if HAVE_CONFIG_H
//#   include <config.h>
#endif

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

#include <errno.h>

#ifdef __GNUC__
#  define UNUSED(x) x __attribute__((unused))
#else
#  define UNUSED(x) x
#endif

#define MAX_ERR_LENGTH 1024
char ad_error_msg[MAX_ERR_LENGTH];
int ad_error_code;

#define MAX_PASSWORD_LENGTH 255

// char *uri=NULL;
char *binddn=NULL;
char *bindpw=NULL;
// char *search_base=NULL;
// TODO: unhardcode this
//char* uri = "ldap://dc0.domain.alt";
//char* search_base = "DC=domain,DC=alt";

// TODO: use this in other appropriate places, like AdInterface
size_t ad_array_size(const char **array) {
    size_t count = 0;

    for (int i = 0; array[i] != NULL; i++) {
        count++;
    }

    return count;
}

void ad_array_free(char **array) {
    if (array == NULL) {
        return;
    }

    for (int i = 0; array[i] != NULL; i++) {
        free(array[i]);
    }

    free(array);
}

typedef struct sasl_defaults_gssapi {
    char *mech;
    char *realm;
    char *authcid;
    char *passwd;
    char *authzid;
} sasl_defaults_gssapi;

int sasl_interact_gssapi(LDAP *ds, unsigned flags, void *indefaults, void *in) {
    sasl_defaults_gssapi *defaults = indefaults;
    sasl_interact_t *interact = (sasl_interact_t*)in;

    if (ds == NULL) {
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

/*
AD can set following limit (http://support.microsoft.com/kb/315071/en-us):
 MaxValRange - This value controls the number of values that are returned
   for an attribute of an object, independent of how many attributes that
   object has, or of how many objects were in the search result. If an
   attribute has more than the number of values that are specified by the
   MaxValRange value, you must use value range controls in LDAP to retrieve
   values that exceed the MaxValRange value. MaxValueRange controls the
   number of values that are returned on a single attribute on a single object.

OpenLDAP does not support ranged controls for values:
  https://www.mail-archive.com/openldap-its@openldap.org/msg00962.html

So the only way is it increase MaxValRange in DC:
 Ntdsutil.exe
   LDAP policies
     connections
       connect to server "DNS name of server"
       q
     Show Values
     Set MaxValRange to 10000
     Show Values
     Commit Changes
     Show Values
     q
   q
*/
char **get_values(LDAP *ds, LDAPMessage *entry) {
    if (ds == NULL) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in get_values: ds is NULL");
        ad_error_code=AD_SERVER_CONNECT_FAILURE;
        return NULL;
    }

    
}

// hosts must be NULL
// NOTE: this is rewritten from
// https://github.com/paleg/libadclient/blob/master/adclient.cpp
// which itself is copied from
// https://www.ccnx.org/releases/latest/doc/ccode/html/ccndc-srv_8c_source.html
// Another example of similar procedure:
// https://www.gnu.org/software/shishi/coverage/shishi/lib/resolv.c.gcov.html
int query_server_for_hosts(const char *dname, char ***hosts) {
    if (*hosts != NULL) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in query_server_for_hosts(%s): hosts pointer is not NULL\n", dname);
        goto error;
    }

    union dns_msg {
        HEADER header;
        unsigned char buf[NS_MAXMSG];
    } msg;

    const size_t msg_len = res_search(dname, ns_c_in, ns_t_srv, msg.buf, sizeof(msg.buf));

    if (msg_len < 0 || msg_len < sizeof(HEADER)) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in query_server_for_hosts(%s): bad msg_len\n", dname);
        goto error;
    }

    const int packet_count = ntohs(msg.header.qdcount);
    const int answer_count = ntohs(msg.header.ancount);

    unsigned char *curr = msg.buf + sizeof(msg.header);
    const unsigned char *eom = msg.buf + msg_len;

    // Skip over packet records
    for (int i = packet_count; i > 0 && curr < eom; i--) {
        const int packet_len = dn_skipname(curr, eom);

        if (packet_len < 0) {
            snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in query_server_for_hosts(%s): dn_skipname < 0\n", dname);
            goto error;
        }

        curr = curr + packet_len + QFIXEDSZ;
    }

    // Init hosts list
    const size_t hosts_size = answer_count + 1;
    *hosts = malloc(sizeof(char *) * hosts_size);

    // Process answers by collecting hosts into list
    size_t hosts_current_i = 0;
    for (int i = 0; i < answer_count; i++) {
        // Get server
        char server[NS_MAXDNAME];
        const int server_len = dn_expand(msg.buf, eom, curr, server, sizeof(server));
        if (server_len < 0) {
            snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in query_server_for_hosts(%s): dn_expand(server) < 0\n", dname);
            goto error;
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
            snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in query_server_for_hosts(%s): record_end > eom\n", dname);
            goto error;
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
            snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in query_server_for_hosts(%s): dn_expand(host) < 0\n", dname);
            goto error;
        }

        (*hosts)[hosts_current_i] = strdup(host);
        hosts_current_i++;

        curr = record_end;
    }

    (*hosts)[hosts_current_i] = NULL;

    return AD_SUCCESS;

    error:
    {
        ad_array_free(*hosts);
        *hosts = NULL;

        return AD_RESOLV_ERROR;
    }
}

int ad_get_domain_hosts(char *domain, char *site, char ***hosts) {
    int result = AD_SUCCESS; 
    
    if (*hosts != NULL) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in ad_get_domain_hosts(%s, %s): hosts pointer is not NULL\n", domain, site);
        result = AD_RESOLV_ERROR;
        goto end;
    }

    char **site_hosts = NULL;
    char **default_hosts = NULL;

    // TODO: confirm site query is formatted properly, currently getting no answer back (might be working as intended, since tested on domain without sites?)

    // Query site hosts
    if (site != NULL) {
        char dname[1000];
        snprintf(dname, sizeof(dname), "_ldap._tcp.%s._sites.%s", site, domain);

        int query_result = query_server_for_hosts(dname, &site_hosts);
        if (query_result != AD_SUCCESS) {
            result = query_result;
            goto end;
        }
    }

    const size_t site_hosts_size = ad_array_size(site_hosts);

    // Query default hosts
    char dname_default[1000];
    snprintf(dname_default, sizeof(dname_default), "_ldap._tcp.%s", domain);

    int query_result = query_server_for_hosts(dname_default, &default_hosts);
    if (query_result != AD_SUCCESS) {
        result = query_result;
        goto end;
    }

    const size_t default_hosts_size = ad_array_size(default_hosts);

    // Combine site and default hosts
    const int hosts_max_size = site_hosts_size + default_hosts_size + 1;
    *hosts = malloc(sizeof(char *) * hosts_max_size);
    size_t hosts_current_i = 0;
    
    // Load all site hosts first
    for (int i = 0; i < site_hosts_size; i++) {
        char *site_host = site_hosts[i];
        (*hosts)[hosts_current_i] = strdup(site_host);
        hosts_current_i++;
    }

    // Add default hosts that aren't already in list
    for (int i = 0; i < default_hosts_size; i++) {
        char *default_host = default_hosts[i];

        bool already_in_list = false;
        for (int j = 0; j < hosts_current_i; j++) {
            char *other_host = (*hosts)[j];

            if (strcmp(default_host, other_host) == 0) {
                already_in_list = true;
                break;
            }
        }

        if (!already_in_list) {
            (*hosts)[hosts_current_i] = strdup(default_host);
            hosts_current_i++;
        }
    }

    (*hosts)[hosts_current_i] = NULL;

    result = AD_SUCCESS;

    end:
    {
        ad_array_free(site_hosts);
        ad_array_free(default_hosts);

        return result;
    }
}

/* connect and authenticate to active directory server.
    returns an ldap connection identifier or 0 on error */
LDAP *ad_login(const char* uri) {
    int version, result, bindresult;

    char **hosts = NULL;
    int hosts_result = ad_get_domain_hosts("DOMAIN.ALT", "SITE", &hosts);
    if (hosts_result == AD_SUCCESS) {
        for (int i = 0; hosts[i] != NULL; i++) {
            printf("%s\n", hosts[i]);
        }

        ad_array_free(hosts);
    }

    /* open the connection to the ldap server */
    LDAP *ds = NULL;
    result=ldap_initialize(&ds, uri);
    if(result!=LDAP_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error doing ldap_initialize on uri %s: %s", uri, ldap_err2string(result));
        ad_error_code=AD_SERVER_CONNECT_FAILURE;
        return NULL;
    }

    // set version
    version=LDAP_VERSION3;
    result=ldap_set_option(ds, LDAP_OPT_PROTOCOL_VERSION, &version);
    if(result!=LDAP_OPT_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in ldap_set_option (protocol->v3): %s", ldap_err2string(result));
        ad_error_code=AD_SERVER_CONNECT_FAILURE;
        return NULL;
    }

    // disable referrals
    result=ldap_set_option(ds, LDAP_OPT_REFERRALS, LDAP_OPT_OFF);
    if(result!=LDAP_OPT_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in ldap_set_option (referrals=0): %s", ldap_err2string(result));
        ad_error_code=AD_SERVER_CONNECT_FAILURE;
        return NULL;
    }

    // NOTE: use gssapi instead of simple
    char* sasl_secprops = "maxssf=56";
    result = ldap_set_option(ds, LDAP_OPT_X_SASL_SECPROPS, (void *) sasl_secprops);
    if (result != LDAP_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in ldap_set_option (LDAP_OPT_X_SASL_SECPROPS): %s", ldap_err2string(result));
        ad_error_code=AD_SERVER_CONNECT_FAILURE;
        return NULL;
    }

    // Setup sasl_defaults_gssapi 
    struct sasl_defaults_gssapi defaults;
    defaults.mech = strdup("GSSAPI");
    ldap_get_option(ds, LDAP_OPT_X_SASL_REALM, &defaults.realm);
    ldap_get_option(ds, LDAP_OPT_X_SASL_AUTHCID, &defaults.authcid);
    ldap_get_option(ds, LDAP_OPT_X_SASL_AUTHZID, &defaults.authzid);
    defaults.passwd = NULL;

    unsigned sasl_flags = LDAP_SASL_QUIET;
    // TODO: why is mech passed in alone and as first member of defaults?
    result = ldap_sasl_interactive_bind_s(ds, NULL,
      defaults.mech, NULL, NULL,
      sasl_flags, sasl_interact_gssapi, &defaults);

    free(defaults.mech);
    ldap_memfree(defaults.realm);
    ldap_memfree(defaults.authcid);
    ldap_memfree(defaults.authzid);
    if (result != LDAP_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in ldap_sasl_interactive_bind_s: %s", ldap_err2string(result));
        ad_error_code=AD_SERVER_CONNECT_FAILURE;
        return NULL;
    }

    // NOTE: not using this for now but might need later
    // The Man says: this function is used when an application needs to bind to another server in order to follow a referral or search continuation reference
    // ldap_set_rebind_proc(*ds, sasl_rebind_gssapi, NULL);

    return ds;
}

/**
 * Convert a distinguished name into the domain controller
 * dns domain, eg: "ou=users,dc=example,dc=com" returns
 * "example.com".
 * memory allocated should be returned with free()
 */
int dn2domain(const char *dn, char** domain) {
    LDAPDN ldn = NULL;
    LDAPRDN lrdn = NULL;
    LDAPAVA* lattr = NULL;

    int result = AD_SUCCESS;
    int i;
    /* This way we'll always have null-terminated string without
     * workarounds. Cost of memory initialization is not comparable
     * with code readability */
    char *dc = calloc(1024, sizeof(char));
    /* Catch malloc error */
    if (NULL == dc) {
        result = ENOMEM;
        goto dn2domain_end;
    }

    /* Explode string into set of structures representing RDNs */
    if (ldap_str2dn(dn, &ldn, LDAP_DN_FORMAT_LDAPV3) != LDAP_SUCCESS) {
        result = AD_INVALID_DN;
        goto dn2domain_end;
    }

    /* Iterate over RDNs checking for Domain Components and extract
     * their values */
    for(i = 0; NULL != ldn[i]; i++) {
        lrdn = ldn[i];
        /* Multi-valued RDNs are not supported so no iteration over
         * lrdn[x] */
        /* Check that we have at least one RDN */
        if (NULL == lrdn[0]) {
            result = AD_ATTRIBUTE_ENTRY_NOT_FOUND;
            goto dn2domain_end;
        }
        lattr = lrdn[0];
        /* BER/DER encoded attributes are unsupported */
        if (0 == (lattr->la_flags & LDAP_AVA_STRING)) {
            result = AD_LDAP_OPERATION_FAILURE;
            goto dn2domain_end;
        }
        // FIXME: Check for buffer overflow
        if(!strncasecmp("DC", lattr[0].la_attr.bv_val, 2)) {
            strncat(dc, lattr[0].la_value.bv_val, lattr[0].la_value.bv_len);
            strncat(dc, ".", 1024);
        }
    }

    i = strlen(dc);
    for(i = 0; '\0' != dc[i]; i++) {
        dc[i] = tolower(dc[i]);
    }

dn2domain_end:
    /* Free the memory allocated by ldap_str2dn */
    if (NULL != ldn) {
        ldap_dnfree(ldn);
    }

    /* Free the memory allocated for resolved DNS domain name in case
     * of resolution errors */
    if (AD_SUCCESS != result) {
        if (NULL != dc) {
            free(dc);
            dc = NULL;
        }
    }

    (*domain) = dc;

    return result;
}

/* public functions */

/* get a pointer to the last error message */
char *ad_get_error() {
    return ad_error_msg;
}

/* return the last error code generated */
int ad_get_error_num() {
    return ad_error_code;
}

/* 
  creates an empty, locked user account with given username and dn
 and attributes:
    objectClass=user
    sAMAccountName=username
    userAccountControl=66050 
 (ACCOUNTDISABLE|NORMAL_ACCOUNT|DONT_EXPIRE_PASSWORD)
    userprincipalname
    returns AD_SUCCESS on success 
*/
int ad_create_user(LDAP *ds, const char *username, const char *dn) {
    LDAPMod *attrs[5];
    LDAPMod attr1, attr2, attr3, attr4;
    int result;
    int result_dn2domain;

    char *objectClass_values[]={"user", NULL};
    char *name_values[2];
    char *accountControl_values[]={"66050", NULL};
    char* upn = NULL;
    char* domain = NULL;
    char *upn_values[2];

    attr1.mod_op = LDAP_MOD_ADD;
    attr1.mod_type = "objectClass";
    attr1.mod_values = objectClass_values;

    name_values[0]=strdup(username);
    name_values[1]=NULL;
    attr2.mod_op = LDAP_MOD_ADD;
    attr2.mod_type = "sAMAccountName";
    attr2.mod_values = name_values;
    
    attr3.mod_op = LDAP_MOD_ADD;
    attr3.mod_type = "userAccountControl";
    attr3.mod_values = accountControl_values;

    result_dn2domain = dn2domain(dn, &domain);
    if (AD_SUCCESS != result_dn2domain) {
        ad_error_code = result_dn2domain;
        goto ad_create_user_end;
    }
    upn=malloc(strlen(username)+strlen(domain)+2);
    sprintf(upn, "%s@%s", username, domain);
    upn_values[0]=upn;
    upn_values[1]=NULL;
    attr4.mod_op = LDAP_MOD_ADD;
    attr4.mod_type = "userPrincipalName";
    attr4.mod_values = upn_values;

    attrs[0]=&attr1;
    attrs[1]=&attr2;
    attrs[2]=&attr3;
    attrs[3]=&attr4;
    attrs[4]=NULL;

    result=ldap_add_s(ds, dn, attrs);
    if(result!=LDAP_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in ldap_add %s", ldap_err2string(result));
        ad_error_code=AD_LDAP_OPERATION_FAILURE;
    } else {
        ad_error_code=AD_SUCCESS;
    }

ad_create_user_end:
    if (NULL != domain) {
        free(domain);
    }
    if (NULL != name_values[0]) {
        free(name_values[0]);
    }

    return ad_error_code;
}

/* 
  creates a computer account
 and attributes:
    objectClass=top,person,organizationalPerson,user,computer
    sAMAccountName=NAME$
    userAccountControl=4128
    returns AD_SUCCESS on success 
*/
int ad_create_computer(LDAP *ds, const char *name, const char *dn) {
    LDAPMod *attrs[4];
    LDAPMod attr1, attr2, attr3;
    int i, result;

    char *objectClass_values[]={"top", "person", "organizationalPerson",
    "user", "computer", NULL};
    char *name_values[2];
    char *accountControl_values[]={"4128", NULL};

    attr1.mod_op = LDAP_MOD_ADD;
    attr1.mod_type = "objectClass";
    attr1.mod_values = objectClass_values;

    name_values[0]=malloc(strlen(name)+2);
    sprintf(name_values[0], "%s$", name);
    for(i=0; name_values[0][i]; i++) 
        name_values[0][i]=toupper(name_values[0][i]);
    name_values[1]=NULL;
    attr2.mod_op = LDAP_MOD_ADD;
    attr2.mod_type = "sAMAccountName";
    attr2.mod_values = name_values;

    attr3.mod_op = LDAP_MOD_ADD;
    attr3.mod_type = "userAccountControl";
    attr3.mod_values = accountControl_values;

    attrs[0]=&attr1;
    attrs[1]=&attr2;
    attrs[2]=&attr3;
    attrs[3]=NULL;

    result=ldap_add_s(ds, dn, attrs);
    if(result!=LDAP_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in ldap_add %s", ldap_err2string(result));
        ad_error_code=AD_LDAP_OPERATION_FAILURE;
    } else {
        ad_error_code=AD_SUCCESS;
    }

    free(name_values[0]);

    return ad_error_code;
}

/* ad_object_delete deletes the given dn
    returns non-zero on success */
int ad_object_delete(LDAP *ds, const char *dn) {
    int result;

    result=ldap_delete_s(ds, dn);
    if(result!=LDAP_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in ldap_delete: %s", ldap_err2string(result));
        ad_error_code=AD_LDAP_OPERATION_FAILURE;
    } else {
        ad_error_code=AD_SUCCESS;
    }
    return ad_error_code;
}

/* ad_setpass sets the password for the given user
    returns AD_SUCCESS on success */
int ad_setpass(LDAP *ds, const char *dn, const char *password) {
    char quoted_password[MAX_PASSWORD_LENGTH+2];
    char unicode_password[(MAX_PASSWORD_LENGTH+2)*2];
    int i;
    LDAPMod *attrs[2];
    LDAPMod attr1;
    struct berval *bervalues[2];
    struct berval pw;
    int result;

    /* put quotes around the password */
    snprintf(quoted_password, sizeof(quoted_password), "\"%s\"", password);
    /* unicode the password string */
    memset(unicode_password, 0, sizeof(unicode_password));
    for(i=0; i<strlen(quoted_password); i++)
        unicode_password[i*2]=quoted_password[i];

    pw.bv_val = unicode_password;
    pw.bv_len = strlen(quoted_password)*2;

    bervalues[0]=&pw;
    bervalues[1]=NULL;

    attr1.mod_type="unicodePwd";
    attr1.mod_op = LDAP_MOD_REPLACE|LDAP_MOD_BVALUES;
    attr1.mod_bvalues = bervalues;

    attrs[0]=&attr1;
    attrs[1]=NULL;

    result = ldap_modify_s(ds, dn, attrs);
    if(result!=LDAP_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in ldap_modify for password: %s", ldap_err2string(result));
        ad_error_code=AD_LDAP_OPERATION_FAILURE;
    } else {
        ad_error_code=AD_SUCCESS;
    }
    return ad_error_code;
}

/* general search function */
char **ad_search(LDAP *ds, const char *attribute, const char *value, const char* search_base) {
    char *filter;
    int filter_length;
    char *attrs[]={"1.1", NULL};
    LDAPMessage *res;
    LDAPMessage *entry;
    int i, result, num_results;
    char **dnlist;
    char *dn;

    if(!search_base) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error: couldn't read active directory searchbase parameter");
        ad_error_code=AD_MISSING_CONFIG_PARAMETER;
        return (char **)-1;
    }

    filter_length=(strlen(attribute)+strlen(value)+4);
    filter=malloc(filter_length);
    snprintf(filter, filter_length, "(%s=%s)", attribute, value);

    result=ldap_search_s(ds, search_base, LDAP_SCOPE_SUBTREE, filter, attrs, 1, &res);
    if(result!=LDAP_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, 
            "Error in ldap_search_s for ad_search: %s", 
            ldap_err2string(result));
        ad_error_code=AD_LDAP_OPERATION_FAILURE;
        return (char **)-1;
    }
    free(filter);

    num_results=ldap_count_entries(ds, res);
    if(num_results==0) {
        ldap_msgfree(res);
        snprintf(ad_error_msg, MAX_ERR_LENGTH,
            "%s not found", value);
        ad_error_code=AD_OBJECT_NOT_FOUND;
        return NULL;
    }

    dnlist=malloc(sizeof(char *)*(num_results+1));

    entry=ldap_first_entry(ds, res);
    dn=ldap_get_dn(ds, entry);
    dnlist[0]=strdup(dn);
    ldap_memfree(dn);

    for(i=1; (entry=ldap_next_entry(ds, entry))!=NULL; i++) {
        dn=ldap_get_dn(ds, entry);
        dnlist[i]=strdup(dn);
        ldap_memfree(dn);
    }
    dnlist[i]=NULL;

    // ldap_memfree(dn);
    ldap_msgfree(res);

    ad_error_code=AD_SUCCESS;
    return dnlist;
}

int ad_mod_add(LDAP *ds, const char *dn, const char *attribute, const char *value) {
    LDAPMod *attrs[2];
    LDAPMod attr;
    char *values[2];
    int result;

    values[0] = strdup(value);
    values[1] = NULL;

    attr.mod_op = LDAP_MOD_ADD;
    attr.mod_type = strdup(attribute);
    attr.mod_values = values;
    
    attrs[0] = &attr;
    attrs[1] = NULL;

    result = ldap_modify_s(ds, dn, attrs);
    if(result!=LDAP_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in ad_mod_add, ldap_mod_s: %s\n", ldap_err2string(result));
        ad_error_code=AD_LDAP_OPERATION_FAILURE;
    } else {
        ad_error_code=AD_SUCCESS;
    }

    free(values[0]);
    free(attr.mod_type);

    return ad_error_code;
}

int ad_mod_add_binary(LDAP *ds, const char *dn, const char *attribute, const char *data, int data_length) {
    LDAPMod *attrs[2];
    LDAPMod attr;
    struct berval *values[2];
    struct berval ber_data;
    int result;

    ber_data.bv_val = strdup(data);
    ber_data.bv_len = data_length;

    values[0] = &ber_data;
    values[1] = NULL;

    attr.mod_op = LDAP_MOD_ADD|LDAP_MOD_BVALUES;
    attr.mod_type = strdup(attribute);
    attr.mod_bvalues = values;
    
    attrs[0] = &attr;
    attrs[1] = NULL;

    result = ldap_modify_s(ds, dn, attrs);
    if(result!=LDAP_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in ad_mod_add_binary, ldap_mod_s: %s\n", ldap_err2string(result));
        ad_error_code=AD_LDAP_OPERATION_FAILURE;
    } else {
        ad_error_code=AD_SUCCESS;
    }

    free(ber_data.bv_val);
    free(attr.mod_type);

    return ad_error_code;
}

int ad_mod_replace(LDAP *ds, const char *dn, const char *attribute, const char *value) {
    LDAPMod *attrs[2];
    LDAPMod attr;
    char *values[2];
    int result;

    values[0] = strdup(value);
    values[1] = NULL;

    attr.mod_op = LDAP_MOD_REPLACE;
    attr.mod_type = strdup(attribute);
    attr.mod_values = values;
    
    attrs[0] = &attr;
    attrs[1] = NULL;

    result = ldap_modify_s(ds, dn, attrs);
    if(result!=LDAP_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in ad_mod_replace, ldap_mod_s: %s\n", ldap_err2string(result));
        ad_error_code=AD_LDAP_OPERATION_FAILURE;
    } else {
        ad_error_code=AD_SUCCESS;
    }

    free(attr.mod_type);
    free(values[0]);

    return ad_error_code;
}

int ad_mod_replace_binary(LDAP *ds, const char *dn, const char *attribute, const char *data, int data_length) {
    LDAPMod *attrs[2];
    LDAPMod attr;
    struct berval *values[2];
    struct berval ber_data;
    int result;

    ber_data.bv_val = strdup(data);
    ber_data.bv_len = data_length;

    values[0] = &ber_data;
    values[1] = NULL;

    attr.mod_op = LDAP_MOD_REPLACE|LDAP_MOD_BVALUES;
    attr.mod_type = strdup(attribute);
    attr.mod_bvalues = values;
    
    attrs[0] = &attr;
    attrs[1] = NULL;

    result = ldap_modify_s(ds, dn, attrs);
    if(result!=LDAP_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in ad_mod_replace_binary, ldap_mod_s: %s\n", ldap_err2string(result));
        ad_error_code=AD_LDAP_OPERATION_FAILURE;
    } else {
        ad_error_code=AD_SUCCESS;
    }

    free(ber_data.bv_val);
    free(attr.mod_type);

    return ad_error_code;
}

int ad_mod_delete(LDAP *ds, const char *dn, const char *attribute, const char *value) {
    LDAPMod *attrs[2];
    LDAPMod attr;
    char *values[2];
    int result;

    values[0] = strdup(value);
    values[1] = NULL;

    attr.mod_op = LDAP_MOD_DELETE;
    attr.mod_type = strdup(attribute);
    attr.mod_values = values;
    
    attrs[0] = &attr;
    attrs[1] = NULL;

    result = ldap_modify_s(ds, dn, attrs);
    if(result!=LDAP_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in ad_mod_replace, ldap_mod_s: %s\n", ldap_err2string(result));
        ad_error_code=AD_LDAP_OPERATION_FAILURE;
    } else {
        ad_error_code=AD_SUCCESS;
    }

    free(values[0]);
    free(attr.mod_type);

    return ad_error_code;
}

typedef struct ber_list {
    char *attribute;
    struct berval **values;
    struct ber_list *next;
} ber_list;

char **ad_get_attribute(LDAP *ds, const char *dn, const char *attribute) {
    char *attrs[2];
    attrs[0]=strdup(attribute);
    attrs[1]=NULL;

    // TODO: use paged search (need to?)
    // adclient has an implementation
    LDAPMessage *res;
    int result = ldap_search_ext_s(ds, dn, LDAP_SCOPE_BASE, "(objectclass=*)", attrs, 0, NULL, NULL, NULL, LDAP_NO_LIMIT, &res);
    if (result != LDAP_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in ad_get_attribute when calling ldap_search_ext_s: %s",
            ldap_err2string(result));
        ad_error_code=AD_LDAP_OPERATION_FAILURE;
        return NULL;
    }

    int entries_count = ldap_count_entries(ds, res);
    if (entries_count == 0) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "No entries found in ad_get_attribute for user %s.", dn);
        ad_error_code=AD_OBJECT_NOT_FOUND;
        return NULL;
    }

    if(entries_count==0) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "No entries found in ad_get_attribute for user %s.\n", dn);
        ldap_msgfree(res);
        ad_error_code=AD_OBJECT_NOT_FOUND;
        return NULL;
    } else if(entries_count>1) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "More than one entry found in ad_get_attributes for user %s.\n", dn);
        ldap_msgfree(res);
        ad_error_code=AD_OBJECT_NOT_FOUND;
        return NULL;
    }

    LDAPMessage *entry = ldap_first_entry(ds, res);

    // Collect values into a linked list
    BerElement *berptr;
    ber_list *head = NULL;
    ber_list *prev = NULL;
    for (char *attr = ldap_first_attribute(ds, entry, &berptr); attr != NULL; attr = ldap_next_attribute(ds, entry, berptr)) {
        ber_list *node = (ber_list *)malloc(sizeof(ber_list));
        node->attribute = strdup(attr);
        node->values = ldap_get_values_len(ds, entry, attr);
        node->next= NULL;

        if (head == NULL) {
            // First entry
            head = node;
        } else {
            // Append to prev
            prev->next = node;
        }
        prev = node;

        ldap_memfree(attr);
    }

    // Turn the linked list of values into an array of key-value pairs
    char **out = NULL;
    if (head != NULL) {
        // Count values
        unsigned int total_count = 0;
        ber_list *curr = head;
        while (curr != NULL) {
            total_count += ldap_count_values_len(curr->values);

            curr = curr->next;
        }

        // NOTE: for each value we need to pair it without key(attribute description), hence the '2'
        // and reserve one more for NULL termination
        out = (char **)malloc(sizeof(char *) * (total_count * 2 + 1));
        out[total_count * 2] = NULL;

        curr = head;
        size_t out_i = 0;
        while (curr != NULL) {
            for (size_t i = 0; curr->values[i] != NULL; i++) {
                struct berval data = *(curr->values)[i];
                char bv_str[1000];
                // TODO: test if bv_len includes null terminator
                strncpy(bv_str, data.bv_val, data.bv_len);
                bv_str[data.bv_len] = '\0';
                // printf("\t%s\n", bv_str);

                out[out_i] = strdup(curr->attribute);
                out_i++;
                out[out_i] = strdup(bv_str);
                out_i++;
            }

            ber_list *prev = curr;
            curr = curr->next;
            
            // Free prev
            free(prev->attribute);
            ldap_value_free_len(prev->values);
            free(prev);
        }
    }

    free(attrs[0]);
    ber_free(berptr, 0); 
    ldap_msgfree(res);

    return out;
}

int ad_mod_rename(LDAP *ds, const char *dn, const char *new_rdn) {
    int result = ldap_rename_s(ds, dn, new_rdn, NULL, 1, NULL, NULL);
    if (result != LDAP_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in ldap_rename_s for ad_mod_rename: %s\n", ldap_err2string(result));
        return ad_error_code;
    }

    return ad_error_code;
}

int ad_rename_user(LDAP *ds, const char *dn, const char *new_name) {
    int result;
    int result_dn2domain;
    char* new_rdn = NULL;
    char* domain = NULL;
    char* upn = NULL;

    result=ad_mod_replace(ds, dn, "sAMAccountName", new_name);
    if(!result) return ad_error_code;

    result_dn2domain = dn2domain(dn, &domain);
    if (AD_SUCCESS != result_dn2domain) {
        ad_error_code = result_dn2domain;
        goto ad_rename_user_end;
    }
    upn=malloc(strlen(new_name)+strlen(domain)+2);
    sprintf(upn, "%s@%s", new_name, domain);
    result=ad_mod_replace(ds, dn, "userPrincipalName", upn);
    if (!result) {
        goto ad_rename_user_end;
    }

    new_rdn=malloc(strlen(new_name)+4);
    sprintf(new_rdn, "cn=%s", new_name);

    result = ldap_rename_s(ds, dn, new_rdn, NULL, 1, NULL, NULL);
    if (result != LDAP_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in ldap_rename_s for ad_rename_user: %s\n", ldap_err2string(result));
        goto ad_rename_user_end;
    }

ad_rename_user_end:
    if (NULL != domain) {
        free(domain);
        domain = NULL;
    }
    if (NULL != upn) {
        free(upn);
        upn = NULL;
    }
    if (NULL != new_rdn) {
        free(new_rdn);
        new_rdn = NULL;
    }

    return ad_error_code;
}

int ad_rename_group(LDAP *ds, const char *dn, const char *new_name) {
    int result;
    char *new_rdn;

    result=ad_mod_replace(ds, dn, "sAMAccountName", new_name);
    if(!result) return ad_error_code;

    new_rdn=malloc(strlen(new_name)+4);
    sprintf(new_rdn, "cn=%s", new_name);

    result = ldap_rename_s(ds, dn, new_rdn, NULL, 1, NULL, NULL);
    if (result != LDAP_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in ldap_rename_s for ad_rename_group: %s\n", ldap_err2string(result));
        return ad_error_code;
    }

    free(new_rdn);
    return ad_error_code;
}

int ad_move_user(LDAP *ds, const char *current_dn, const char *new_container) {
    int result;
    int result_dn2domain;
    char **exdn;
    char **username;
    char* domain = NULL;
    char* upn = NULL;

    // Modify userPrincipalName in case of domain change
    username=ad_get_attribute(ds, current_dn, "sAMAccountName");;
    if(username==NULL) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH,
          "Error getting username for dn %s for ad_move_user\n",
          current_dn);
        ad_error_code=AD_INVALID_DN;
        return ad_error_code;
    }
    result_dn2domain = dn2domain(new_container, &domain);
    if (AD_SUCCESS != result_dn2domain) {
        ad_error_code = result_dn2domain;
        goto ad_move_user_end;
    }
    upn=malloc(strlen(username[0])+strlen(domain)+2);
    sprintf(upn, "%s@%s", username[0], domain);
    result=ad_mod_replace(ds, current_dn, "userPrincipalName", upn);
    if (!result) {
        goto ad_move_user_end;
    }

    ad_error_code=ad_move(ds, current_dn, new_container);

ad_move_user_end:
    if (NULL != domain) {
        free(domain);
        domain = NULL;
    }
    if (NULL != upn) {
        free(upn);
        upn = NULL;
    }

    return ad_error_code;
}

int ad_move(LDAP *ds, const char *current_dn, const char *new_container) {
    int result;
    char **exdn;
    char **username, *domain, *upn;

    exdn=ldap_explode_dn(current_dn, 0);
    if(exdn==NULL) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH,
          "Error exploding dn %s for ad_move\n",
          current_dn);
        ad_error_code=AD_INVALID_DN;
        return ad_error_code;
    }

    result=ldap_rename_s(ds, current_dn, exdn[0], new_container,
        1, NULL, NULL);
    ldap_memfree(exdn);
    if(result!=LDAP_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH,
          "Error in ldap_rename_s for ad_move: %s\n",
          ldap_err2string(result));
        ad_error_code=AD_LDAP_OPERATION_FAILURE;
    } else {
        ad_error_code=AD_SUCCESS;
    }

    return ad_error_code;
}

/* returns AD_SUCCESS on success */
int ad_lock_user(LDAP *ds, const char *dn) {
    int result;
    char **flags;
    char newflags[255];
    int iflags;

    flags=ad_get_attribute(ds, dn, "userAccountControl");
    if(flags==NULL) return ad_error_code;

    iflags=atoi(flags[0]);
    iflags|=2;
    snprintf(newflags, sizeof(newflags), "%d", iflags);

    result=ad_mod_replace(ds, dn, "userAccountControl", newflags);
    if(!result) return AD_LDAP_OPERATION_FAILURE;

    return AD_SUCCESS;
}

/* Returns AD_SUCCESS on success */
int ad_unlock_user(LDAP *ds, const char *dn) {
    int result;
    char **flags;
    char newflags[255];
    int iflags;

    flags=ad_get_attribute(ds, dn, "userAccountControl");
    if(flags==NULL) return ad_error_code;

    iflags=atoi(flags[0]);
    if(iflags&2) {
        iflags^=2;
        snprintf(newflags, sizeof(newflags), "%d", iflags);
        result=ad_mod_replace(ds, dn, "userAccountControl", newflags);
        if(!result) return AD_LDAP_OPERATION_FAILURE;
    }

    return AD_SUCCESS;
}

/* 
  creates a new group
 sets objectclass=group and samaccountname=groupname
  Returns AD_SUCCESS on success 
*/
int ad_group_create(LDAP *ds, const char *group_name, const char *dn) {
    LDAPMod *attrs[4];
    LDAPMod attr1, attr2, attr3;
    int result;

    char *objectClass_values[]={"group", NULL};
    char *name_values[2];
    char *sAMAccountName_values[2];

    attr1.mod_op = LDAP_MOD_ADD;
    attr1.mod_type = "objectClass";
    attr1.mod_values = objectClass_values;

    name_values[0]=strdup(group_name);
    name_values[1]=NULL;
    attr2.mod_op = LDAP_MOD_ADD;
    attr2.mod_type = "name";
    attr2.mod_values = name_values;
    
    sAMAccountName_values[0]=strdup(group_name);
    sAMAccountName_values[1]=NULL;
    attr3.mod_op = LDAP_MOD_ADD;
    attr3.mod_type = "sAMAccountName";
    attr3.mod_values = sAMAccountName_values;

    attrs[0]=&attr1;
    attrs[1]=&attr2;
    attrs[2]=&attr3;
    attrs[3]=NULL;

    result=ldap_add_s(ds, dn, attrs);
    if(result!=LDAP_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in ldap_add: %s", ldap_err2string(result));
        ad_error_code=AD_LDAP_OPERATION_FAILURE;
    } else {
        ad_error_code=AD_SUCCESS;
    }

    free(name_values[0]);
    free(sAMAccountName_values[0]);

    return ad_error_code;
}

int ad_group_add_user(LDAP *ds, const char *group_dn, const char *user_dn) {
    return ad_mod_add(ds, group_dn, "member", user_dn);
}

int ad_group_remove_user(LDAP *ds, const char *group_dn, const char *user_dn) {
    return ad_mod_delete(ds, group_dn, "member", user_dn);
}

/* Remove the user from all groups below the given container */
int ad_group_subtree_remove_user(LDAP *ds, const char *container_dn, const char *user_dn) {
    char *filter;
    int filter_length;
    char *attrs[]={"1.1", NULL};
    LDAPMessage *res;
    LDAPMessage *entry;
    int result, num_results;
    char *group_dn=NULL;

    filter_length=(strlen(user_dn)+255);
    filter=malloc(filter_length);
    snprintf(filter, filter_length, 
        "(&(objectclass=group)(member=%s))", user_dn);

    result=ldap_search_s(ds, container_dn, LDAP_SCOPE_SUBTREE, 
        filter, attrs, 0, &res);
    if(result!=LDAP_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, 
            "Error in ldap_search_s for ad_group_subtree_remove_user: %s", 
            ldap_err2string(result));
        ad_error_code=AD_LDAP_OPERATION_FAILURE;
        return ad_error_code;
    }
    free(filter);

    num_results=ldap_count_entries(ds, res);
    if(num_results==0) {
        ad_error_code=AD_SUCCESS;
        return ad_error_code;
    }

    entry=ldap_first_entry(ds, res);
    while(entry!=NULL) {
        group_dn=ldap_get_dn(ds, entry);
        if(ad_group_remove_user(ds, group_dn, user_dn)!=AD_SUCCESS) {
            snprintf(ad_error_msg, MAX_ERR_LENGTH, 
                "Error in ad_group_subtree_remove_user"
                "\nwhen removing %s from %s:\n%s", 
                user_dn, group_dn, ad_error_msg);
            return ad_error_code;
        }
        entry=ldap_next_entry(ds, entry);
    }

    if(group_dn!=NULL) ldap_memfree(group_dn);
    ldap_msgfree(res);
    ad_error_code=AD_SUCCESS;
    return ad_error_code; 
}


/* 
  creates a new organizational unit
 sets objectclass=organizationalUnit and name=ou name
  Returns AD_SUCCESS on success 
*/
int ad_ou_create(LDAP *ds, const char *ou_name, const char *dn) {
    LDAPMod *attrs[3];
    LDAPMod attr1, attr2;
    int result;

    char *objectClass_values[]={"organizationalUnit", NULL};
    char *name_values[2];

    attr1.mod_op = LDAP_MOD_ADD;
    attr1.mod_type = "objectClass";
    attr1.mod_values = objectClass_values;

    name_values[0]=strdup(ou_name);
    name_values[1]=NULL;
    attr2.mod_op = LDAP_MOD_ADD;
    attr2.mod_type = "name";
    attr2.mod_values = name_values;
    
    attrs[0]=&attr1;
    attrs[1]=&attr2;
    attrs[2]=NULL;

    result=ldap_add_s(ds, dn, attrs);
    if(result!=LDAP_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in ldap_add: %s", ldap_err2string(result));
        ad_error_code=AD_LDAP_OPERATION_FAILURE;
    } else {
        ad_error_code=AD_SUCCESS;
    }

    free(name_values[0]);

    return ad_error_code;
}

/* ad_list returns a NULL terminated array of character strings
    with one entry for object below the given dn
    returns NULL if no values are found */
char **ad_list(LDAP *ds, const char *dn) {
    char *attrs[2];
    int result;
    LDAPMessage *res;
    LDAPMessage *entry;
    int num_entries;
    int i;
    char **dnlist;

    attrs[0]="1.1";
    attrs[1]=NULL;

    result=ldap_search_s(ds, dn, LDAP_SCOPE_ONELEVEL, "(objectclass=*)", attrs, 0, &res);
    if(result!=LDAP_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH,
            "Error in ldap_search_s for ad_list: %s",
            ldap_err2string(result));
        ad_error_code=AD_LDAP_OPERATION_FAILURE;
        return NULL;
    }
    num_entries=ldap_count_entries(ds, res);

    if(num_entries==0) {
        ldap_msgfree(res);
        return NULL;
    }

    dnlist=malloc(sizeof(char *)*(num_entries+1));

    entry=ldap_first_entry(ds, res);
    {
        char *child_dn=ldap_get_dn(ds, entry);
        dnlist[0]=strdup(child_dn);
        ldap_memfree(child_dn);
    }

    for(i=1; (entry=ldap_next_entry(ds, entry))!=NULL; i++) {
        char *child_dn=ldap_get_dn(ds, entry);
        dnlist[i]=strdup(child_dn);
        ldap_memfree(child_dn);
    }
    dnlist[i]=NULL;

    ldap_msgfree(res);

    ad_error_code=AD_SUCCESS;
    return dnlist;
}
