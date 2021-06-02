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

#include "ad_interface.h"
#include "ad_interface_p.h"

#include "ad_utils.h"
#include "ad_object.h"
#include "ad_display.h"
#include "ad_config.h"
#include "samba/ndr_security.h"
#include "samba/gp_manage.h"
#include "samba/dom_sid.h"

#include "ad_filter.h"

#include <ldap.h>
#include <lber.h>
#include <resolv.h>
#include <libsmbclient.h>
#include <uuid/uuid.h>
#include <krb5.h>
#include <sasl/sasl.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <QTextCodec>
#include <QDebug>

// NOTE: LDAP library char* inputs are non-const in the API
// but are const for practical purposes so we use forced
// casts (const char *) -> (char *)

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

QList<QString> query_server_for_hosts(const char *dname);
bool ad_connect(const char* uri, LDAP **ld_out);
int sasl_interact_gssapi(LDAP *ld, unsigned flags, void *indefaults, void *in);

AdConfig *AdInterfacePrivate::s_adconfig = nullptr;
bool AdInterfacePrivate::s_log_searches = false;
QString AdInterfacePrivate::s_dc = QString();

// TODO: values of SEC_ADS_GENERIC_READ and
// SEC_ADS_GENERIC_WRITE constants don't match with the bits
// that ADUC sets when you enable those permissions in
// security tab. There are some extra bits in these
// constants, took them out as a quick fix.
const QHash<AcePermission, uint32_t> ace_permission_to_mask_map = {
    {AcePermission_FullControl, SEC_ADS_GENERIC_ALL},
    // {AcePermission_Read, SEC_ADS_GENERIC_READ},
    {AcePermission_Read, (SEC_STD_READ_CONTROL | SEC_ADS_LIST | SEC_ADS_READ_PROP)},
    // {AcePermission_Write, SEC_ADS_GENERIC_WRITE},
    {AcePermission_Write, (SEC_ADS_SELF_WRITE | SEC_ADS_WRITE_PROP)},
    {AcePermission_CreateChild, SEC_ADS_CREATE_CHILD},
    {AcePermission_DeleteChild, SEC_ADS_DELETE_CHILD},
    {AcePermission_AllowedToAuthenticate, SEC_ADS_CONTROL_ACCESS},
    {AcePermission_ChangePassword, SEC_ADS_CONTROL_ACCESS},
    {AcePermission_ReceiveAs, SEC_ADS_CONTROL_ACCESS},
    {AcePermission_ResetPassword, SEC_ADS_CONTROL_ACCESS},
    {AcePermission_SendAs, SEC_ADS_CONTROL_ACCESS},
    {AcePermission_ReadAccountRestrictions, SEC_ADS_READ_PROP},
    {AcePermission_WriteAccountRestrictions, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadGeneralInfo, SEC_ADS_READ_PROP},
    {AcePermission_WriteGeneralInfo, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadGroupMembership, SEC_ADS_READ_PROP},
    {AcePermission_ReadLogonInfo, SEC_ADS_READ_PROP},
    {AcePermission_WriteLogonInfo, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadPersonalInfo, SEC_ADS_READ_PROP},
    {AcePermission_WritePersonalInfo, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadPhoneAndMailOptions, SEC_ADS_READ_PROP},
    {AcePermission_WritePhoneAndMailOptions, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadPrivateInfo, SEC_ADS_READ_PROP},
    {AcePermission_WritePrivateInfo, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadPublicInfo, SEC_ADS_READ_PROP},
    {AcePermission_WritePublicInfo, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadRemoteAccessInfo, SEC_ADS_READ_PROP},
    {AcePermission_WriteRemoteAccessInfo, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadTerminalServerLicenseServer, SEC_ADS_READ_PROP},
    {AcePermission_WriteTerminalServerLicenseServer, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadWebInfo, SEC_ADS_READ_PROP},
    {AcePermission_WriteWebInfo, SEC_ADS_WRITE_PROP},
};

// NOTE: store right's cn value here, then search for it to
// get right's guid, which is compared to ace type.
const QHash<AcePermission, QString> ace_permission_to_type_map = {
    {AcePermission_AllowedToAuthenticate, "Allowed-To-Authenticate"},
    {AcePermission_ChangePassword, "User-Change-Password"},
    {AcePermission_ReceiveAs, "Receive-As"},
    {AcePermission_ResetPassword, "User-Force-Change-Password"},
    {AcePermission_SendAs, "Send-As"},
    {AcePermission_ReadAccountRestrictions, "User-Account-Restrictions"},
    {AcePermission_WriteAccountRestrictions, "User-Account-Restrictions"},
    {AcePermission_ReadGeneralInfo, "General-Information"},
    {AcePermission_WriteGeneralInfo, "General-Information"},
    {AcePermission_ReadGroupMembership, "Membership"},
    {AcePermission_ReadLogonInfo, "User-Logon"},
    {AcePermission_WriteLogonInfo, "User-Logon"},
    {AcePermission_ReadPersonalInfo, "Personal-Information"},
    {AcePermission_WritePersonalInfo, "Personal-Information"},
    {AcePermission_ReadPhoneAndMailOptions, "Email-Information"},
    {AcePermission_WritePhoneAndMailOptions, "Email-Information"},
    {AcePermission_ReadPrivateInfo, "Private-Information"},
    {AcePermission_WritePrivateInfo, "Private-Information"},
    {AcePermission_ReadPublicInfo, "Public-Information"},
    {AcePermission_WritePublicInfo, "Public-Information"},
    {AcePermission_ReadRemoteAccessInfo, "RAS-Information"},
    {AcePermission_WriteRemoteAccessInfo, "RAS-Information"},
    {AcePermission_ReadTerminalServerLicenseServer, "Terminal-Server-License-Server"},
    {AcePermission_WriteTerminalServerLicenseServer, "Terminal-Server-License-Server"},
    {AcePermission_ReadWebInfo, "Web-Information"},
    {AcePermission_WriteWebInfo, "Web-Information"}
};

const QList<AcePermission> all_permissions_list =
[]() {
    QList<AcePermission> out;

    for (int permission_i = 0; permission_i < AcePermission_COUNT; permission_i++) {
        const AcePermission permission = (AcePermission) permission_i;
        out.append(permission);
    }

    return out;
}();

const QSet<AcePermission> all_permissions = all_permissions_list.toSet();

QSet<AcePermission> get_permission_set(const uint32_t mask) {
    QSet<AcePermission> out;

    for (const AcePermission &permission : all_permissions) {
        const uint32_t this_mask = ace_permission_to_mask_map[permission];

        if (this_mask == mask) {
            out.insert(permission);
        }
    }

    return out;
}

const QSet<AcePermission> read_prop_permissions = get_permission_set(SEC_ADS_READ_PROP);
const QSet<AcePermission> write_prop_permissions = get_permission_set(SEC_ADS_WRITE_PROP);

void get_auth_data_fn(const char * pServer, const char * pShare, char * pWorkgroup, int maxLenWorkgroup, char * pUsername, int maxLenUsername, char * pPassword, int maxLenPassword) {

}

AdInterface::AdInterface(AdConfig *adconfig) {
    d = new AdInterfacePrivate();

    if (adconfig != nullptr) {
        d->adconfig = adconfig;
    } else if (AdInterfacePrivate::s_adconfig != nullptr) {
        d->adconfig = AdInterfacePrivate::s_adconfig;
    } else {
        d->adconfig = nullptr;
    }

    d->ld = NULL;
    d->smbc = NULL;
    d->is_connected = false;

    d->domain = get_default_domain_from_krb5();
    d->domain_head = domain_to_domain_dn(d->domain);

    const QString dc =
    []() {
        if (!AdInterfacePrivate::s_dc.isEmpty()) {
            return AdInterfacePrivate::s_dc;
        } else {
            const QString domain = get_default_domain_from_krb5();
            const QList<QString> host_list = get_domain_hosts(domain, QString());

            if (!host_list.isEmpty()) {
                return host_list[0];
            } else {
                return QString();
            }
        }
    }();

    const QString uri = "ldap://" + dc;

    const bool success = ad_connect(cstr(uri), &d->ld);
    if (success) {
        // NOTE: this doesn't leak memory. False positive.
        smbc_init(get_auth_data_fn, 0);
        d->smbc = smbc_new_context();
        smbc_setOptionUseKerberos(d->smbc, true);
        smbc_setOptionFallbackAfterKerberos(d->smbc, true);
        if (!smbc_init_context(d->smbc)) {
            smbc_free_context(d->smbc, 0);
            qDebug() << "Could not initialize smbc context";
        }
        smbc_set_context(d->smbc);

        d->is_connected = true;
    }
}

