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

/* active_directory.c
 * generic directory management functions */

// TODO: ldap_add_s is deprecated but works, not a big deal

#if HAVE_CONFIG_H
#	include <config.h>
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

#define MAX_ERR_LENGTH 1024
char ad_error_msg[MAX_ERR_LENGTH];
int ad_error_code;

#define MAX_PASSWORD_LENGTH 255

char *user_config_file;
char *user_config_path;
char *user_config_filename="/.adtool.cfg";
// #define AD_CONFIG_FILE SYSCONFDIR "/adtool.cfg"
#define AD_CONFIG_FILE "~/adtool.cfg"
char *system_config_file=AD_CONFIG_FILE;
char *config_file;

// char *uri=NULL;
char *binddn=NULL;
char *bindpw=NULL;
// char *search_base=NULL;
// TODO: unhardcode this
char* uri = "ldap://dc0.domain.alt";
char* search_base = "DC=domain,DC=alt";

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


/* connect and authenticate to active directory server.
	returns an ldap connection identifier or 0 on error */
LDAP *ad_login() {
    static LDAP *ds=NULL;

    int version, result, bindresult;

    if(ds!=NULL) return ds;

    /* open the connection to the ldap server */
    result=ldap_initialize(&ds, uri);
    if(result!=LDAP_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error doing ldap_initialize on uri %s: %s", uri, ldap_err2string(result));
        ad_error_code=AD_SERVER_CONNECT_FAILURE;
        return 0;
    }

    // set version
    version=LDAP_VERSION3;
    result=ldap_set_option(ds, LDAP_OPT_PROTOCOL_VERSION, &version);
    if(result!=LDAP_OPT_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in ldap_set_option (protocol->v3): %s", ldap_err2string(result));
        ad_error_code=AD_SERVER_CONNECT_FAILURE;
        return 0;
    }

    // disable referrals
    result=ldap_set_option(ds, LDAP_OPT_REFERRALS, LDAP_OPT_OFF);
    if(result!=LDAP_OPT_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in ldap_set_option (referrals=0): %s", ldap_err2string(result));
        ad_error_code=AD_SERVER_CONNECT_FAILURE;
        return 0;
    }

    // NOTE: use gssapi instead of simple
    char* sasl_secprops = "maxssf=56";
    result = ldap_set_option(ds, LDAP_OPT_X_SASL_SECPROPS, (void *) sasl_secprops);
    if (result != LDAP_SUCCESS) {
        snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error in ldap_set_option (LDAP_OPT_X_SASL_SECPROPS): %s", ldap_err2string(result));
        ad_error_code=AD_SERVER_CONNECT_FAILURE;
        return 0;
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
        return 0;
    }

    // NOTE: not using this for now but might need later
    // The Man says: this function is used when an application needs to bind to another server in order to follow a referral or search continuation reference
    // ldap_set_rebind_proc(*ds, sasl_rebind_gssapi, NULL);

    return ds;
}