AdInterface::~AdInterface() {
    smbc_free_context(d->smbc, 0);

    if (d->is_connected) {
        ldap_unbind_ext(d->ld, NULL, NULL);
    } else {
        ldap_memfree(d->ld);
    }

    delete d;
}

void AdInterface::set_permanent_adconfig(AdConfig *adconfig) {
    AdInterfacePrivate::s_adconfig = adconfig;
}

void AdInterface::set_log_searches(const bool enabled) {
    AdInterfacePrivate::s_log_searches = enabled;
}

void AdInterface::set_dc(const QString &dc) {
    AdInterfacePrivate::s_dc = dc;
}

QString AdInterface::get_dc() {
    return AdInterfacePrivate::s_dc;
}

AdInterfacePrivate::AdInterfacePrivate() {

}

bool AdInterface::is_connected() const {
    return d->is_connected;
}

QList<AdMessage> AdInterface::messages() const {
    return d->messages;
}

bool AdInterface::any_error_messages() const {
    for (const auto &message : d->messages) {
        if (message.type() == AdMessageType_Error) {
            return true;
        }
    }

    return false;
}

void AdInterface::clear_messages() {
    d->messages.clear();
}

// Helper f-n for search()
// NOTE: cookie is starts as NULL. Then after each while
// loop, it is set to the value returned by
// ldap_search_ext_s(). At the end cookie is set back to
// NULL.
bool AdInterfacePrivate::search_paged_internal(const char *base, const int scope, const char *filter, char **attributes, QHash<QString, AdObject> *results, AdCookie *cookie) {
    int result;
    LDAPMessage *res = NULL;
    LDAPControl *page_control = NULL;
    LDAPControl **returned_controls = NULL;
    struct berval *prev_cookie = cookie->cookie;
    struct berval *new_cookie = NULL;
    
    auto cleanup =
    [&]() {
        ldap_msgfree(res);
        ldap_control_free(page_control);
        ldap_controls_free(returned_controls);
        ber_bvfree(prev_cookie);
        ber_bvfree(new_cookie);
    };

    // Create page control
    const ber_int_t page_size = 1000;
    const int is_critical = 1;
    result = ldap_create_page_control(ld, page_size, prev_cookie, is_critical, &page_control);
    if (result != LDAP_SUCCESS) {
        qDebug() << "Failed to create page control: " << ldap_err2string(result);

        cleanup();
        return false;
    }
    LDAPControl *server_controls[2] = {page_control, NULL};

    // Perform search
    const int attrsonly = 0;
    result = ldap_search_ext_s(ld, base, scope, filter, attributes, attrsonly, server_controls, NULL, NULL, LDAP_NO_LIMIT, &res);
    if ((result != LDAP_SUCCESS) && (result != LDAP_PARTIAL_RESULTS)) {
        // NOTE: it's not really an error for an object to
        // not exist. For example, sometimes it's needed to
        // check whether an object exists. Not sure how to
        // distinguish this error type from others
        if (result != LDAP_NO_SUCH_OBJECT) {
            qDebug() << "Error in paged ldap_search_ext_s: " << ldap_err2string(result);
        }

        cleanup();
        return false;
    }

    // Collect results for this search
    for (LDAPMessage *entry = ldap_first_entry(ld, res); entry != NULL; entry = ldap_next_entry(ld, entry)) {
        char *dn_cstr = ldap_get_dn(ld, entry);
        const QString dn(dn_cstr);
        ldap_memfree(dn_cstr);

        QHash<QString, QList<QByteArray>> object_attributes;

        BerElement *berptr;
        for (char *attr = ldap_first_attribute(ld, entry, &berptr); attr != NULL; attr = ldap_next_attribute(ld, entry, berptr)) {
            struct berval **values_ldap = ldap_get_values_len(ld, entry, attr);

            const QList<QByteArray> values_bytes =
            [=]() {
                QList<QByteArray> out;

                if (values_ldap != NULL) {
                    const int values_count = ldap_count_values_len(values_ldap);
                    for (int i = 0; i < values_count; i++) {
                        struct berval value_berval = *values_ldap[i];
                        const QByteArray value_bytes(value_berval.bv_val, value_berval.bv_len);

                        out.append(value_bytes);
                    }
                }

                return out;
            }();

            const QString attribute(attr);

            object_attributes[attribute] = values_bytes;

            ldap_value_free_len(values_ldap);
            ldap_memfree(attr);
        }
        ber_free(berptr, 0);

        AdObject object;
        object.load(dn, object_attributes);

        results->insert(dn, object);
    }

    // Parse the results to retrieve returned controls
    int errcodep;
    result = ldap_parse_result(ld, res, &errcodep, NULL, NULL, NULL, &returned_controls, false);
    if (result != LDAP_SUCCESS) {
        qDebug() << "Failed to parse result: " << ldap_err2string(result);

        cleanup();
        return false;
    }

    // Get page response control
    LDAPControl *pageresponse_control = ldap_control_find(LDAP_CONTROL_PAGEDRESULTS, returned_controls, NULL);
    if (pageresponse_control == NULL) {
        qDebug() << "Failed to find PAGEDRESULTS control";

        cleanup();
        return false;
    }

    // Parse page response control to determine whether
    // there are more pages
    ber_int_t total_count;
    new_cookie = (struct berval *) malloc(sizeof(struct berval));
    result = ldap_parse_pageresponse_control(ld, pageresponse_control, &total_count, new_cookie);
    if (result != LDAP_SUCCESS) {
        qDebug() << "Failed to parse pageresponse control: " << ldap_err2string(result);
        
        cleanup();
        return false;
    }

    // Switch to new cookie if there are more pages
    // NOTE: there are more pages if the cookie isn't
    // empty
    const bool more_pages = (new_cookie->bv_len > 0);
    if (more_pages) {
        cookie->cookie = ber_bvdup(new_cookie);
    } else {
        cookie->cookie = NULL;
    }

    cleanup();
    return true;
}

QHash<QString, AdObject> AdInterface::search(const QString &base, const SearchScope scope, const QString &filter, const QList<QString> &attributes) {
    AdCookie cookie;
    QHash<QString, AdObject> results;

    if (AdInterfacePrivate::s_log_searches) {
        const QString attributes_string = "{" + attributes.join(",") + "}";

        const QString scope_string =
        [&scope]() -> QString {
            switch (scope) {
                case SearchScope_Object: return "object";
                case SearchScope_Children: return "children";
                case SearchScope_Descendants: return "descendants";
                case SearchScope_All: return "all";
                default: break;
            }
            return QString();
        }();

        d->success_message(QString(tr("Search:\n\tfilter = \"%1\"\n\tattributes = %2\n\tscope = \"%3\"\n\tbase = \"%4\"")).arg(filter, attributes_string, scope_string, base));
    }

    while (true) {
        const bool success = search_paged(base, scope, filter, attributes, &results, &cookie);

        if (!success) {
            break;
        }

        if (!cookie.more_pages()) {
            break;
        }
    }

    return results;
}

bool AdInterface::search_paged(const QString &base, const SearchScope scope, const QString &filter, const QList<QString> &attributes, QHash<QString, AdObject> *results, AdCookie *cookie) {
    const char *base_cstr = cstr(base);

    const int scope_int =
    [&]() {
        switch (scope) {
            case SearchScope_Object: return LDAP_SCOPE_BASE;
            case SearchScope_Children: return LDAP_SCOPE_ONELEVEL;
            case SearchScope_All: return LDAP_SCOPE_SUBTREE;
            case SearchScope_Descendants: return LDAP_SCOPE_CHILDREN;
        }
        return 0;
    }();

    const char *filter_cstr =
    [&]() {
        if (filter.isEmpty()) {
            // NOTE: need to pass NULL instead of empty
            // string to denote "no filter"
            return (const char *) NULL;
        } else {
            return cstr(filter);
        }
    }();

    // Convert attributes list to NULL-terminated array
    char **attributes_array =
    [&]() {
        char **out;
        if (attributes.isEmpty()) {
            // Pass NULL so LDAP gets all attributes
            out = NULL;
        } else {
            out = (char **) malloc((attributes.size() + 1) * sizeof(char *));

            if (out != NULL) {
                for (int i = 0; i < attributes.size(); i++) {
                    const QString attribute = attributes[i];
                    out[i] = strdup(cstr(attribute));
                }
                out[attributes.size()] = NULL;
            }
        }

        return out;
    }();

    const bool search_success = d->search_paged_internal(base_cstr, scope_int, filter_cstr, attributes_array, results, cookie);
    if (!search_success) {
        results->clear();

        return false;
    }

    if (attributes_array != NULL) {
        for (int i = 0; attributes_array[i] != NULL; i++) {
            free(attributes_array[i]);
        }
        free(attributes_array);
    }

    return true;
}

AdObject AdInterface::search_object(const QString &dn, const QList<QString> &attributes) {
    const QString base = dn;
    const SearchScope scope = SearchScope_Object;
    const QString filter = QString();
    const QHash<QString, AdObject> results = search(base, scope, filter, attributes);

    if (results.contains(dn)) {
        return results[dn];
    } else {
        return AdObject();
    }
}

bool AdInterface::attribute_replace_values(const QString &dn, const QString &attribute, const QList<QByteArray> &values, const DoStatusMsg do_msg) {
    const AdObject object = search_object(dn, {attribute});
    const QList<QByteArray> old_values = object.get_values(attribute);
    const QString name = dn_get_name(dn);
    const QString values_display = attribute_display_values(attribute, values, d->adconfig);
    const QString old_values_display = attribute_display_values(attribute, old_values, d->adconfig);

    // Do nothing if both new and old values are empty
    if (old_values.isEmpty() && values.isEmpty()) {
        return true;
    }

    // NOTE: store bvalues in array instead of dynamically allocating ptrs
    struct berval bvalues_storage[values.size()];
    struct berval *bvalues[values.size() + 1];
    bvalues[values.size()] = NULL;
    for (int i = 0; i < values.size(); i++) {
        const QByteArray value = values[i];
        struct berval *bvalue = &(bvalues_storage[i]);

        bvalue->bv_val = (char *) value.constData();
        bvalue->bv_len = (size_t) value.size();

        bvalues[i] = bvalue;
    }

    LDAPMod attr;
    attr.mod_op = (LDAP_MOD_REPLACE | LDAP_MOD_BVALUES);
    attr.mod_type = (char *) cstr(attribute);
    attr.mod_bvalues = bvalues;
    
    LDAPMod *attrs[] = {&attr, NULL};

    const int result = ldap_modify_ext_s(d->ld, cstr(dn), attrs, NULL, NULL);

    if (result == LDAP_SUCCESS) {
        d->success_message(QString(tr("Changed attribute \"%1\" of object \"%2\" from \"%3\" to \"%4\"")).arg(attribute, name, old_values_display, values_display), do_msg);

        return true;
    } else {
        const QString context = QString(tr("Failed to change attribute \"%1\" of object \"%2\" from \"%3\" to \"%4\"")).arg(attribute, name, old_values_display, values_display);

        d->error_message(context, d->default_error(), do_msg);

        return false;
    }
}

bool AdInterface::attribute_replace_value(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg) {
    const QList<QByteArray> values =
    [=]() -> QList<QByteArray> {
        if (value.isEmpty()) {
            return QList<QByteArray>();
        } else {
            return {value};
        }
    }();

    return attribute_replace_values(dn, attribute, values, do_msg);
}

bool AdInterface::attribute_add_value(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg) {
    char *data_copy = (char *) malloc(value.size());
    if (data_copy == NULL) {
        return false;
    }
    memcpy(data_copy, value.constData(), value.size());

    struct berval ber_data;
    ber_data.bv_val = data_copy;
    ber_data.bv_len = value.size();

    struct berval *values[] = {&ber_data, NULL};

    LDAPMod attr;
    attr.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    attr.mod_type = (char *)cstr(attribute);
    attr.mod_bvalues = values;
    
    LDAPMod *attrs[] = {&attr, NULL};

    const int result = ldap_modify_ext_s(d->ld, cstr(dn), attrs, NULL, NULL);
    free(data_copy);

    const QString name = dn_get_name(dn);
    const QString new_display_value = attribute_display_value(attribute, value, d->adconfig);

    if (result == LDAP_SUCCESS) {
        const QString context = QString(tr("Added value \"%1\" for attribute \"%2\" of object \"%3\"")).arg(new_display_value, attribute, name);

        d->success_message(context, do_msg);

        return true;
    } else {
        const QString context = QString(tr("Failed to add value \"%1\" for attribute \"%2\" of object \"%3\"")).arg(new_display_value, attribute, name);

        d->error_message(context, d->default_error(), do_msg);

        return false;
    }
}

bool AdInterface::attribute_delete_value(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg) {
    const QString name = dn_get_name(dn);
    const QString value_display = attribute_display_value(attribute, value, d->adconfig);

    char *data_copy = (char *) malloc(value.size());
    if (data_copy == NULL) {
        return false;
    }
    memcpy(data_copy, value.constData(), value.size());

    struct berval ber_data;
    ber_data.bv_val = data_copy;
    ber_data.bv_len = value.size();

    LDAPMod attr;
    struct berval *values[] = {&ber_data, NULL};
    attr.mod_op = LDAP_MOD_DELETE | LDAP_MOD_BVALUES;
    attr.mod_type = (char *)cstr(attribute);
    attr.mod_bvalues = values;
    
    LDAPMod *attrs[] = {&attr, NULL};

    const int result = ldap_modify_ext_s(d->ld, cstr(dn), attrs, NULL, NULL);
    free(data_copy);

    if (result == LDAP_SUCCESS) {
        const QString context = QString(tr("Deleted value \"%1\" for attribute \"%2\" of object \"%3\"")).arg(value_display, attribute, name);

        d->success_message(context, do_msg);

        return true;
    } else {
        const QString context = QString(tr("Failed to delete value \"%1\" for attribute \"%2\" of object \"%3\"")).arg(value_display, attribute, name);

        d->error_message(context, d->default_error(), do_msg);

        return false;
    }
}

bool AdInterface::attribute_replace_string(const QString &dn, const QString &attribute, const QString &value, const DoStatusMsg do_msg) {
    const QByteArray value_bytes = value.toUtf8();

    return attribute_replace_value(dn, attribute, value_bytes, do_msg);
}

bool AdInterface::attribute_replace_int(const QString &dn, const QString &attribute, const int value, const DoStatusMsg do_msg) {
    const QString value_string = QString::number(value);
    const bool result = attribute_replace_string(dn, attribute, value_string, do_msg);

    return result;
}

bool AdInterface::attribute_replace_datetime(const QString &dn, const QString &attribute, const QDateTime &datetime) {
    const QString datetime_string = datetime_qdatetime_to_string(attribute, datetime, d->adconfig);
    const bool result = attribute_replace_string(dn, attribute, datetime_string);

    return result;
}