/* 
convert a distinguished name into the domain controller
dns domain, eg: "ou=users,dc=example,dc=com" returns
"example.com".
memory allocated should be returned with free()
*/
char *dn2domain(const char *dn) {
	char **exp_dn;
	int i;
	char *dc;

	dc=malloc(1024);
	dc[0]='\0';
	exp_dn=ldap_explode_dn(dn, 0);
	for(i=0; exp_dn[i]!=NULL; i++) {
		if(!strncasecmp("dc=", exp_dn[i], 3)) {
			strncat(dc, exp_dn[i]+3, 1024);
			strncat(dc, ".", 1024);
		}
	}
	ldap_value_free(exp_dn);
	i=strlen(dc);
	if(i>0) dc[i-1]='\0';
	for(i=0; dc[i]!='\0'; i++) dc[i]=tolower(dc[i]);
     return dc;
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
int ad_create_user(const char *username, const char *dn) {
	LDAP *ds;
	LDAPMod *attrs[5];
	LDAPMod attr1, attr2, attr3, attr4;
	int result;

	char *objectClass_values[]={"user", NULL};
	char *name_values[2];
	char *accountControl_values[]={"66050", NULL};
	char *upn, *domain;
	char *upn_values[2];

	ds=ad_login();
	if(!ds) return ad_error_code;

	attr1.mod_op = LDAP_MOD_ADD;
	attr1.mod_type = "objectClass";
	attr1.mod_values = objectClass_values;

	name_values[0]=username;
	name_values[1]=NULL;
	attr2.mod_op = LDAP_MOD_ADD;
	attr2.mod_type = "sAMAccountName";
	attr2.mod_values = name_values;
	
	attr3.mod_op = LDAP_MOD_ADD;
	attr3.mod_type = "userAccountControl";
	attr3.mod_values = accountControl_values;

	domain=dn2domain(dn);
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

	free(domain);
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
int ad_create_computer(const char *name, const char *dn) {
    LDAP *ds;
    LDAPMod *attrs[4];
    LDAPMod attr1, attr2, attr3;
    int i, result;

    char *objectClass_values[]={"top", "person", "organizationalPerson",
    "user", "computer", NULL};
    char *name_values[2];
    char *accountControl_values[]={"4128", NULL};

    ds=ad_login();
    if(!ds) return ad_error_code;

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
    return ad_error_code;
}

/* ad_object_delete deletes the given dn
	returns non-zero on success */
int ad_object_delete(const char *dn) {
	LDAP *ds;
	int result;

	ds=ad_login();
	if(!ds) return ad_error_code;

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
int ad_setpass(const char *dn, const char *password) {
	LDAP *ds;
	char quoted_password[MAX_PASSWORD_LENGTH+2];
	char unicode_password[(MAX_PASSWORD_LENGTH+2)*2];
	int i;
	LDAPMod *attrs[2];
	LDAPMod attr1;
	struct berval *bervalues[2];
	struct berval pw;
	int result;

	ds=ad_login();
	if(!ds) return ad_error_code;

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
char **ad_search(const char *attribute, const char *value) {
	LDAP *ds;
	char *filter;
	int filter_length;
	char *attrs[]={"1.1", NULL};
	LDAPMessage *res;
	LDAPMessage *entry;
	int i, result, num_results;
	char **dnlist;
	char *dn;

	ds=ad_login();
	if(!ds) return (char **)-1;

	if(!search_base) {
		snprintf(ad_error_msg, MAX_ERR_LENGTH, "Error: couldn't read active directory searchbase parameter from config file %s, ~/.adtool.cfg or command line", config_file);
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

int ad_mod_add(const char *dn, const char *attribute, const char *value) {
	LDAP *ds;
	LDAPMod *attrs[2];
	LDAPMod attr;
	char *values[2];
	int result;

	ds=ad_login();
	if(!ds) return ad_error_code;

	values[0] = value;
	values[1] = NULL;

	attr.mod_op = LDAP_MOD_ADD;
	attr.mod_type = attribute;
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
	return ad_error_code;
}

int ad_mod_add_binary(const char *dn, const char *attribute, const char *data, int data_length) {
	LDAP *ds;
	LDAPMod *attrs[2];
	LDAPMod attr;
	struct berval *values[2];
	struct berval ber_data;
	int result;

	ds=ad_login();
	if(!ds) return ad_error_code;

	ber_data.bv_val = data;
	ber_data.bv_len = data_length;

	values[0] = &ber_data;
	values[1] = NULL;

	attr.mod_op = LDAP_MOD_ADD|LDAP_MOD_BVALUES;
	attr.mod_type = attribute;
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
	return ad_error_code;
}

int ad_mod_replace(const char *dn, const char *attribute, const char *value) {
	LDAP *ds;
	LDAPMod *attrs[2];
	LDAPMod attr;
	char *values[2];
	int result;

	ds=ad_login();
	if(!ds) return ad_error_code;

	values[0] = value;
	values[1] = NULL;

	attr.mod_op = LDAP_MOD_REPLACE;
	attr.mod_type = attribute;
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
	return ad_error_code;
}

int ad_mod_replace_binary(const char *dn, const char *attribute, const char *data, int data_length) {
	LDAP *ds;
	LDAPMod *attrs[2];
	LDAPMod attr;
	struct berval *values[2];
	struct berval ber_data;
	int result;

	ds=ad_login();
	if(!ds) return ad_error_code;

	ber_data.bv_val = data;
	ber_data.bv_len = data_length;

	values[0] = &ber_data;
	values[1] = NULL;

	attr.mod_op = LDAP_MOD_REPLACE|LDAP_MOD_BVALUES;
	attr.mod_type = attribute;
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
	return ad_error_code;
}

int ad_mod_delete(const char *dn, const char *attribute, const char *value) {
	LDAP *ds;
	LDAPMod *attrs[2];
	LDAPMod attr;
	char *values[2];
	int result;

	ds=ad_login();
	if(!ds) return ad_error_code;

	values[0] = value;
	values[1] = NULL;

	attr.mod_op = LDAP_MOD_DELETE;
	attr.mod_type = attribute;
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
	return ad_error_code;
}

typedef struct ber_list {
    char *attribute;
    struct berval **values;
    struct ber_list *next;
} ber_list;

char **ad_get_attribute(const char *dn, const char *attribute) {
    LDAP *ds=ad_login();
    if(!ds) return NULL;

    char *attrs[2];
    attrs[0]=attribute;
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
    for (char *attribute = ldap_first_attribute(ds, entry, &berptr); attribute != NULL; attribute = ldap_next_attribute(ds, entry, berptr)) {
        ber_list *node = (ber_list *)malloc(sizeof(ber_list));
        node->attribute = strdup(attribute);
        node->values = ldap_get_values_len(ds, entry, attribute);
        node->next= NULL;

        if (head == NULL) {
            // First entry
            head = node;
        } else {
            // Append to prev
            prev->next = node;
        }
        prev = node;

        ldap_memfree(attribute);
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

    ber_free(berptr, 0); 
    ldap_msgfree(res);

    return out;
}

/* ad_get_attribute returns a NULL terminated array of character strings
	with one entry for each attribute/value pair
	returns NULL if no values are found */
// char **ad_get_attribute_old(char *dn, char *attribute) {
// 	LDAP *ds;
// 	char **values;
// 	int result;
// 	char *attrs[2];
// 	LDAPMessage *res;
// 	LDAPMessage *entry;
// 	int num_entries;

// 	ds=ad_login();
// 	if(!ds) return NULL;

// 	attrs[0]=attribute;
// 	attrs[1]=NULL;

// 	result=ldap_search_s(ds, dn, LDAP_SCOPE_BASE, "(objectclass=*)", attrs, 0, &res);
// 	if(result!=LDAP_SUCCESS) {
// 		snprintf(ad_error_msg, MAX_ERR_LENGTH,
// 			"Error in ldap_search_s for ad_get_attribute: %s",
// 			ldap_err2string(result));
// 		ad_error_code=AD_LDAP_OPERATION_FAILURE;
// 		return NULL;
// 	}
// 	num_entries=ldap_count_entries(ds, res);
// 	if(num_entries==0) {
// 		snprintf(ad_error_msg, MAX_ERR_LENGTH, 
// 			"No entries found in ad_get_attribute for user %s.",
// 			dn);
// 		ldap_msgfree(res);
// 		ad_error_code=AD_OBJECT_NOT_FOUND;
// 		return NULL;
// 	} else if(num_entries>1) {
// 		snprintf(ad_error_msg, MAX_ERR_LENGTH, 
// 			"More than one entry found in "
// 			"ad_get_attributes for user %s.",
// 			dn);
// 		ldap_msgfree(res);
// 		ad_error_code=AD_OBJECT_NOT_FOUND;
// 		return NULL;
// 	}

// 	entry=ldap_first_entry(ds, res);
// 	values=ldap_get_values(ds, entry, attribute);
// 	if(values==NULL) {
// 		snprintf(ad_error_msg, MAX_ERR_LENGTH,
// 			"Error in ldap_get_values for ad_get_attribute:"
// 			"no values found for attribute %s in object %s",
// 			attribute, dn);
// 		ad_error_code=AD_ATTRIBUTE_ENTRY_NOT_FOUND;
// 	} else {
// 		ad_error_code=AD_SUCCESS;
// 	}
// 	return values;
// }

/* 
  rename a user
  changes samaccountname, userprincipalname and rdn/cn
	return AD_SUCCESS on success 
*/
int ad_rename_user(const char *dn, const char *new_username) {
	LDAP *ds;
	int result;
	char *new_rdn;
	char *domain, *upn;

	ds=ad_login();
	if(!ds) return ad_error_code;

	result=ad_mod_replace(dn, "sAMAccountName", new_username);
	if(!result) return ad_error_code;

	domain=dn2domain(dn);
	upn=malloc(strlen(new_username)+strlen(domain)+2);
	sprintf(upn, "%s@%s", new_username, domain);
	free(domain);
	result=ad_mod_replace(dn, "userPrincipalName", upn);
	free(upn);
	if(!result) return ad_error_code;

	new_rdn=malloc(strlen(new_username)+4);
	sprintf(new_rdn, "cn=%s", new_username);

	result=ldap_modrdn2_s(ds, dn, new_rdn, 1);
	if(result!=LDAP_SUCCESS) {
		snprintf(ad_error_msg, MAX_ERR_LENGTH,
          "Error in ldap_modrdn2_s for ad_rename_user: %s\n",
          ldap_err2string(result));
		ad_error_code=AD_LDAP_OPERATION_FAILURE;
		free(new_rdn);
		return ad_error_code;
	}

	ad_error_code=AD_SUCCESS;
	free(new_rdn);
	return ad_error_code;
}

/* 
  move a user to another container
  sets userprincipalname based on the destination container
	return AD_SUCCESS on success 
*/
int ad_move_user(const char *current_dn, const char *new_container) {
	LDAP *ds;
	int result;
	char **exdn;
	char **username, *domain, *upn;

	ds=ad_login();
	if(!ds) return ad_error_code;

	username=ad_get_attribute(current_dn, "sAMAccountName");;
	if(username==NULL) {
		snprintf(ad_error_msg, MAX_ERR_LENGTH,
          "Error getting username for dn %s for ad_move_user\n",
          current_dn);
		ad_error_code=AD_INVALID_DN;
		return ad_error_code;
	}
	domain=dn2domain(new_container);
	upn=malloc(strlen(username[0])+strlen(domain)+2);
	sprintf(upn, "%s@%s", username[0], domain);
	free(domain);
	result=ad_mod_replace(current_dn, "userPrincipalName", upn);
	free(upn);
	if(!result) return ad_error_code;

	exdn=ldap_explode_dn(current_dn, 0);
	if(exdn==NULL) {
		snprintf(ad_error_msg, MAX_ERR_LENGTH,
          "Error exploding dn %s for ad_move_user\n",
          current_dn);
		ad_error_code=AD_INVALID_DN;
		return ad_error_code;
	}

	result=ldap_rename_s(ds, current_dn, exdn[0], new_container,
        1, NULL, NULL);
	ldap_memfree(exdn);
	if(result!=LDAP_SUCCESS) {
		snprintf(ad_error_msg, MAX_ERR_LENGTH,
          "Error in ldap_rename_s for ad_move_user: %s\n",
          ldap_err2string(result));
		ad_error_code=AD_LDAP_OPERATION_FAILURE;
	} else {
		ad_error_code=AD_SUCCESS;
	}
	return ad_error_code;
}

/* returns AD_SUCCESS on success */
int ad_lock_user(const char *dn) {
	LDAP *ds;
	int result;
	char **flags;
	char newflags[255];
	int iflags;

	ds=ad_login();
	if(!ds) return ad_error_code;

	flags=ad_get_attribute(dn, "userAccountControl");
	if(flags==NULL) return ad_error_code;

	iflags=atoi(flags[0]);
	iflags|=2;
	snprintf(newflags, sizeof(newflags), "%d", iflags);

	result=ad_mod_replace(dn, "userAccountControl", newflags);
	if(!result) return AD_LDAP_OPERATION_FAILURE;

	return AD_SUCCESS;
}

/* Returns AD_SUCCESS on success */
int ad_unlock_user(const char *dn) {
	LDAP *ds;
	int result;
	char **flags;
	char newflags[255];
	int iflags;

	ds=ad_login();
	if(!ds) return ad_error_code;

	flags=ad_get_attribute(dn, "userAccountControl");
	if(flags==NULL) return ad_error_code;

	iflags=atoi(flags[0]);
	if(iflags&2) {
		iflags^=2;
		snprintf(newflags, sizeof(newflags), "%d", iflags);
		result=ad_mod_replace(dn, "userAccountControl", newflags);
		if(!result) return AD_LDAP_OPERATION_FAILURE;
	}

	return AD_SUCCESS;
}

/* 
  creates a new group
 sets objectclass=group and samaccountname=groupname
  Returns AD_SUCCESS on success 
*/
int ad_group_create(const char *group_name, const char *dn) {
	LDAP *ds;
	LDAPMod *attrs[4];
	LDAPMod attr1, attr2, attr3;
	int result;

	char *objectClass_values[]={"group", NULL};
	char *name_values[2];
	char *sAMAccountName_values[2];

	ds=ad_login();
	if(!ds) return ad_error_code;

	attr1.mod_op = LDAP_MOD_ADD;
	attr1.mod_type = "objectClass";
	attr1.mod_values = objectClass_values;

	name_values[0]=group_name;
	name_values[1]=NULL;
	attr2.mod_op = LDAP_MOD_ADD;
	attr2.mod_type = "name";
	attr2.mod_values = name_values;
	
	sAMAccountName_values[0]=group_name;
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
	return ad_error_code;
}

int ad_group_add_user(const char *group_dn, const char *user_dn) {
	return ad_mod_add(group_dn, "member", user_dn);
}

int ad_group_remove_user(const char *group_dn, const char *user_dn) {
	return ad_mod_delete(group_dn, "member", user_dn);
}

/* Remove the user from all groups below the given container */
int ad_group_subtree_remove_user(const char *container_dn, const char *user_dn) {
	LDAP *ds;
	char *filter;
	int filter_length;
	char *attrs[]={"1.1", NULL};
	LDAPMessage *res;
	LDAPMessage *entry;
	int result, num_results;
	char *group_dn=NULL;

	ds=ad_login();
	if(!ds) return ad_error_code;

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
		if(ad_group_remove_user(group_dn, user_dn)!=AD_SUCCESS) {
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
int ad_ou_create(const char *ou_name, const char *dn) {
	LDAP *ds;
	LDAPMod *attrs[3];
	LDAPMod attr1, attr2;
	int result;

	char *objectClass_values[]={"organizationalUnit", NULL};
	char *name_values[2];

	ds=ad_login();
	if(!ds) return ad_error_code;

	attr1.mod_op = LDAP_MOD_ADD;
	attr1.mod_type = "objectClass";
	attr1.mod_values = objectClass_values;

	name_values[0]=ou_name;
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
	return ad_error_code;
}

/* ad_list returns a NULL terminated array of character strings
	with one entry for object below the given dn
	returns NULL if no values are found */
char **ad_list(const char *dn) {
	LDAP *ds;
	char *attrs[2];
	int result;
	LDAPMessage *res;
	LDAPMessage *entry;
	int num_entries;
	int i;
	char **dnlist;

	ds=ad_login();
	if(!ds) return NULL;

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
	dn=ldap_get_dn(ds, entry);
	dnlist[0]=strdup(dn);
    // NOTE: this should be freed here, right?
    ldap_memfree(dn);

    for(i=1; (entry=ldap_next_entry(ds, entry))!=NULL; i++) {
      dn=ldap_get_dn(ds, entry);
      dnlist[i]=strdup(dn);
        // NOTE: this should be freed here, right?
      ldap_memfree(dn);
  }
  dnlist[i]=NULL;

	// ldap_memfree(dn);
  ldap_msgfree(res);

  ad_error_code=AD_SUCCESS;
  return dnlist;
}