bool AdInterface::attribute_replace_security_descriptor(const QString &dn, const QHash<QByteArray, QHash<AcePermission, PermissionState>> &descriptor_state_arg) {
    const QByteArray new_descriptor_bytes =
    [&]() {
        // Remove redundancy from permission state
        const QHash<QByteArray, QHash<AcePermission, PermissionState>> state =
        [&]() {
            QHash<QByteArray, QHash<AcePermission, PermissionState>> out;

            out = descriptor_state_arg;

            // Remove child permission states. For example if
            // "Read" is allowed, then there's no need to
            // include any other state for "read prop"
            // permissions.
            for (const QByteArray &trustee : out.keys()) {
                const bool full_control = out[trustee].contains(AcePermission_FullControl) && (out[trustee][AcePermission_FullControl] != PermissionState_None);
                const bool read = out[trustee].contains(AcePermission_Read) && (out[trustee][AcePermission_Read] != PermissionState_None);
                const bool write = out[trustee].contains(AcePermission_Write) && (out[trustee][AcePermission_Write] != PermissionState_None);

                if (full_control) {
                    for (const AcePermission &permission : all_permissions) {
                        if (permission != AcePermission_FullControl) {
                            out[trustee].remove(permission);
                        }
                    }
                } else if (read) {
                    for (const AcePermission &permission : read_prop_permissions) {
                        if (permission != AcePermission_Read) {
                            out[trustee].remove(permission);
                        }
                    }
                } else if (write) {
                    for (const AcePermission &permission : write_prop_permissions) {
                        if (permission != AcePermission_Read) {
                            out[trustee].remove(permission);
                        }
                    }
                }
            }

            return out;
        }();

        TALLOC_CTX* tmp_ctx = talloc_new(NULL);

        struct security_descriptor *sd = talloc(tmp_ctx, struct security_descriptor);

        // Copy everything that is unchanged from old sd
        const AdObject object = search_object(dn, {ATTRIBUTE_SECURITY_DESCRIPTOR});
        const QByteArray old_descriptor_bytes = object.get_value(ATTRIBUTE_SECURITY_DESCRIPTOR);
        const SecurityDescriptor old_sd = SecurityDescriptor(old_descriptor_bytes);
        struct security_descriptor *original_sd = old_sd.data;
        sd->revision = original_sd->revision;
        sd->type = original_sd->type;
        sd->owner_sid = original_sd->owner_sid;
        sd->group_sid = original_sd->group_sid;
        sd->sacl = original_sd->sacl;

        // Generate new dacl
        const QList<security_ace *> dacl_qlist =
        [&]() {
            QList<security_ace *> out;

            for (const QByteArray &trustee : state.keys()) {
                const QHash<AcePermission, PermissionState> permission_map = state[trustee];

                for (const AcePermission &permission : permission_map.keys()) {
                    const PermissionState permission_state = permission_map[permission];

                    if (permission_state == PermissionState_None) {
                        continue;
                    }

                    struct security_ace *ace = talloc(tmp_ctx, struct security_ace);

                    const bool object_present = ace_permission_to_type_map.contains(permission);

                    ace->type =
                    [&]() {
                        if (permission_state == PermissionState_Allowed) {
                            if (object_present) {
                                return SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT;
                            } else {
                                return SEC_ACE_TYPE_ACCESS_ALLOWED;
                            }
                        } else if (permission_state == PermissionState_Denied) {
                            if (object_present) {
                                return SEC_ACE_TYPE_ACCESS_DENIED_OBJECT;
                            } else {
                                return SEC_ACE_TYPE_ACCESS_DENIED;
                            }
                        }

                        return SEC_ACE_TYPE_ACCESS_ALLOWED;
                    }();

                    // TODO: these flags should be set to something
                    // in some cases, for now just 0
                    ace->flags = 0x00;
                    ace->access_mask = ace_permission_to_mask_map[permission];
                    ace->object.object.flags =
                    [&]() {
                        if (object_present) {
                            return SEC_ACE_OBJECT_TYPE_PRESENT;
                        } else {
                            return 0;
                        }
                    }();
                    ace->object.object.type.type =
                    [&]() {
                        if (object_present) {
                            const QString type_name_string = ace_permission_to_type_map[permission];
                            const QString type_string = d->adconfig->get_right_guid(type_name_string);
                            const QByteArray type_bytes = guid_string_to_bytes(type_string);

                            struct GUID guid;
                            memcpy(&guid, type_bytes.data(), sizeof(GUID));

                            return guid;
                        } else {
                            return GUID();
                        }
                    }();
                    memcpy(&ace->trustee, trustee.data(), sizeof(dom_sid));

                    out.append(ace);
                }
            }

            return out;
        }();

        sd->dacl =
        [&]() {
            struct security_acl *out = talloc(tmp_ctx, struct security_acl);
            out->revision = original_sd->dacl->revision;

            out->num_aces = dacl_qlist.size();

            out->aces = talloc_array(tmp_ctx, security_ace, dacl_qlist.size());
            for (int i = 0; i < dacl_qlist.size(); i++) {
                memcpy(&out->aces[i], dacl_qlist[i], sizeof(security_ace));
            }

            return out;
        }();

        DATA_BLOB blob;
        ndr_push_struct_blob(&blob, tmp_ctx, sd, (ndr_push_flags_fn_t)ndr_push_security_descriptor);

        const QByteArray out = QByteArray((char *) blob.data, blob.length);

        talloc_free(tmp_ctx);

        return out;
    }();

    const bool apply_success = attribute_replace_value(dn, ATTRIBUTE_SECURITY_DESCRIPTOR, new_descriptor_bytes);

    return apply_success;
}

bool AdInterface::object_add(const QString &dn, const QString &object_class) {
    const char *classes[2] = {cstr(object_class), NULL};

    LDAPMod attr;
    attr.mod_op = LDAP_MOD_ADD;
    attr.mod_type = (char *) "objectClass";
    attr.mod_values = (char **) classes;

    LDAPMod *attrs[] = {&attr, NULL};

    const int result = ldap_add_ext_s(d->ld, cstr(dn), attrs, NULL, NULL);

    if (result == LDAP_SUCCESS) {
        d->success_message(QString(tr("Created object \"%1\"")).arg(dn));

        return true;
    } else {
        const QString context = QString(tr("Failed to create object \"%1\"")).arg(dn);

        d->error_message(context, d->default_error());

        return false;
    }
}

bool AdInterface::object_delete(const QString &dn) {
    int result;
    LDAPControl *tree_delete_control = NULL;

    auto cleanup =
    [tree_delete_control]() {
        ldap_control_free(tree_delete_control);
    };

    const QString name = dn_get_name(dn);

    auto error_message =
    [this, name](const QString &error) {
        const QString context = QString(tr("Failed to delete object \"%1\"")).arg(name);

        d->error_message(context, error);
    };

    // Use a tree delete control to enable recursive delete
    tree_delete_control = (LDAPControl *) malloc(sizeof(LDAPControl));
    if (tree_delete_control == NULL) {
        error_message(tr("LDAP Operation error - Failed to allocate tree delete control"));
        cleanup();

        return false;
    }

    result = ldap_control_create(LDAP_CONTROL_X_TREE_DELETE, 1, NULL, 0, &tree_delete_control);
    if (result != LDAP_SUCCESS) {
        error_message(tr("LDAP Operation error - Failed to create tree delete control"));
        cleanup();

        return false;
    }

    LDAPControl *server_controls[2] = {tree_delete_control, NULL};
    
    result = ldap_delete_ext_s(d->ld, cstr(dn), server_controls, NULL);

    cleanup();

    if (result == LDAP_SUCCESS) {
        d->success_message(QString(tr("Deleted object \"%1\"")).arg(name));

        return true;
    } else {
        error_message(d->default_error());

        return false;
    }
}

bool AdInterface::object_move(const QString &dn, const QString &new_container) {
    const QString rdn = dn.split(',')[0];
    const QString new_dn = rdn + "," + new_container;
    const QString object_name = dn_get_name(dn);
    const QString container_name = dn_get_name(new_container);

    const int result = ldap_rename_s(d->ld, cstr(dn), cstr(rdn), cstr(new_container), 1, NULL, NULL);

    if (result == LDAP_SUCCESS) {
        d->success_message(QString(tr("Moved object \"%1\" to \"%2\"")).arg(object_name, container_name));

        return true;
    } else {
        const QString context = QString(tr("Failed to move object \"%1\" to \"%2\"")).arg(object_name, container_name);

        d->error_message(context, d->default_error());

        return false;
    }
}

bool AdInterface::object_rename(const QString &dn, const QString &new_name) {
    const QString new_dn = dn_rename(dn, new_name);
    const QString new_rdn = new_dn.split(",")[0];
    const QString old_name = dn_get_name(dn);

    const int result = ldap_rename_s(d->ld, cstr(dn), cstr(new_rdn), NULL, 1, NULL, NULL);

    if (result == LDAP_SUCCESS) {
        d->success_message(QString(tr("Renamed object \"%1\" to \"%2\"")).arg(old_name, new_name));

        return true;
    } else {
        const QString context = QString(tr("Failed to rename object \"%1\" to \"%2\"")).arg(old_name, new_name);

        d->error_message(context, d->default_error());

        return false;
    }
}

bool AdInterface::group_add_member(const QString &group_dn, const QString &user_dn) {
    const QByteArray user_dn_bytes = user_dn.toUtf8();
    const bool success = attribute_add_value(group_dn, ATTRIBUTE_MEMBER, user_dn_bytes, DoStatusMsg_No);

    const QString user_name = dn_get_name(user_dn);
    const QString group_name = dn_get_name(group_dn);
    
    if (success) {
        d->success_message(QString(tr("Added user \"%1\" to group \"%2\"")).arg(user_name, group_name));

        return true;
    } else {
        const QString context = QString(tr("Failed to add user \"%1\" to group \"%2\"")).arg(user_name, group_name);

        d->error_message(context, d->default_error());

        return false;
    }
}

bool AdInterface::group_remove_member(const QString &group_dn, const QString &user_dn) {
    const QByteArray user_dn_bytes = user_dn.toUtf8();
    const bool success = attribute_delete_value(group_dn, ATTRIBUTE_MEMBER, user_dn_bytes, DoStatusMsg_No);

    const QString user_name = dn_get_name(user_dn);
    const QString group_name = dn_get_name(group_dn);

    if (success) {
        d->success_message(QString(tr("Removed user \"%1\" from group \"%2\"")).arg(user_name, group_name));

        return true;
    } else {
        const QString context = QString(tr("Failed to remove user \"%1\" from group \"%2\"")).arg(user_name, group_name);

        d->error_message(context, d->default_error());

        return false;
    }
}

bool AdInterface::group_set_scope(const QString &dn, GroupScope scope, const DoStatusMsg do_msg) {
    // NOTE: it is not possible to change scope from
    // global<->domainlocal directly, so have to switch to
    // universal first.
    const bool need_to_switch_to_universal =
    [=]() {
        const AdObject object = search_object(dn, {ATTRIBUTE_GROUP_TYPE});
        const GroupScope current_scope = object.get_group_scope();

        return (current_scope == GroupScope_Global && scope == GroupScope_DomainLocal) || (current_scope == GroupScope_DomainLocal && scope == GroupScope_Global) ;
    }();

    if (need_to_switch_to_universal) {
        group_set_scope(dn, GroupScope_Universal, DoStatusMsg_No);
    }

    const AdObject object = search_object(dn, {ATTRIBUTE_GROUP_TYPE});
    int group_type = object.get_int(ATTRIBUTE_GROUP_TYPE);

    // Unset all scope bits, because scope bits are exclusive
    for (int i = 0; i < GroupScope_COUNT; i++) {
        const GroupScope this_scope = (GroupScope) i;
        const int this_scope_bit = group_scope_bit(this_scope);

        group_type = bit_set(group_type, this_scope_bit, false);
    }

    // Set given scope bit
    const int scope_bit = group_scope_bit(scope);
    group_type = bit_set(group_type, scope_bit, true);

    const QString name = dn_get_name(dn);
    const QString scope_string = group_scope_string(scope);
    
    const bool result = attribute_replace_int(dn, ATTRIBUTE_GROUP_TYPE, group_type);
    if (result) {
        d->success_message(QString(tr("Set scope for group \"%1\" to \"%2\"")).arg(name, scope_string), do_msg);

        return true;
    } else {
        const QString context = QString(tr("Failed to set scope for group \"%1\" to \"%2\"")).arg(name, scope_string);
        
        d->error_message(context, d->default_error(), do_msg);

        return false;
    }
}

bool AdInterface::group_set_type(const QString &dn, GroupType type) {
    const AdObject object = search_object(dn, {ATTRIBUTE_GROUP_TYPE});
    const int group_type = object.get_int(ATTRIBUTE_GROUP_TYPE);

    const bool set_security_bit = type == GroupType_Security;

    const int update_group_type = bit_set(group_type, GROUP_TYPE_BIT_SECURITY, set_security_bit);
    const QString update_group_type_string = QString::number(update_group_type);

    const QString name = dn_get_name(dn);
    const QString type_string = group_type_string(type);
    
    const bool result = attribute_replace_string(dn, ATTRIBUTE_GROUP_TYPE, update_group_type_string);
    if (result) {
        d->success_message(QString(tr("Set type for group \"%1\" to \"%2\"")).arg(name, type_string));

        return true;
    } else {
        const QString context = QString(tr("Failed to set type for group \"%1\" to \"%2\"")).arg(name, type_string);
        d->error_message(context, d->default_error());

        return false;
    }
}


bool AdInterface::user_set_primary_group(const QString &group_dn, const QString &user_dn) {
    const AdObject group_object = search_object(group_dn, {ATTRIBUTE_OBJECT_SID, ATTRIBUTE_MEMBER});

    // NOTE: need to add user to group before it can become primary
    const QList<QString> group_members = group_object.get_strings(ATTRIBUTE_MEMBER);
    const bool user_is_in_group = group_members.contains(user_dn);
    if (!user_is_in_group) {
        group_add_member(group_dn, user_dn);
    }

    const QByteArray group_sid = group_object.get_value(ATTRIBUTE_OBJECT_SID);
    const QString group_rid = extract_rid_from_sid(group_sid, d->adconfig);

    const bool success = attribute_replace_string(user_dn, ATTRIBUTE_PRIMARY_GROUP_ID, group_rid, DoStatusMsg_No);

    const QString user_name = dn_get_name(user_dn);
    const QString group_name = dn_get_name(group_dn);

    if (success) {
        d->success_message(QString(tr("Set primary group for user \"%1\" to \"%2\"")).arg(user_name, group_name));

        return true;
    } else {
        const QString context = QString(tr("Failed to set primary group for user \"%1\" to \"%2\"")).arg(user_name, group_name);

        d->error_message(context, d->default_error());

        return false;
    }
}

bool AdInterface::user_set_pass(const QString &dn, const QString &password) {
    // NOTE: AD requires that the password:
    // 1. is surrounded by quotes
    // 2. is encoded as UTF16-LE
    // 3. has no Byte Order Mark
    const QString quoted_password = QString("\"%1\"").arg(password);
    const auto codec = QTextCodec::codecForName("UTF-16LE");
    QByteArray password_bytes = codec->fromUnicode(quoted_password);
    // Remove BOM
    // NOTE: gotta be a way to tell codec not to add BOM
    // but couldn't find it, only QTextStream has
    // setGenerateBOM()
    if (password_bytes[0] != '\"') {
        password_bytes.remove(0, 2);
    }

    const bool success = attribute_replace_value(dn, ATTRIBUTE_PASSWORD, password_bytes, DoStatusMsg_No);

    const QString name = dn_get_name(dn);
    
    if (success) {
        d->success_message(QString(tr("Set password for user \"%1\"")).arg(name));

        return true;
    } else {
        const QString context = QString(tr("Failed to set password for user \"%1\"")).arg(name);

        const QString error =
        [this]() {
            const int ldap_result = d->get_ldap_result();
            if (ldap_result == LDAP_CONSTRAINT_VIOLATION) {
                return tr("Password doesn't match rules");
            } else {
                return d->default_error();
            }
        }();

        d->error_message(context, error);

        return false;
    }
}

// TODO:
// "User cannot change password" - CAN'T just set PASSWD_CANT_CHANGE. See: https://docs.microsoft.com/en-us/windows/win32/adsi/modifying-user-cannot-change-password-ldap-provider?redirectedfrom=MSDN
// "This account supports 128bit encryption" (and for 256bit)
// "Use Kerberos DES encryption types for this account"
bool AdInterface::user_set_account_option(const QString &dn, AccountOption option, bool set) {
    if (dn.isEmpty()) {
        return false;
    }

    bool success = false;

    switch (option) {
        case AccountOption_PasswordExpired: {
            QString pwdLastSet_value;
            if (set) {
                pwdLastSet_value = AD_PWD_LAST_SET_EXPIRED;
            } else {
                pwdLastSet_value = AD_PWD_LAST_SET_RESET;
            }

            success = attribute_replace_string(dn, ATTRIBUTE_PWD_LAST_SET, pwdLastSet_value, DoStatusMsg_No);

            break;
        }
        default: {
            const int uac =
            [this, dn]() {
                const AdObject object = search_object(dn, {ATTRIBUTE_USER_ACCOUNT_CONTROL});
                return object.get_int(ATTRIBUTE_USER_ACCOUNT_CONTROL);
            }();

            const int bit = account_option_bit(option);
            const int updated_uac = bit_set(uac, bit, set);

            success = attribute_replace_int(dn, ATTRIBUTE_USER_ACCOUNT_CONTROL, updated_uac, DoStatusMsg_No);
        }
    }

    const QString name = dn_get_name(dn);
    
    if (success) {
        const QString success_context =
        [option, set, name]() {
            switch (option) {
                case AccountOption_Disabled: {
                    if (set) {
                        return QString(tr("Disabled account for user - \"%1\"")).arg(name);
                    } else {
                        return QString(tr("Enabled account for user - \"%1\"")).arg(name);
                    }
                }
                default: {
                    const QString description = account_option_string(option);

                    if (set) {
                        return QString(tr("Turned ON account option \"%1\" for user \"%2\"")).arg(description, name);
                    } else {
                        return QString(tr("Turned OFF account option \"%1\" for user \"%2\"")).arg(description, name);
                    }
                }
            }
        }();
        
        d->success_message(success_context);

        return true;
    } else {
        const QString context =
        [option, set, name]() {
            switch (option) {
                case AccountOption_Disabled: {
                    if (set) {
                        return QString(tr("Failed to disable account for user \"%1\"")).arg(name);
                    } else {
                        return QString(tr("Failed to enable account for user \"%1\"")).arg(name);
                    }
                }
                default: {
                    const QString description = account_option_string(option);

                    if (set) {
                        return QString(tr("Failed to turn ON account option \"%1\" for user \"%2\"")).arg(description, name);
                    } else {
                        return QString(tr("Failed to turn OFF account option \"%1\" for user \"%2\"")).arg(description, name);
                    }
                }
            }
        }();

        d->error_message(context, d->default_error());

        return false;
    }
}

bool AdInterface::user_unlock(const QString &dn) {
    const bool result = attribute_replace_string(dn, ATTRIBUTE_LOCKOUT_TIME, LOCKOUT_UNLOCKED_VALUE);

    const QString name = dn_get_name(dn);
    
    if (result) {
        d->success_message(QString(tr("Unlocked user \"%1\"")).arg(name));
        
        return true;
    } else {
        const QString context = QString(tr("Failed to unlock user \"%1\"")).arg(name);

        d->error_message(context, d->default_error());

        return result;
    }
}

bool AdInterface::create_gpo(const QString &display_name, QString &dn_out) {
    //
    // Generate UUID used for directory and object names
    //
    // TODO: make sure endianess works fine on processors
    // with weird endianess.
    const QString uuid =
    [](){
        uuid_t uuid_struct;
        uuid_generate_random(uuid_struct);

        char uuid_cstr[UUID_STR_LEN];
        uuid_unparse_upper(uuid_struct, uuid_cstr);

        const QString out = "{" + QString(uuid_cstr) + "}";

        return out;
    }();

    //
    // Create dirs and files for policy on sysvol
    //

    // Create main dir
    // "smb://domain.alt/sysvol/domain.alt/Policies/{FF7E0880-F3AD-4540-8F1D-4472CB4A7044}"
    const QString main_dir = QString("smb://%1/sysvol/%2/Policies/%3").arg(AdInterfacePrivate::s_dc, d->domain.toLower(), uuid);
    const int result_mkdir_main = smbc_mkdir(cstr(main_dir), 0);
    if (result_mkdir_main != 0) {
        qDebug() << "Failed to create policy main dir";
        return false;
    }

    const QString machine_dir = main_dir + "/Machine";
    const int result_mkdir_machine = smbc_mkdir(cstr(machine_dir), 0);
    if (result_mkdir_machine != 0) {
        qDebug() << "Failed to create policy machine dir";
        return false;
    }

    const QString user_dir = main_dir + "/User";
    const int result_mkdir_user = smbc_mkdir(cstr(user_dir), 0);
    if (result_mkdir_user != 0) {
        qDebug() << "Failed to create policy user dir";
        return false;
    }

    const QString init_file_path = main_dir + "/GPT.INI";
    const int ini_file = smbc_open(cstr(init_file_path), O_WRONLY | O_CREAT, 0);

    const char *ini_contents = "[General]\r\nVersion=0\r\n";
    const int result_write_ini = smbc_write(ini_file, ini_contents, strlen(ini_contents));
    if (result_write_ini < 0) {
        qDebug() << "Failed to write policy ini";
        return false;
    }

    //
    // Create AD object for gpo
    //
    const QString dn = QString("CN=%1,CN=Policies,CN=System,%2").arg(uuid, d->domain_head);
    dn_out = dn;
    const bool result_add = object_add(dn, CLASS_GP_CONTAINER);
    if (!result_add) {
        qDebug() << "Failed to create gpo";
        return false;
    }
    attribute_replace_string(dn, ATTRIBUTE_DISPLAY_NAME, display_name);
    // "\\domain.alt\sysvol\domain.alt\Policies\{FF7E0880-F3AD-4540-8F1D-4472CB4A7044}"
    const QString gPCFileSysPath = QString("\\\\%1\\sysvol\\%2\\Policies\\%3").arg(d->domain.toLower(), uuid);
    attribute_replace_string(dn, ATTRIBUTE_GPC_FILE_SYS_PATH, gPCFileSysPath);
    // TODO: samba defaults to 1, ADUC defaults to 0. Figure out what's this supposed to be.
    attribute_replace_string(dn, ATTRIBUTE_FLAGS, "1");
    attribute_replace_string(dn, ATTRIBUTE_VERSION_NUMBER, "0");
    attribute_replace_string(dn, ATTRIBUTE_SHOW_IN_ADVANCED_VIEW_ONLY, "TRUE");
    attribute_replace_string(dn, ATTRIBUTE_GPC_FUNCTIONALITY_VERSION, "2");

    // User object
    const QString user_dn = "CN=User," + dn;
    const bool result_add_user = object_add(user_dn, CLASS_CONTAINER);
    attribute_replace_string(dn, ATTRIBUTE_SHOW_IN_ADVANCED_VIEW_ONLY, "TRUE");
    if (!result_add_user) {
        qDebug() << "Failed to create gpo user";
        return false;
    }

    // Machine object
    const QString machine_dn = "CN=Machine," + dn;
    const bool result_add_machine = object_add(machine_dn, CLASS_CONTAINER);
    attribute_replace_string(dn, ATTRIBUTE_SHOW_IN_ADVANCED_VIEW_ONLY, "TRUE");
    if (!result_add_machine) {
        qDebug() << "Failed to create gpo machine";
        return false;
    }

    //
    // Set security descriptor for sysvol dir
    //

    // First get descriptor of the GPO
    const QString base = dn;
    const SearchScope scope = SearchScope_Object;
    const QString filter = QString();
    const QList<QString> attributes = {ATTRIBUTE_SECURITY_DESCRIPTOR};
    const QHash<QString, AdObject> results = search(base, scope, filter, attributes);
    
    const AdObject object = results[dn];
    const QByteArray descriptor_bytes = object.get_value(ATTRIBUTE_SECURITY_DESCRIPTOR);

    // Transform descriptor bytes into descriptor struct
    TALLOC_CTX *tmp_ctx = talloc_new(NULL);
    
    DATA_BLOB blob;
    blob.data = (uint8_t *)descriptor_bytes.data();
    blob.length = descriptor_bytes.size();

    struct ndr_pull *ndr_pull = ndr_pull_init_blob(&blob, tmp_ctx);

    struct security_descriptor domain_sd;
    ndr_pull_security_descriptor(ndr_pull, NDR_SCALARS|NDR_BUFFERS, &domain_sd);

    // TODO: not sure why but my
    // gp_create_gpt_security_descriptor() call creates an
    // sd that has 1 extra ace than samba's version
    // (ACL:S-1-5-11:5/3/0x00000000)
    // sid = S-1-5-11 = SID_NT_AUTHENTICATED_USERS
    // type = 5 = SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT

    // Create sysvol descriptor from domain descriptor (not
    // one to one, some modifications are needed)
    struct security_descriptor *sysvol_sd;
    const bool create_sd_success = gp_create_gpt_security_descriptor(tmp_ctx, &domain_sd, &sysvol_sd);
    if (!create_sd_success) {
        qDebug() << "Failed to create gpo sd";
        return false;
    }

    const QString sysvol_sd_string =
    [tmp_ctx, &sysvol_sd]() {
        QString out;

        out += QString("REVISION:%1,OWNER:%2,GROUP:%3,").arg(QString::number(sysvol_sd->revision), dom_sid_string(tmp_ctx, sysvol_sd->owner_sid), dom_sid_string(tmp_ctx, sysvol_sd->group_sid));

        // NOTE: don't need sacl

        for (uint32_t i = 0; i < sysvol_sd->dacl->num_aces; i++) {
            struct security_ace ace = sysvol_sd->dacl->aces[i];

            char access_mask_buffer[100];
            snprintf(access_mask_buffer, sizeof(access_mask_buffer), "0x%08x", ace.access_mask);

            if (i > 0) {
                out += ",";
            }

            out += QString("ACL:%1:%2/%3/%4").arg(dom_sid_string(tmp_ctx, &ace.trustee), QString::number(ace.type), QString::number(ace.flags), access_mask_buffer);
        }

        return out;
    }();
    const QByteArray sysvol_sd_string_bytes = sysvol_sd_string.toUtf8();
    const char *sysvol_sd_cstr = sysvol_sd_string_bytes.constData();

    // Set descriptor
    const int set_sd_result = smbc_setxattr(cstr(main_dir), "system.nt_sec_desc.*", sysvol_sd_cstr, sysvol_sd_string_bytes.size(), 0);
    if (set_sd_result != 0) {
        qDebug() << "Failed to set gpo sd. Error:" << strerror(errno);
        return false;
    }

    talloc_free(tmp_ctx);

    return true;
}

bool AdInterface::delete_gpo(const QString &gpo_dn) {
    const bool delete_success = object_delete(gpo_dn);
    if (!delete_success) {
        qDebug() << "Failed to delete gpo object";
        return false;
    }

    const AdObject object = search_object(gpo_dn, {ATTRIBUTE_GPC_FILE_SYS_PATH});
    const QString sysvol_path = object.get_string(ATTRIBUTE_GPC_FILE_SYS_PATH);
    const QString url = sysvol_path_to_smb(sysvol_path);
    const int result_rmdir = smbc_rmdir(cstr(url));
    if (result_rmdir < 0) {
        qDebug() << "Failed to remove gpo dir";
        return false;
    }

    return true;
}

QString AdInterface::sysvol_path_to_smb(const QString &sysvol_path) const {
    QString out = sysvol_path;
    
    out.replace("\\", "/");

    // TODO: file sys path as it is, is like this:
    // "smb://domain.alt/sysvol/domain.alt/Policies/{D7E75BC7-138D-4EE1-8974-105E4A2DE560}"
    // But that fails to load the whole directory sometimes
    // Replacing domain at the start with current host fixes it
    // "smb://dc0.domain.alt/sysvol/domain.alt/Policies/{D7E75BC7-138D-4EE1-8974-105E4A2DE560}"
    // not sure if this is required and which host/DC is the correct one
    const int sysvol_i = out.indexOf("sysvol");
    out.remove(0, sysvol_i);

    out = QString("smb://%1/%2").arg(AdInterfacePrivate::s_dc, out);

    return out;
}

void AdInterfacePrivate::success_message(const QString &msg, const DoStatusMsg do_msg) {
    if (do_msg == DoStatusMsg_No) {
        return;
    }

    const AdMessage message(msg, AdMessageType_Success);
    messages.append(message);
}

void AdInterfacePrivate::error_message(const QString &context, const QString &error, const DoStatusMsg do_msg) {
    if (do_msg == DoStatusMsg_No) {
        return;
    }

    QString msg = context;
    if (!error.isEmpty()) {
        msg += QString(tr(". Error: \"%1\"")).arg(error);;
    }

    const AdMessage message(msg, AdMessageType_Error);
    messages.append(message);
}

QString AdInterfacePrivate::default_error() const {
    const int ldap_result = get_ldap_result();
    switch (ldap_result) {
        case LDAP_NO_SUCH_OBJECT: return tr("No such object");
        case LDAP_CONSTRAINT_VIOLATION: return tr("Constraint violation");
        case LDAP_UNWILLING_TO_PERFORM: return tr("Server is unwilling to perform");
        case LDAP_ALREADY_EXISTS: return tr("Already exists");
        default: {
            char *ldap_err = ldap_err2string(ldap_result);
            const QString ldap_err_qstr(ldap_err);
            return QString(tr("Server error: %1")).arg(ldap_err_qstr);
        }
    }
}

int AdInterfacePrivate::get_ldap_result() const {
    int result;
    ldap_get_option(ld, LDAP_OPT_RESULT_CODE, &result);

    return result;
}

QList<QString> get_domain_hosts(const QString &domain, const QString &site) {
    QList<QString> hosts;

    // TODO: confirm site query is formatted properly, currently getting no answer back (might be working as intended, since tested on domain without sites?)

    // Query site hosts
    if (!site.isEmpty()) {
        char dname[1000];
        snprintf(dname, sizeof(dname), "_ldap._tcp.%s._sites.%s", cstr(site), cstr(domain));

        const QList<QString> site_hosts = query_server_for_hosts(dname);
        hosts.append(site_hosts);
    }

    // Query default hosts
    char dname_default[1000];
    snprintf(dname_default, sizeof(dname_default), "_ldap._tcp.%s", cstr(domain));

    const QList<QString> default_hosts = query_server_for_hosts(dname_default);
    hosts.append(default_hosts);

    hosts.removeDuplicates();

    return hosts;
}

/**
 * Perform a query for dname and output hosts
 * dname is a combination of protocols (d->ldap, tcp), domain and site
 * NOTE: this is rewritten from
 * https://github.com/paleg/libadclient/blob/master/adclient.cpp
 * which itself is copied from
 * https://www.ccnx.org/releases/latest/doc/ccode/html/ccndc-srv_8c_source.html
 * Another example of similar procedure:
 * https://www.gnu.org/software/shishi/coverage/shishi/lib/resolv.c.gcov.html
 */
QList<QString> query_server_for_hosts(const char *dname) {
    union dns_msg {
        HEADER header;
        unsigned char buf[NS_MAXMSG];
    } msg;

    auto error =
    []() {
        return QList<QString>();
    };

    const long unsigned msg_len = res_search(dname, ns_c_in, ns_t_srv, msg.buf, sizeof(msg.buf));

    const bool message_error = (msg_len < 0 || msg_len < sizeof(HEADER));
    if (message_error) {
        error();
    }

    const int packet_count = ntohs(msg.header.qdcount);
    const int answer_count = ntohs(msg.header.ancount);

    unsigned char *curr = msg.buf + sizeof(msg.header);
    const unsigned char *eom = msg.buf + msg_len;

    // Skip over packet records
    for (int i = packet_count; i > 0 && curr < eom; i--) {
        const int packet_len = dn_skipname(curr, eom);

        const bool packet_error = (packet_len < 0);
        if (packet_error) {
            error();
        }

        curr = curr + packet_len + QFIXEDSZ;
    }

    QList<QString> hosts;

    // Process answers by collecting hosts into list
    for (int i = 0; i < answer_count; i++) {
        // Get server
        char server[NS_MAXDNAME];
        const int server_len = dn_expand(msg.buf, eom, curr, server, sizeof(server));
        
        const bool server_error = (server_len < 0);
        if (server_error) {
            error();
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
            error();
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

        // Get host
        char host[NS_MAXDNAME];
        const int host_len = dn_expand(msg.buf, eom, curr, host, sizeof(host));
        const bool host_error = (host_len < 0);
        if (host_error) {
            error();
        }

        hosts.append(QString(host));

        curr = record_end;
    }

    return hosts;
}

bool ad_connect(const char* uri, LDAP **ld_out) {
    int result;
    LDAP *ld = NULL;

    auto cleanup =
    [ld]() {
        ldap_memfree(ld);
    };

    // NOTE: this doesn't leak memory. False positive.
    result = ldap_initialize(&ld, uri);
    if (result != LDAP_SUCCESS) {
        cleanup();
        return false;
    }

    // Set version
    const int version = LDAP_VERSION3;
    result = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &version);
    if (result != LDAP_OPT_SUCCESS) {
        cleanup();
        return false;
    }

    // Disable referrals
    result =ldap_set_option(ld, LDAP_OPT_REFERRALS, LDAP_OPT_OFF);
    if (result != LDAP_OPT_SUCCESS) {
        cleanup();
        return false;
    }

    // Set maxssf
    const char* sasl_secprops = "maxssf=56";
    result = ldap_set_option(ld, LDAP_OPT_X_SASL_SECPROPS, sasl_secprops);
    if (result != LDAP_SUCCESS) {
        cleanup();
        return false;
    }

    result = ldap_set_option(ld, LDAP_OPT_X_SASL_NOCANON, LDAP_OPT_ON);
    if (result != LDAP_SUCCESS) {
        cleanup();
        return false;
    }

    // TODO: add option to turn off
    ldap_set_option(ld, LDAP_OPT_X_TLS_REQUIRE_CERT, LDAP_OPT_X_TLS_NEVER);
    if (result != LDAP_SUCCESS) {
        cleanup();
        return false;
    }

    // Setup sasl_defaults_gssapi 
    struct sasl_defaults_gssapi defaults;
    defaults.mech = (char *)"GSSAPI";
    ldap_get_option(ld, LDAP_OPT_X_SASL_REALM, &defaults.realm);
    ldap_get_option(ld, LDAP_OPT_X_SASL_AUTHCID, &defaults.authcid);
    ldap_get_option(ld, LDAP_OPT_X_SASL_AUTHZID, &defaults.authzid);
    defaults.passwd = NULL;

    // Perform bind operation
    unsigned sasl_flags = LDAP_SASL_QUIET;
    result = ldap_sasl_interactive_bind_s(ld, NULL, defaults.mech, NULL, NULL, sasl_flags, sasl_interact_gssapi, &defaults);
    ldap_memfree(defaults.realm);
    ldap_memfree(defaults.authcid);
    ldap_memfree(defaults.authzid);
    if (result != LDAP_SUCCESS) {
        cleanup();
        return false;
    }

    if (ld_out != NULL) {
        *ld_out = ld;
    }

    return true;
}

/**
 * Callback for ldap_sasl_interactive_bind_s
 */
int sasl_interact_gssapi(LDAP *ld, unsigned flags, void *indefaults, void *in) {
    sasl_defaults_gssapi *defaults = (sasl_defaults_gssapi *) indefaults;
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
        interact->len = strlen((const char *) interact->result);
        interact++;
    }

    return LDAP_SUCCESS;
}

AdCookie::AdCookie() {
    cookie = NULL;
}

bool AdCookie::more_pages() const {
    return (cookie != NULL);
}

AdCookie::~AdCookie() {
    ber_bvfree(cookie);
}

AdMessage::AdMessage(const QString &text, const AdMessageType &type) {
    m_text = text;
    m_type = type;
}

QString AdMessage::text() const {
    return m_text;
}

AdMessageType AdMessage::type() const {
    return m_type;
}

#define TRUSTEE_ENUM_TO_STRING(ENUM) {ENUM, #ENUM}

// TODO: write custom trustee names. Not sure if have to be
// translated, trustee object dn's aren't translated so
// maybe names are untranslated as well?
const QHash<QString, QString> trustee_name_map = {
    TRUSTEE_ENUM_TO_STRING(NAME_WORLD),
    TRUSTEE_ENUM_TO_STRING(SID_WORLD_DOMAIN),
    TRUSTEE_ENUM_TO_STRING(SID_WORLD),
    TRUSTEE_ENUM_TO_STRING(SID_CREATOR_OWNER_DOMAIN),
    TRUSTEE_ENUM_TO_STRING(SID_CREATOR_OWNER),
    TRUSTEE_ENUM_TO_STRING(SID_CREATOR_GROUP),
    TRUSTEE_ENUM_TO_STRING(SID_OWNER_RIGHTS),
    TRUSTEE_ENUM_TO_STRING(NAME_NT_AUTHORITY),
    TRUSTEE_ENUM_TO_STRING(SID_NT_AUTHORITY),
    TRUSTEE_ENUM_TO_STRING(SID_NT_DIALUP),
    TRUSTEE_ENUM_TO_STRING(SID_NT_NETWORK),
    TRUSTEE_ENUM_TO_STRING(SID_NT_BATCH),
    TRUSTEE_ENUM_TO_STRING(SID_NT_INTERACTIVE),
    TRUSTEE_ENUM_TO_STRING(SID_NT_SERVICE),
    TRUSTEE_ENUM_TO_STRING(SID_NT_ANONYMOUS),
    TRUSTEE_ENUM_TO_STRING(SID_NT_PROXY),
    TRUSTEE_ENUM_TO_STRING(SID_NT_ENTERPRISE_DCS),
    TRUSTEE_ENUM_TO_STRING(SID_NT_SELF),
    TRUSTEE_ENUM_TO_STRING(SID_NT_AUTHENTICATED_USERS),
    TRUSTEE_ENUM_TO_STRING(SID_NT_RESTRICTED),
    TRUSTEE_ENUM_TO_STRING(SID_NT_TERMINAL_SERVER_USERS),
    TRUSTEE_ENUM_TO_STRING(SID_NT_REMOTE_INTERACTIVE),
    TRUSTEE_ENUM_TO_STRING(SID_NT_THIS_ORGANISATION),
    TRUSTEE_ENUM_TO_STRING(SID_NT_IUSR),
    TRUSTEE_ENUM_TO_STRING(SID_NT_SYSTEM),
    TRUSTEE_ENUM_TO_STRING(SID_NT_LOCAL_SERVICE),
    TRUSTEE_ENUM_TO_STRING(SID_NT_NETWORK_SERVICE),
    TRUSTEE_ENUM_TO_STRING(SID_NT_DIGEST_AUTHENTICATION),
    TRUSTEE_ENUM_TO_STRING(SID_NT_NTLM_AUTHENTICATION),
    TRUSTEE_ENUM_TO_STRING(SID_NT_SCHANNEL_AUTHENTICATION),
    TRUSTEE_ENUM_TO_STRING(SID_NT_OTHER_ORGANISATION),
};

SecurityDescriptor::SecurityDescriptor(const QByteArray &descriptor_bytes) {
    mem_ctx = talloc_new(NULL);

    DATA_BLOB blob = data_blob_const(descriptor_bytes.data(), descriptor_bytes.size());

    data = talloc(mem_ctx, struct security_descriptor);

    ndr_pull_struct_blob(&blob, data, data, (ndr_pull_flags_fn_t)ndr_pull_security_descriptor);
}

SecurityDescriptor::~SecurityDescriptor() {
    talloc_free(mem_ctx);
}

QList<QByteArray> SecurityDescriptor::get_trustee_list() const {
    QSet<QByteArray> out;

    for (security_ace *ace : dacl()) {
        const QByteArray trustee_bytes = dom_sid_to_bytes(ace->trustee);

        out.insert(trustee_bytes);
    }

    return out.toList();
}

QList<security_ace *> SecurityDescriptor::dacl() const {
    QList<security_ace *> out;

    for (uint32_t i = 0; i < data->dacl->num_aces; i++) {
        security_ace *ace = &data->dacl->aces[i];
        out.append(ace);
    }

    return out;
}

QString AdInterface::get_trustee_name(const QByteArray &trustee) {
    const QString trustee_string =
    [&]() {
        dom_sid *sid = (dom_sid *) trustee.data();

        TALLOC_CTX *tmp_ctx = talloc_new(NULL);

        const char *sid_cstr = dom_sid_string(tmp_ctx, sid);
        const QString out = QString(sid_cstr);

        talloc_free(tmp_ctx);

        return out;
    }();

    if (trustee_name_map.contains(trustee_string)) {
        return trustee_name_map[trustee_string];
    } else {
        // Try to get name of trustee by finding it's DN
        const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_SID, trustee_string);
        const auto trustee_search = search(d->domain_head, SearchScope_All, filter, QList<QString>());
        if (!trustee_search.isEmpty()) {
            const QString trustee_dn = trustee_search.keys()[0];
            const QString name = dn_get_name(trustee_dn);

            return name;
        } else {
            // Return raw sid as last option
            return trustee_string;
        }
    }
}

QList<security_ace *> SecurityDescriptor::get_ace_list(const QByteArray &trustee) const {
    QList<security_ace *> out;

    for (security_ace *ace : dacl()) {
        const QByteArray this_trustee_bytes = dom_sid_to_bytes(ace->trustee);

        const bool trustee_match = (this_trustee_bytes == trustee);
        if (trustee_match) {
            out.append(ace);
        }
    }

    return out;
}

void SecurityDescriptor::print_acl(const QByteArray &trustee) const {
    TALLOC_CTX *tmp_ctx = talloc_new(NULL);

    const QList<security_ace *> ace_list = get_ace_list(trustee);

    for (security_ace *ace : ace_list) {
        const QString ace_string =
        [&]() {
            char *ace_cstr = ndr_print_struct_string(tmp_ctx, (ndr_print_fn_t)ndr_print_security_ace,
                "ace", ace);

            const QString out = QString(ace_cstr);

            return out;
        }();

        qDebug().noquote() << ace_string;
    }

    talloc_free(tmp_ctx);
}

QByteArray dom_sid_to_bytes(const dom_sid &sid) {
    const QByteArray bytes = QByteArray((char *) &sid, sizeof(struct dom_sid));

    return bytes;
}
