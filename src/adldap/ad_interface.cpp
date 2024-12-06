/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2022 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
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

#include "ad_config.h"
#include "ad_display.h"
#include "ad_object.h"
#include "ad_security.h"
#include "ad_utils.h"
#include "gplink.h"
#include "samba/dom_sid.h"
#include "samba/gp_manage.h"
#include "samba/libsmb_xattr.h"
#include "samba/ndr_security.h"
#include "samba/security_descriptor.h"

#include "ad_filter.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <krb5.h>
#include <lber.h>
#include <ldap.h>
#include <libsmbclient.h>
#include <resolv.h>
#include <sasl/sasl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <uuid/uuid.h>

#include <QDebug>
#include <QTextCodec>

// NOTE: LDAP library char* inputs are non-const in the API
// but are const for practical purposes so we use forced
// casts (const char *) -> (char *)

#ifdef __GNUC__
#define UNUSED(x) x __attribute__((unused))
#else
#define UNUSED(x) x
#endif

#define UNUSED_ARG(x) (void) (x)

#define MAX_DN_LENGTH 1024
#define MAX_PASSWORD_LENGTH 255
#ifndef UUID_STR_LEN
#define UUID_STR_LEN 37
#endif

typedef struct sasl_defaults_gssapi {
    char *mech;
    char *realm;
    char *authcid;
    char *passwd;
    char *authzid;
} sasl_defaults_gssapi;

enum AceMaskFormat {
    AceMaskFormat_Hexadecimal,
    AceMaskFormat_Decimal,
};

QList<QString> query_server_for_hosts(const char *dname);
int sasl_interact_gssapi(LDAP *ld, unsigned flags, void *indefaults, void *in);
QString get_gpt_sd_string(const AdObject &gpc_object, const AceMaskFormat format);
int create_sd_control(bool get_sacl, int is_critical, LDAPControl **ctrlp, bool set_dacl = false);

AdConfig *AdInterfacePrivate::adconfig = nullptr;
bool AdInterfacePrivate::s_log_searches = false;
QString AdInterfacePrivate::s_dc = QString();
bool AdInterfacePrivate::s_domain_is_default = true;
QString AdInterfacePrivate::s_custom_domain = QString();
void *AdInterfacePrivate::s_sasl_nocanon = LDAP_OPT_ON;
int AdInterfacePrivate::s_port = 0;
CertStrategy AdInterfacePrivate::s_cert_strat = CertStrategy_Never;
SMBCCTX *AdInterfacePrivate::smbc = NULL;
QMutex AdInterfacePrivate::mutex;

void get_auth_data_fn(const char *pServer, const char *pShare, char *pWorkgroup, int maxLenWorkgroup, char *pUsername, int maxLenUsername, char *pPassword, int maxLenPassword) {
    UNUSED_ARG(pServer);
    UNUSED_ARG(pShare);
    UNUSED_ARG(pWorkgroup);
    UNUSED_ARG(maxLenWorkgroup);
    UNUSED_ARG(pUsername);
    UNUSED_ARG(maxLenUsername);
    UNUSED_ARG(pPassword);
    UNUSED_ARG(maxLenPassword);
}

AdInterface::AdInterface() {
    d = new AdInterfacePrivate(this);

    d->is_connected = false;

    d->ld = NULL;

    const QString connect_error_context = tr("Failed to connect.");

    if (AdInterfacePrivate::s_domain_is_default)
        d->domain = get_default_domain_from_krb5();
    else
        d->domain = AdInterfacePrivate::s_custom_domain;

    if (d->domain.isEmpty()) {
        d->error_message(connect_error_context, tr("Failed to get a domain. Check that you have initialized kerberos credentials (kinit)."));
        return;
    }

    //
    // Connect via LDAP
    //

    d->dc = [&]() {
        const QList<QString> dc_list = get_domain_hosts(d->domain, QString());
        if (dc_list.isEmpty()) {
            d->error_message_plain(tr("Failed to find domain controllers. Make sure your computer is in the domain and that domain controllers are operational."));

            return QString();
        }

        if (!AdInterfacePrivate::s_dc.isEmpty()) {
            if (dc_list.contains(AdInterfacePrivate::s_dc)) {
                return AdInterfacePrivate::s_dc;
            } else {
                return dc_list[0];
            }
        } else {
            return dc_list[0];
        }
    }();

    if (AdInterfacePrivate::s_dc.isEmpty()) {
        AdInterfacePrivate::s_dc = d->dc;
    }

    if (!ldap_init()) {
        return;
    }

    // Initialize SMB context

    // NOTE: initialize only once, because otherwise
    // wouldn't be able to have multiple active
    // AdInterface's instances at the same time
    if (!init_smb_context()) {
        return;
    }

    d->is_connected = true;
}

AdInterface::~AdInterface() {
    ldap_free();
    delete d;
}

void AdInterface::set_config(AdConfig *config_arg) {
    AdInterfacePrivate::adconfig = config_arg;
}

void AdInterface::set_log_searches(const bool enabled) {
    AdInterfacePrivate::s_log_searches = enabled;
}

void AdInterface::set_dc(const QString &dc) {
    AdInterfacePrivate::s_dc = dc;
}

void AdInterface::set_sasl_nocanon(const bool is_on) {
    AdInterfacePrivate::s_sasl_nocanon = [&]() {
        if (is_on) {
            return LDAP_OPT_ON;
        } else {
            return LDAP_OPT_OFF;
        }
    }();
}

void AdInterface::set_port(const int port) {
    AdInterfacePrivate::s_port = port;
}

void AdInterface::set_cert_strategy(const CertStrategy strategy) {
    AdInterfacePrivate::s_cert_strat = strategy;
}

void AdInterface::set_domain_is_default(const bool is_default) {
    AdInterfacePrivate::s_domain_is_default = is_default;
}

void AdInterface::set_custom_domain(const QString &domain)
{
    AdInterfacePrivate::s_custom_domain = domain;
}

AdInterfacePrivate::AdInterfacePrivate(AdInterface *q_arg) {
    mutex.lock();
    q = q_arg;
    mutex.unlock();
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

AdConfig *AdInterface::adconfig() const {
    return d->adconfig;
}

QString AdInterface::client_user() const {
    return d->client_user;
}

// Helper f-n for search()
// NOTE: cookie is starts as NULL. Then after each while
// loop, it is set to the value returned by
// ldap_search_ext_s(). At the end cookie is set back to
// NULL.
bool AdInterfacePrivate::search_paged_internal(const char *base, const int scope, const char *filter, char **attributes, QHash<QString, AdObject> *results, AdCookie *cookie, const bool get_sacl) {
    int result;
    LDAPMessage *res = NULL;
    LDAPControl *page_control = NULL;
    LDAPControl *sd_control = NULL;
    LDAPControl **returned_controls = NULL;
    struct berval *prev_cookie = cookie->cookie;
    struct berval *new_cookie = NULL;

    auto cleanup = [&]() {
        ldap_msgfree(res);
        ldap_control_free(page_control);
        ldap_control_free(sd_control);
        ldap_controls_free(returned_controls);
        ber_bvfree(prev_cookie);
        ber_bvfree(new_cookie);
    };

    const int is_critical = 1;

    result = create_sd_control(get_sacl, is_critical, &sd_control);
    if (result != LDAP_SUCCESS) {
        qDebug() << "Failed to create sd control: " << ldap_err2string(result);

        cleanup();
        return false;
    }

    // Create page control
    const ber_int_t page_size = 100;
    result = ldap_create_page_control(ld, page_size, prev_cookie, is_critical, &page_control);
    if (result != LDAP_SUCCESS) {
        qDebug() << "Failed to create page control: " << ldap_err2string(result);

        cleanup();
        return false;
    }
    LDAPControl *server_controls[3] = {page_control, sd_control, NULL};

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

            const QList<QByteArray> values_bytes = [=]() {
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
    //
    // NOTE: not sure if absence of page response control is
    // an error. Decided to not treat it as error because
    // searching the rootDSE doesn't return this control.
    LDAPControl *pageresponse_control = ldap_control_find(LDAP_CONTROL_PAGEDRESULTS, returned_controls, NULL);
    if (pageresponse_control != NULL) {
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
    } else {
        cookie->cookie = NULL;
    }

    cleanup();
    return true;
}

QHash<QString, AdObject> AdInterface::search(const QString &base, const SearchScope scope, const QString &filter, const QList<QString> &attributes, const bool get_sacl) {
    AdCookie cookie;
    QHash<QString, AdObject> results;

    while (true) {
        const bool success = search_paged(base, scope, filter, attributes, &results, &cookie, get_sacl);

        if (!success) {
            break;
        }

        if (!cookie.more_pages()) {
            break;
        }
    }

    return results;
}

bool AdInterface::search_paged(const QString &base, const SearchScope scope, const QString &filter, const QList<QString> &attributes, QHash<QString, AdObject> *results, AdCookie *cookie, const bool get_sacl) {
    // NOTE: only log once per cycle of search pages,
    // to avoid duplicate messages
    const bool is_first_page = results->isEmpty();
    const bool need_to_log = (AdInterfacePrivate::s_log_searches && is_first_page);
    if (need_to_log) {
        const QString attributes_string = "{" + attributes.join(",") + "}";

        const QString scope_string = [&scope]() -> QString {
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

    const char *base_cstr = cstr(base);

    const int scope_int = [&]() {
        switch (scope) {
            case SearchScope_Object: return LDAP_SCOPE_BASE;
            case SearchScope_Children: return LDAP_SCOPE_ONELEVEL;
            case SearchScope_All: return LDAP_SCOPE_SUBTREE;
            case SearchScope_Descendants: return LDAP_SCOPE_CHILDREN;
        }
        return 0;
    }();

    const char *filter_cstr = [&]() {
        if (filter.isEmpty()) {
            // NOTE: need to pass NULL instead of empty
            // string to denote "no filter"
            return (const char *) NULL;
        } else {
            return cstr(filter);
        }
    }();

    // Convert attributes list to NULL-terminated array
    char **attributes_array;
    QList<QByteArray> attr_bytearray_list;
    if (attributes.isEmpty()) {
        // Pass NULL so LDAP gets all attributes
        attributes_array = NULL;
    } else {
        attributes_array = (char **) malloc((attributes.size() + 1) * sizeof(char *));
        if (attributes_array != NULL) {
            for (int i = 0; i < attributes.size(); i++) {
                attr_bytearray_list.append(attributes[i].toUtf8());
                attributes_array[i] = strdup(attr_bytearray_list.last().constData());
            }
            attributes_array[attributes.size()] = NULL;
        }
    }

    const bool search_success = d->search_paged_internal(base_cstr, scope_int, filter_cstr, attributes_array, results, cookie, get_sacl);
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

AdObject AdInterface::search_object(const QString &dn, const QList<QString> &attributes, const bool get_sacl) {
    const QString base = dn;
    const SearchScope scope = SearchScope_Object;
    const QString filter = QString();
    const QHash<QString, AdObject> results = search(base, scope, filter, attributes, get_sacl);

    if (results.contains(dn)) {
        return results[dn];
    } else {
        return AdObject();
    }
}

bool AdInterface::attribute_replace_values(const QString &dn, const QString &attribute, const QList<QByteArray> &values, const DoStatusMsg do_msg, const bool set_dacl) {
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

    int result;

    LDAPControl *server_controls[2] = {NULL, NULL};
    if (set_dacl) {
        LDAPControl *sd_control = NULL;
        const int is_critical = 1;

        result = create_sd_control(false, is_critical, &sd_control, set_dacl);
        if (result != LDAP_SUCCESS) {
            qDebug() << "Failed to create sd control: " << ldap_err2string(result);

            ldap_control_free(sd_control);
            return false;
        }

        server_controls[0] = sd_control;
    }

    result = ldap_modify_ext_s(d->ld, cstr(dn), attrs, server_controls, NULL);

    if (result == LDAP_SUCCESS) {
        d->success_message(QString(tr("Attribute %1 of object %2 was changed from \"%3\" to \"%4\".")).arg(attribute, name, old_values_display, values_display), do_msg);

        return true;
    } else {
        const QString context = QString(tr("Failed to change attribute %1 of object %2 from \"%3\" to \"%4\".")).arg(attribute, name, old_values_display, values_display);

        d->error_message(context, d->default_error(), do_msg);

        return false;
    }
}

bool AdInterface::attribute_replace_value(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg, const bool set_dacl) {
    const QList<QByteArray> values = [=]() -> QList<QByteArray> {
        if (value.isEmpty()) {
            return QList<QByteArray>();
        } else {
            return {value};
        }
    }();

    return attribute_replace_values(dn, attribute, values, do_msg, set_dacl);
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
    attr.mod_type = (char *) cstr(attribute);
    attr.mod_bvalues = values;

    LDAPMod *attrs[] = {&attr, NULL};

    const int result = ldap_modify_ext_s(d->ld, cstr(dn), attrs, NULL, NULL);
    free(data_copy);

    const QString name = dn_get_name(dn);
    const QString new_display_value = attribute_display_value(attribute, value, d->adconfig);

    if (result == LDAP_SUCCESS) {
        const QString context = QString(tr("Value \"%1\" was added for attribute %2 of object %3.")).arg(new_display_value, attribute, name);

        d->success_message(context, do_msg);

        return true;
    } else {
        const QString context = QString(tr("Failed to add value \"%1\" for attribute %2 of object %3.")).arg(new_display_value, attribute, name);

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
    attr.mod_type = (char *) cstr(attribute);
    attr.mod_bvalues = values;

    LDAPMod *attrs[] = {&attr, NULL};

    const int result = ldap_modify_ext_s(d->ld, cstr(dn), attrs, NULL, NULL);
    free(data_copy);

    if (result == LDAP_SUCCESS) {
        const QString context = QString(tr("Value \"%1\" for attribute %2 of object %3 was deleted.")).arg(value_display, attribute, name);

        d->success_message(context, do_msg);

        return true;
    } else {
        const QString context = QString(tr("Failed to delete value \"%1\" for attribute %2 of object %3.")).arg(value_display, attribute, name);

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

bool AdInterface::object_add(const QString &dn, const QHash<QString, QList<QString>> &attrs_map) {
    LDAPMod **attrs = [&attrs_map]() {
        LDAPMod **out = (LDAPMod **) malloc((attrs_map.size() + 1) * sizeof(LDAPMod *));

        const QList<QString> attrs_map_keys = attrs_map.keys();
        for (int i = 0; i < attrs_map_keys.size(); i++) {
            LDAPMod *attr = (LDAPMod *) malloc(sizeof(LDAPMod));

            const QString attr_name = attrs_map_keys[i];
            const QList<QString> value_list = attrs_map[attr_name];

            char **value_array = (char **) malloc((value_list.size() + 1) * sizeof(char *));
            for (int j = 0; j < value_list.size(); j++) {
                const QString value = value_list[j];
                value_array[j] = (char *) strdup(cstr(value));
            }
            value_array[value_list.size()] = NULL;

            attr->mod_type = (char *) strdup(cstr(attr_name));
            attr->mod_op = LDAP_MOD_ADD;
            attr->mod_values = value_array;

            out[i] = attr;
        }

        out[attrs_map.size()] = NULL;

        return out;
    }();

    const int result = ldap_add_ext_s(d->ld, cstr(dn), attrs, NULL, NULL);

    ldap_mods_free(attrs, 1);

    if (result == LDAP_SUCCESS) {
        d->success_message(QString(tr("Object %1 was created.")).arg(dn));

        return true;
    } else {
        const QString context = QString(tr("Failed to create object %1.")).arg(dn);

        const QString error = [this, dn]() {
            const bool wrong_ou_parent = [&]() {
                const int ldap_result = d->get_ldap_result();

                const bool is_ou = dn.startsWith("OU=");
                const QString parent = dn_get_parent(dn);
                const bool bad_parent = parent.startsWith("CN=");

                const bool out = (ldap_result == LDAP_NAMING_VIOLATION && is_ou && bad_parent);

                return out;
            }();

            if (wrong_ou_parent) {
                return tr("Can't create OU under this object type.");
            } else {
                return d->default_error();
            }
        }();

        d->error_message(context, error);

        return false;
    }
}

bool AdInterface::object_add(const QString &dn, const QString &object_class) {
    const QHash<QString, QList<QString>> attrs_map = {
        {"objectClass", {object_class}},
    };

    const bool success = object_add(dn, attrs_map);

    return success;
}

bool AdInterface::object_delete(const QString &dn, const DoStatusMsg do_msg) {
    int result;
    LDAPControl *tree_delete_control = NULL;

    auto cleanup = [tree_delete_control]() {
        ldap_control_free(tree_delete_control);
    };

    const QString name = dn_get_name(dn);

    const QString error_context = QString(tr("Failed to delete object %1.")).arg(name);

    // Use a tree delete control to enable recursive delete
    result = ldap_control_create(LDAP_CONTROL_X_TREE_DELETE, 1, NULL, 0, &tree_delete_control);
    if (result != LDAP_SUCCESS) {
        d->error_message(error_context, tr("LDAP Operation error - Failed to create tree delete control."));
        cleanup();

        return false;
    }

    LDAPControl *server_controls[2] = {NULL, NULL};

    const bool tree_delete_is_supported = adconfig()->control_is_supported(LDAP_CONTROL_X_TREE_DELETE);
    if (tree_delete_is_supported) {
        server_controls[0] = tree_delete_control;
    }

    result = ldap_delete_ext_s(d->ld, cstr(dn), server_controls, NULL);

    cleanup();

    if (result == LDAP_SUCCESS) {
        d->success_message(QString(tr("Object %1 was deleted.")).arg(name), do_msg);

        return true;
    } else {
        d->error_message(error_context, d->default_error(), do_msg);

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
        d->success_message(QString(tr("Object %1 was moved to %2.")).arg(object_name, container_name));

        return true;
    } else {
        const QString context = QString(tr("Failed to move object %1 to %2.")).arg(object_name, container_name);

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
        d->success_message(QString(tr("Object %1 was renamed to %2.")).arg(old_name, new_name));

        return true;
    } else {
        const QString context = QString(tr("Failed to rename object %1 to %2.")).arg(old_name, new_name);

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
        d->success_message(QString(tr("Object %1 was added to group %2.")).arg(user_name, group_name));

        return true;
    } else {
        const QString context = QString(tr("Failed to add object %1 to group %2.")).arg(user_name, group_name);

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
        d->success_message(QString(tr("Object %1 was removed from group %2.")).arg(user_name, group_name));

        return true;
    } else {
        const QString context = QString(tr("Failed to remove object %1 from group %2.")).arg(user_name, group_name);

        d->error_message(context, d->default_error());

        return false;
    }
}

bool AdInterface::group_set_scope(const QString &dn, GroupScope scope, const DoStatusMsg do_msg) {
    // NOTE: it is not possible to change scope from
    // global<->domainlocal directly, so have to switch to
    // universal first.
    const bool need_to_switch_to_universal = [=]() {
        const AdObject object = search_object(dn, {ATTRIBUTE_GROUP_TYPE});
        const GroupScope current_scope = object.get_group_scope();

        return (current_scope == GroupScope_Global && scope == GroupScope_DomainLocal) || (current_scope == GroupScope_DomainLocal && scope == GroupScope_Global);
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

        group_type = bitmask_set(group_type, this_scope_bit, false);
    }

    // Set given scope bit
    const int scope_bit = group_scope_bit(scope);
    group_type = bitmask_set(group_type, scope_bit, true);

    const QString name = dn_get_name(dn);
    const QString scope_string = group_scope_string(scope);

    const bool result = attribute_replace_int(dn, ATTRIBUTE_GROUP_TYPE, group_type);
    if (result) {
        d->success_message(QString(tr("Group scope for %1 was changed to \"%2\".")).arg(name, scope_string), do_msg);

        return true;
    } else {
        const QString context = QString(tr("Failed to change group scope for %1 to \"%2\".")).arg(name, scope_string);

        d->error_message(context, d->default_error(), do_msg);

        return false;
    }
}

bool AdInterface::group_set_type(const QString &dn, GroupType type) {
    const AdObject object = search_object(dn, {ATTRIBUTE_GROUP_TYPE});
    const int group_type = object.get_int(ATTRIBUTE_GROUP_TYPE);

    const bool set_security_bit = type == GroupType_Security;

    const int update_group_type = bitmask_set(group_type, GROUP_TYPE_BIT_SECURITY, set_security_bit);
    const QString update_group_type_string = QString::number(update_group_type);

    const QString name = dn_get_name(dn);
    const QString type_string = group_type_string(type);

    const bool result = attribute_replace_string(dn, ATTRIBUTE_GROUP_TYPE, update_group_type_string);
    if (result) {
        d->success_message(QString(tr("Group type for %1 was changed to \"%2\".")).arg(name, type_string));

        return true;
    } else {
        const QString context = QString(tr("Failed to change group type for %1 to \"%2\".")).arg(name, type_string);
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
        d->success_message(QString(tr("Primary group for object %1 was changed to %2.")).arg(user_name, group_name));

        return true;
    } else {
        const QString context = QString(tr("Failed to change primary group for user %1 to %2.")).arg(user_name, group_name);

        d->error_message(context, d->default_error());

        return false;
    }
}

bool AdInterface::user_set_pass(const QString &dn, const QString &password, const DoStatusMsg do_msg) {
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
        d->success_message(QString(tr("Password for object %1 was changed.")).arg(name), do_msg);

        return true;
    } else {
        const QString context = QString(tr("Failed to change password for object %1.")).arg(name);

        const QString error = [this]() {
            const int ldap_result = d->get_ldap_result();
            if (ldap_result == LDAP_CONSTRAINT_VIOLATION) {
                return tr("Password doesn't match rules.");
            } else {
                return d->default_error();
            }
        }();

        d->error_message(context, error, do_msg);

        return false;
    }
}

bool AdInterface::user_set_account_option(const QString &dn, AccountOption option, bool set) {
    if (dn.isEmpty()) {
        return false;
    }

    bool success = false;

    switch (option) {
        case AccountOption_CantChangePassword: {
            success = ad_security_set_user_cant_change_pass(this, dn, set);

            break;
        }
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
            const int uac = [this, dn]() {
                const AdObject object = search_object(dn, {ATTRIBUTE_USER_ACCOUNT_CONTROL});
                return object.get_int(ATTRIBUTE_USER_ACCOUNT_CONTROL);
            }();

            const int bit = account_option_bit(option);
            const int updated_uac = bitmask_set(uac, bit, set);

            success = attribute_replace_int(dn, ATTRIBUTE_USER_ACCOUNT_CONTROL, updated_uac, DoStatusMsg_No);
        }
    }

    const QString name = dn_get_name(dn);

    if (success) {
        const QString success_context = [option, set, name]() {
            switch (option) {
                case AccountOption_Disabled: {
                    if (set) {
                        return QString(tr("Object %1 has been disabled.")).arg(name);
                    } else {
                        return QString(tr("Object %1 has been enabled.")).arg(name);
                    }
                }
                default: {
                    const QString description = account_option_string(option);

                    if (set) {
                        return QString(tr("Account option \"%1\" was turned ON for object %2.")).arg(description, name);
                    } else {
                        return QString(tr("Account option \"%1\" was turned OFF for object %2.")).arg(description, name);
                    }
                }
            }
        }();

        d->success_message(success_context);

        return true;
    } else {
        const QString context = [option, set, name]() {
            switch (option) {
                case AccountOption_Disabled: {
                    if (set) {
                        return QString(tr("Failed to disable object %1.")).arg(name);
                    } else {
                        return QString(tr("Failed to enable object %1.")).arg(name);
                    }
                }
                default: {
                    const QString description = account_option_string(option);

                    if (set) {
                        return QString(tr("Failed to turn ON account option \"%1\" for object %2.")).arg(description, name);
                    } else {
                        return QString(tr("Failed to turn OFF account option \"%1\" for object %2.")).arg(description, name);
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
        d->success_message(QString(tr("User \"%1\" was unlocked.")).arg(name));

        return true;
    } else {
        const QString context = QString(tr("Failed to unlock user %1.")).arg(name);

        d->error_message(context, d->default_error());

        return result;
    }
}

bool AdInterface::computer_reset_account(const QString &dn) {
    const QString name = dn_get_name(dn);
    const QString reset_password = QString("%1$").arg(name);

    const bool success = user_set_pass(dn, reset_password, DoStatusMsg_No);

    if (success) {
        d->success_message(QString(tr("Computer \"%1\" was reset.")).arg(name));

        return true;
    } else {
        const QString context = QString(tr("Failed to reset computer %1.")).arg(name);

        d->error_message(context, d->default_error());

        return false;
    }
}

bool AdInterface::gpo_add(const QString &display_name, QString &dn_out) {
    const bool is_domain_admin = logged_in_as_domain_admin();

    auto error_message = [&](const QString &error) {
        if (!is_domain_admin) {
            d->error_message_plain(tr("Warning: User is not domain administrator."));
        }
        d->error_message(tr("Failed to create GPO."), error);
    };

    //
    // Generate UUID used for directory and object names
    //
    const QString uuid = []() {
        uuid_t uuid_struct;
        uuid_generate_random(uuid_struct);

        char uuid_cstr[UUID_STR_LEN];
        uuid_unparse_upper(uuid_struct, uuid_cstr);

        const QString out = "{" + QString(uuid_cstr) + "}";

        return out;
    }();

    // Ex: "\\domain.alt\sysvol\domain.alt\Policies\{FF7E0880-F3AD-4540-8F1D-4472CB4A7044}"
    const QString filesys_path = QString("\\\\%1\\sysvol\\%2\\Policies\\%3").arg(d->domain.toLower(), d->domain.toLower(), uuid);
    const QString gpt_path = filesys_path_to_smb_path(filesys_path);
    const QString gpc_dn = QString("CN=%1,CN=Policies,CN=System,%2").arg(uuid, adconfig()->domain_dn());

    // After each error case we need to clean up whatever we
    // have created successfully so far. Don't just use
    // gpo_delete() because we want to delete partially in
    // some error cases and that shouldn't print any error
    // messages.
    auto cleanup = [&]() {
        const AdObject gpc_object = search_object(gpc_dn);
        const bool gpc_exists = !gpc_object.is_empty();
        if (gpc_exists) {
            object_delete(gpc_dn);
        }

        struct stat filestat;
        const int stat_result = smbc_stat(cstr(gpt_path), &filestat);
        const bool gpt_exists = (stat_result == 0);
        if (gpt_exists) {
            d->delete_gpt(gpt_path);
        }
    };

    //
    // Create GPT
    //

    // Create root dir
    // "smb://domain.alt/sysvol/domain.alt/Policies/{FF7E0880-F3AD-4540-8F1D-4472CB4A7044}"
    const int result_mkdir_gpt = smbc_mkdir(cstr(gpt_path), 0755);
    if (result_mkdir_gpt != 0) {
        error_message(tr("Failed to create GPT root dir."));

        cleanup();

        return false;
    }

    const QString gpt_machine_path = gpt_path + "/Machine";
    const int result_mkdir_machine = smbc_mkdir(cstr(gpt_machine_path), 0755);
    if (result_mkdir_machine != 0) {
        error_message(tr("Failed to create GPT machine dir."));

        cleanup();

        return false;
    }

    const QString gpt_user_path = gpt_path + "/User";
    const int result_mkdir_user = smbc_mkdir(cstr(gpt_user_path), 0755);
    if (result_mkdir_user != 0) {
        error_message(tr("Failed to create GPT user dir."));

        cleanup();

        return false;
    }

    const QString gpt_ini_path = gpt_path + "/GPT.INI";
    const int ini_file = smbc_open(cstr(gpt_ini_path), O_WRONLY | O_CREAT, 0644);
    if (ini_file < 0) {
        error_message(tr("Failed to open GPT ini file."));

        cleanup();

        return false;
    }

    const char *ini_contents = "[General]\r\nVersion=0\r\n";
    const int bytes_written = smbc_write(ini_file, ini_contents, strlen(ini_contents));
    smbc_close(ini_file);
    if (bytes_written < 0) {
        error_message(tr("Failed to write GPT ini file."));

        cleanup();

        return false;
    }

    //
    // Create AD object for gpo
    //
    const bool result_add = object_add(gpc_dn, CLASS_GP_CONTAINER);
    if (!result_add) {
        error_message(tr("Failed to create GPC object."));

        cleanup();

        return false;
    }

    const QHash<QString, QString> attribute_value_map = {
        {ATTRIBUTE_DISPLAY_NAME, display_name},
        {ATTRIBUTE_GPC_FILE_SYS_PATH, filesys_path},
        // NOTE: samba defaults to 1, ADUC defaults to 0. Figure out what's this supposed to be.
        {ATTRIBUTE_FLAGS, "0"},
        {ATTRIBUTE_VERSION_NUMBER, "0"},
        {ATTRIBUTE_SHOW_IN_ADVANCED_VIEW_ONLY, "TRUE"},
        {ATTRIBUTE_GPC_FUNCTIONALITY_VERSION, "2"},
    };

    for (const QString &attribute : attribute_value_map.keys()) {
        const QString value = attribute_value_map[attribute];

        const bool replace_success = attribute_replace_string(gpc_dn, attribute, value);

        if (!replace_success) {
            error_message(QString(tr("Failed to set GPC attribute %1.")).arg(attribute));

            cleanup();

            return false;
        }
    }

    // User object
    const QString user_dn = "CN=User," + gpc_dn;
    const bool result_add_user = object_add(user_dn, CLASS_CONTAINER);
    attribute_replace_string(gpc_dn, ATTRIBUTE_SHOW_IN_ADVANCED_VIEW_ONLY, "TRUE");
    if (!result_add_user) {
        error_message(tr("Failed to create user folder object for GPO."));

        cleanup();

        return false;
    }

    // Machine object
    const QString machine_dn = "CN=Machine," + gpc_dn;
    const bool result_add_machine = object_add(machine_dn, CLASS_CONTAINER);
    attribute_replace_string(gpc_dn, ATTRIBUTE_SHOW_IN_ADVANCED_VIEW_ONLY, "TRUE");
    if (!result_add_machine) {
        error_message(tr("Failed to create machine folder object for GPO."));

        cleanup();

        return false;
    }

    const bool sync_perms_success = gpo_sync_perms(gpc_dn);

    if (!sync_perms_success) {
        // NOTE: don't fail if failed to sync perms, user
        // can retry it later
    }

    dn_out = gpc_dn;

    return true;
}

QList<QString> AdInterfacePrivate::gpo_get_gpt_contents(const QString &gpt_root_path, bool *ok) {
    // Collect all contents of the path into a list
    QList<QString> explore_stack;
    QList<QString> seen_stack;

    explore_stack.append(gpt_root_path);
    seen_stack.append(gpt_root_path);

    const QString error_context = QString(tr("Failed to get contents of GPT \"%1\".")).arg(gpt_root_path);

    while (!explore_stack.isEmpty()) {
        const QString path = explore_stack.takeLast();

        const int dirp = smbc_opendir(cstr(path));

        if (dirp < 0) {
            *ok = false;

            error_message(error_context, tr("Failed to open dir."));

            return QList<QString>();
        }

        // NOTE: set errno to 0, so that we know
        // when readdir() fails because it will
        // change errno.
        errno = 0;

        smbc_dirent *child_dirent;
        while ((child_dirent = smbc_readdir(dirp)) != NULL) {
            const QString child_name = QString(child_dirent->name);

            const bool is_dot_path = (child_name == "." || child_name == "..");
            if (is_dot_path) {
                continue;
            } else {
                const QString child_path = path + "/" + child_name;

                seen_stack.append(child_path);

                const bool child_is_dir = smb_path_is_dir(child_path, ok);
                if (!*ok) {
                    return QList<QString>();
                }

                if (child_is_dir) {
                    explore_stack.append(child_path);
                }
            }
        }

        if (errno != 0) {
            *ok = false;

            error_message(error_context, tr("Failed to read dir."));

            return QList<QString>();
        }

        smbc_closedir(dirp);
    }

    return seen_stack;
}

bool AdInterface::gpo_delete(const QString &dn, bool *deleted_object) {
    // NOTE: try to execute both steps, even if first one
    // (deleting gpc) fails

    // NOTE: get filesys path before deleting object,
    // otherwise it won't be available!
    const AdObject object = search_object(dn, {ATTRIBUTE_GPC_FILE_SYS_PATH, ATTRIBUTE_DISPLAY_NAME});
    const QString filesys_path = object.get_string(ATTRIBUTE_GPC_FILE_SYS_PATH);

    const QString name = object.get_string(ATTRIBUTE_DISPLAY_NAME);
    const QString smb_path = filesys_path_to_smb_path(filesys_path);

    const bool delete_gpc_success = object_delete(dn);
    if (!delete_gpc_success) {
        d->error_message(tr("Failed to delete GPC."), d->default_error());
    }

    const bool delete_gpt_success = d->delete_gpt(smb_path);
    if (!delete_gpt_success) {
        d->error_message_plain(tr("Failed to delete GPT."));
    }

    // Unlink policy
    const QString base = adconfig()->domain_dn();
    const SearchScope scope = SearchScope_All;
    const QList<QString> attributes = {ATTRIBUTE_GPLINK};
    const QString filter = filter_CONDITION(Condition_Contains, ATTRIBUTE_GPLINK, dn);
    const QHash<QString, AdObject> results = search(base, scope, filter, attributes);
    for (const AdObject &linked_object : results.values()) {
        const QString gplink_old_string = linked_object.get_string(ATTRIBUTE_GPLINK);

        Gplink gplink = Gplink(gplink_old_string);
        gplink.remove(dn);

        attribute_replace_string(linked_object.get_dn(), ATTRIBUTE_GPLINK, gplink.to_string());
    }

    const bool total_success = (delete_gpc_success && delete_gpt_success);

    if (total_success) {
        d->success_message(QString(tr("Group policy %1 was deleted.")).arg(name));
    } else {
        const bool partial_success = (delete_gpc_success || delete_gpt_success);
        if (partial_success) {
            d->success_message(QString(tr("Errors happened while trying to delete policy %1.")).arg(name));
        } else {
            d->success_message(QString(tr("Failed to delete policy %1.")).arg(name));
        }
    }

    *deleted_object = delete_gpc_success;

    return total_success;
}

QString AdInterface::filesys_path_to_smb_path(const QString &filesys_path) const {
    QString out = filesys_path;

    // NOTE: sysvol paths created by windows have this weird
    // capitalization and smbclient does NOT like it
    out.replace("\\SysVol\\", "\\sysvol\\");

    out.replace("\\", "/");

    const int sysvol_i = out.indexOf("/sysvol/");

    out.remove(0, sysvol_i);

    out = QString("smb://%1%2").arg(d->dc, out);

    return out;
}

bool AdInterface::ldap_init() {
    const QString connect_error_context = tr("Failed to connect.");

    const QString uri = [&]() {
        QString out;

        if (!d->dc.isEmpty()) {
            out = "ldap://" + d->dc;

            if (AdInterfacePrivate::s_port > 0) {
                out = out + ":" + AdInterfacePrivate::s_port;
            }
        }

        return out;
    }();

    if (uri.isEmpty()) {
        return false;
    }

    int result;

    // NOTE: this doesn't leak memory. False positive.
    result = ldap_initialize(&d->ld, cstr(uri));
    if (result != LDAP_SUCCESS) {
        ldap_memfree(d->ld);
        d->error_message(tr("Failed to initialize LDAP library."), strerror(errno));

        return false;
    }

    auto option_error = [&](const QString &option) {
        d->error_message(connect_error_context, QString(tr("Failed to set ldap option %1.")).arg(option));
    };

    // Set version
    const int version = LDAP_VERSION3;
    result = ldap_set_option(d->ld, LDAP_OPT_PROTOCOL_VERSION, &version);
    if (result != LDAP_OPT_SUCCESS) {
        option_error("LDAP_OPT_PROTOCOL_VERSION");
        return false;
    }

    // Disable referrals
    result = ldap_set_option(d->ld, LDAP_OPT_REFERRALS, LDAP_OPT_OFF);
    if (result != LDAP_OPT_SUCCESS) {
        option_error("LDAP_OPT_REFERRALS");
        return false;
    }

    // Set SASL propertry min_ssf to the minimum acceptable security layer strength.
    // SSF is a rough indication of how secure the connection is. A connection
    // secured by 56-bit DES would have an SSF of 56.
    const char *sasl_secprops = "minssf=56";
    result = ldap_set_option(d->ld, LDAP_OPT_X_SASL_SECPROPS, sasl_secprops);
    if (result != LDAP_SUCCESS) {
        option_error("LDAP_OPT_X_SASL_SECPROPS");
        return false;
    }

    result = ldap_set_option(d->ld, LDAP_OPT_X_SASL_NOCANON, AdInterfacePrivate::s_sasl_nocanon);
    if (result != LDAP_SUCCESS) {
        option_error("LDAP_OPT_X_SASL_NOCANON");
        return false;
    }

    const int cert_strategy = [&]() {
        switch (AdInterfacePrivate::s_cert_strat) {
            case CertStrategy_Never: return LDAP_OPT_X_TLS_NEVER;
            case CertStrategy_Hard: return LDAP_OPT_X_TLS_HARD;
            case CertStrategy_Demand: return LDAP_OPT_X_TLS_DEMAND;
            case CertStrategy_Allow: return LDAP_OPT_X_TLS_ALLOW;
            case CertStrategy_Try: return LDAP_OPT_X_TLS_TRY;
        }

        return LDAP_OPT_X_TLS_NEVER;
    }();

    ldap_set_option(d->ld, LDAP_OPT_X_TLS_REQUIRE_CERT, &cert_strategy);
    if (result != LDAP_SUCCESS) {
        option_error("LDAP_OPT_X_TLS_REQUIRE_CERT");
        return false;
    }

    // Setup sasl_defaults_gssapi
    struct sasl_defaults_gssapi defaults;
    defaults.mech = (char *) "GSSAPI";
    ldap_get_option(d->ld, LDAP_OPT_X_SASL_REALM, &defaults.realm);
    ldap_get_option(d->ld, LDAP_OPT_X_SASL_AUTHCID, &defaults.authcid);
    ldap_get_option(d->ld, LDAP_OPT_X_SASL_AUTHZID, &defaults.authzid);
    defaults.passwd = NULL;

    // Perform bind operation
    unsigned sasl_flags = LDAP_SASL_QUIET;
    result = ldap_sasl_interactive_bind_s(d->ld, NULL, defaults.mech, NULL, NULL, sasl_flags, sasl_interact_gssapi, &defaults);
    ldap_memfree(defaults.realm);
    ldap_memfree(defaults.authcid);
    ldap_memfree(defaults.authzid);
    if (result != LDAP_SUCCESS) {
        d->error_message_plain(tr("Failed to connect to server. Check your connection and make sure you have initialized your credentials using kinit."));
        d->error_message_plain(d->default_error());

        return false;
    }

    d->client_user = [&]() {
        char *out_cstr = NULL;
        ldap_get_option(d->ld, LDAP_OPT_X_SASL_USERNAME, &out_cstr);

        if (out_cstr == NULL) {
            return QString();
        }

        QString out = QString(out_cstr);
        out = out.toLower();

        ldap_memfree(out_cstr);

        return out;
    }();

    return true;
}

void AdInterface::ldap_free() {
    if (d->is_connected) {
        ldap_unbind_ext(d->ld, NULL, NULL);
    } else {
        ldap_memfree(d->ld);
    }
}

bool AdInterface::init_smb_context() {
    const QString connect_error_context = tr("Failed to connect.");

    if (AdInterfacePrivate::smbc == NULL) {
        AdInterfacePrivate::smbc = smbc_new_context();
        smbc_setOptionUseKerberos(AdInterfacePrivate::smbc, true);
        smbc_setOptionFallbackAfterKerberos(AdInterfacePrivate::smbc, true);
        if (!smbc_init_context(AdInterfacePrivate::smbc)) {
            d->error_message(connect_error_context, tr("Failed to initialize SMB context."));

            return false;
        }
        smbc_set_context(AdInterfacePrivate::smbc);
    }
    return true;
}

bool AdInterface::gpo_check_perms(const QString &gpo, bool *ok) {
    // NOTE: skip perms check for non-admins, because don't
    // have enough rights to get full sd
    if (!logged_in_as_domain_admin()) {
        return true;
    }

    const QList<QString> attributes = QList<QString>();
    const bool get_sacl = true;
    const AdObject gpc_object = search_object(gpo, attributes, get_sacl);
    const QString name = gpc_object.get_string(ATTRIBUTE_DISPLAY_NAME);

    const QString error_context = QString(tr("Failed to check permissions for GPO \"%1\".")).arg(name);

    const QString gpc_sd = [&]() {
        const QString out = get_gpt_sd_string(gpc_object, AceMaskFormat_Hexadecimal);

        if (out.isEmpty()) {
            d->error_message(error_context, tr("Failed to get GPT security descriptor."));

            return QString();
        }

        return out;
    }();

    const QString gpt_sd = [&]() {
        const QString filesys_path = gpc_object.get_string(ATTRIBUTE_GPC_FILE_SYS_PATH);
        const QString smb_path = filesys_path_to_smb_path(filesys_path);
        const char *smb_path_cstr = cstr(smb_path);

        // NOTE: the length of gpt sd string doesn't have a
        // well defined bound, so we have to use an
        // expanding buffer
        size_t buffer_size = 1024 * sizeof(char);
        char *buffer = (char *) malloc(buffer_size);

        while (true) {
            const int getxattr_result = smbc_getxattr(smb_path_cstr, "system.nt_sec_desc.*", buffer, buffer_size);

            // NOTE: for some reason getxattr() returns positive
            // non-zero return code on success, even though f-n
            // description says it "returns 0 on success"
            const bool success = (getxattr_result >= 0);

            if (success) {
                break;
            } else {
                const bool buffer_is_too_small = (errno == ERANGE);

                if (buffer_is_too_small) {
                    // Error occured, but it is due to
                    // insufficient buffer size, so try
                    // again with bigger buffer
                    buffer_size = 2 * buffer_size;
                    buffer = (char *) realloc(buffer, buffer_size);
                } else {
                    const QString text = QString(tr("Failed to get GPT security descriptor, %1.")).arg(strerror(errno));
                    d->error_message(error_context, text);

                    free(buffer);

                    return QString();
                }
            }
        }

        const QString out = QString(buffer);

        free(buffer);

        return out;
    }();

    qDebug() << "--------";
    qDebug() << "gpc_sd:";
    for (auto e : QString(gpc_sd).split(",")) {
        qDebug() << e;
    }

    qDebug() << "--------";
    qDebug() << "gpt_sd:";
    for (auto e : QString(gpt_sd).split(",")) {
        qDebug() << e;
    }

    if (gpc_sd.isEmpty() || gpt_sd.isEmpty()) {
        *ok = false;

        return false;
    }

    // SD's match if they both contain all lines of the
    // other one. Order doesn't matter. Note that
    // simple equality doesn't work because entry order
    // may not match.
    //
    // NOTE: there's also a weird thing where RSAT
    // creates GPO's with duplicate ace's for Domain
    // Admins. Not sure why that happens but this
    // matching method ignores that quirk.
    const bool sd_match = [&]() {
        const QList<QString> gpt_list = QString(gpt_sd).split(",");
        const QList<QString> gpc_list = QString(gpc_sd).split(",");

        for (const QString &line : gpt_list) {
            if (!gpc_list.contains(line)) {
                return false;
            }
        }

        for (const QString &line : gpc_list) {
            if (!gpt_list.contains(line)) {
                return false;
            }
        }

        return true;
    }();

    return sd_match;
}

bool AdInterface::gpo_sync_perms(const QString &dn) {
    // First get GPC descriptor
    const QList<QString> attributes = QList<QString>();
    const bool get_sacl = true;
    const AdObject gpc_object = search_object(dn, attributes, get_sacl);
    const QString name = gpc_object.get_string(ATTRIBUTE_DISPLAY_NAME);
    const QString gpt_sd_string = get_gpt_sd_string(gpc_object, AceMaskFormat_Decimal);

    const QString error_context = QString(tr("Failed to sync permissions of GPO \"%1\".")).arg(name);

    if (gpt_sd_string.isEmpty()) {
        d->error_message(error_context, tr("Failed to generate GPT security descriptor."));

        return false;
    }

    // Get list of GPT contents

    // NOTE: order is important, have to set perms of parent
    // folders before their contents, otherwise fails to
    // set! Default gpo_get_gpt_contents() order is good.
    const QString filesys_path = gpc_object.get_string(ATTRIBUTE_GPC_FILE_SYS_PATH);
    const QString smb_path = filesys_path_to_smb_path(filesys_path);
    bool ok = true;
    const QList<QString> path_list = d->gpo_get_gpt_contents(smb_path, &ok);
    if (!ok || path_list.isEmpty()) {
        d->error_message(error_context, QString(tr("Failed to read GPT contents of \"%1\".")).arg(smb_path));
        return false;
    }

    // Set descriptor on all GPT contents
    for (const QString &path : path_list) {
        const int set_sd_result = smbc_setxattr(cstr(path), "system.nt_sec_desc.*", cstr(gpt_sd_string), strlen(cstr(gpt_sd_string)), 0);
        if (set_sd_result != 0) {
            const QString error = QString(tr("Failed to set permissions, %1.")).arg(strerror(errno));
            d->error_message(error_context, error);

            return false;
        }
    }

    d->success_message(QString(tr("Synced permissions of GPO \"%1\".")).arg(name));

    return true;
}

bool AdInterface::gpo_get_sysvol_version(const AdObject &gpc_object, int *version_out) {
    const QString error_context = tr("Failed to load GPO's sysvol version.");

    const QString ini_contents = [&]() {
        const QString filesys_path = gpc_object.get_string(ATTRIBUTE_GPC_FILE_SYS_PATH);
        const QString smb_path = filesys_path_to_smb_path(filesys_path);

        const QString ini_path = smb_path + "/GPT.INI";

        const int ini_fd = smbc_open(cstr(ini_path), O_RDONLY, 0);

        if (ini_fd < 0) {
            const QString error_text = QString(tr("Failed to open GPT.INI, %1.")).arg(strerror(errno));
            d->error_message(error_context, error_text);

            return QString();
        }

        const size_t buffer_size = 2000;
        char buffer[buffer_size];
        const ssize_t bytes_read = smbc_read(ini_fd, buffer, buffer_size);

        if (bytes_read < 0) {
            const QString error_text = QString(tr("Failed to open GPT.INI, %1.")).arg(strerror(errno));
            d->error_message(error_context, error_text);

            return QString();
        }

        smbc_close(ini_fd);

        return QString(buffer);
    }();

    if (ini_contents.isEmpty()) {
        return false;
    }

    const int version = [&]() {
        int out;

        const int scan_result = sscanf(cstr(ini_contents), "[General]\r\nVersion=%i\r\n", &out);
        const bool scan_success = (scan_result > 0);

        if (!scan_success) {
            const QString error_text = QString(tr("Failed to extract version from GPT.INI, %1.")).arg(strerror(errno));
            d->error_message(error_context, error_text);

            return -1;
        }

        return out;
    }();

    if (version >= 0) {
        *version_out = version;

        return true;
    } else {
        return false;
    }
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
        msg += QString(tr(" Error: \"%1\"")).arg(error);

        // Add period if needed, because some error strings,
        // like the ones from LDAP, might not have one
        if (!msg.endsWith(".")) {
            msg += ".";
        }
    }

    const AdMessage message(msg, AdMessageType_Error);
    messages.append(message);
}

void AdInterfacePrivate::error_message_plain(const QString &text, const DoStatusMsg do_msg) {
    if (do_msg == DoStatusMsg_No) {
        return;
    }

    const AdMessage message(text, AdMessageType_Error);
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

bool AdInterfacePrivate::delete_gpt(const QString &parent_path) {
    bool ok = true;

    QList<QString> path_list = gpo_get_gpt_contents(parent_path, &ok);
    if (!ok) {
        return false;
    }

    // NOTE: have to reverse so deepest paths are first to
    // delete correctly
    std::reverse(path_list.begin(), path_list.end());

    for (const QString &path : path_list) {
        const bool is_dir = smb_path_is_dir(path, &ok);
        if (!ok) {
            return false;
        }

        if (is_dir) {
            const int result_rmdir = smbc_rmdir(cstr(path));

            if (result_rmdir != 0) {
                error_message(QString(tr("Failed to delete GPT folder %1.")).arg(path), strerror(errno));

                return false;
            }
        } else {
            const int result_unlink = smbc_unlink(cstr(path));

            if (result_unlink != 0) {
                error_message(QString(tr("Failed to delete GPT file %1.")).arg(path), strerror(errno));

                return false;
            }
        }
    }

    return true;
}

bool AdInterfacePrivate::smb_path_is_dir(const QString &path, bool *ok) {
    struct stat filestat;
    const int stat_result = smbc_stat(cstr(path), &filestat);
    if (stat_result != 0) {
        error_message(QString(tr("Failed to get filestat for \"%1\".")).arg(path), strerror(errno));

        *ok = false;

        return false;
    } else {
        *ok = true;

        const bool is_dir = S_ISDIR(filestat.st_mode);

        return is_dir;
    }
}

// NOTE: this f-n is analogous to
// ldap_create_page_control() and others. See pagectl.c
// in ldap sources for examples. Extracted to contain
// the C madness.
int create_sd_control(bool get_sacl, int is_critical, LDAPControl **ctrlp, bool set_dacl) {
    // NOTE: SACL part of the sd can only be obtained
    // by administrators. For most operations we don't
    // need SACL. Some operations, like creating a GPO,
    // do require SACL, so for those operations we turn
    // on the "get_sacl" options.
    int flags = OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION;
    if (get_sacl) {
        flags |= SACL_SECURITY_INFORMATION;
    }
    else if (set_dacl) {
        flags = DACL_SECURITY_INFORMATION;
    }

    BerElement *value_be = NULL;
    struct berval value;

    value_be = ber_alloc_t(LBER_USE_DER);
    ber_printf(value_be, "{i}", flags);
    ber_flatten2(value_be, &value, 1);

    // Create control
    const int result = ldap_control_create(LDAP_SERVER_SD_FLAGS_OID,
        is_critical, &value, 0, ctrlp);

    if (result != LDAP_SUCCESS) {
        ber_memfree(value.bv_val);
    }

    ber_free(value_be, 1);

    return result;
}

bool AdInterface::logged_in_as_domain_admin() {
    const QString sam_account_name = d->client_user.split('@')[0];
    const QString client_user_filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_SAM_ACCOUNT_NAME, sam_account_name);
    const QHash<QString, AdObject> client_user_results = search(adconfig()->domain_dn(), SearchScope_All, client_user_filter, {ATTRIBUTE_PRIMARY_GROUP_ID});
    if (client_user_results.isEmpty()) {
        return false;
    }

    const QString user_dn = client_user_results.keys()[0];
    if (user_dn.isEmpty()) {
        return false;
    }

    int primary_group_id = client_user_results.values()[0].get_int(ATTRIBUTE_PRIMARY_GROUP_ID);
    const int domain_admins_rid = 512;
    if (primary_group_id == domain_admins_rid) {
        return true;
    }

    const QString domain_admins_sid = adconfig()->domain_sid() + "-" + QString::number(domain_admins_rid);
    const QString filter_group = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_GROUP);
    const QString filter_sid = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_SID, domain_admins_sid);
    const QString filter = filter_AND({filter_group, filter_sid});
    const QHash<QString, AdObject> admin_group_results = search(adconfig()->domain_dn(), SearchScope_All, filter, QList<QString>());
    if (admin_group_results.isEmpty()) {
        d->error_message(tr("Failed to check user permissions."), tr("Can't find domain admins group with SID ") + domain_admins_sid);
        return false;
    }

    const AdObject domain_admins_object = admin_group_results.values()[0];
    const QString rule_in_chain_filter = filter_matching_rule_in_chain(ATTRIBUTE_MEMBER_OF, domain_admins_object.get_dn());
    const QHash<QString, AdObject> res = search(user_dn, SearchScope_Object, rule_in_chain_filter, QList<QString>());

    return res.keys().contains(user_dn);
}

QString AdInterface::get_dc() const {
    return d->dc;
}

QString AdInterface::get_domain() const {
    return d->domain;
}

void AdInterface::update_dc() {
    d->dc = AdInterfacePrivate::s_dc;

    // Reinit ldap connection with updated DC
    ldap_free();
    d->is_connected = ldap_init();
    d->is_connected = init_smb_context();
}

QList<QString> get_domain_hosts(const QString &domain, const QString &site) {
    QList<QString> hosts;

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

    auto error = []() {
        return QList<QString>();
    };

    const long unsigned msg_len = res_search(dname, ns_c_in, ns_t_srv, msg.buf, sizeof(msg.buf));

    const bool message_error = (msg_len < sizeof(HEADER));
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

/**
 * Callback for ldap_sasl_interactive_bind_s
 */
int sasl_interact_gssapi(LDAP *ld, unsigned flags, void *indefaults, void *in) {
    UNUSED_ARG(flags);

    sasl_defaults_gssapi *defaults = (sasl_defaults_gssapi *) indefaults;
    sasl_interact_t *interact = (sasl_interact_t *) in;

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

// NOTE: decimal format option is provided to deal with this
// bug in libsmbclient:
// https://bugzilla.samba.org/show_bug.cgi?id=14303. You
// only need to use decimal format when making the string to
// pass to smbc_setxattr(), otherwise you should use hex
// format.
QString get_gpt_sd_string(const AdObject &gpc_object, const AceMaskFormat format_enum) {
    TALLOC_CTX *mem_ctx = talloc_new(NULL);

    security_descriptor *gpc_sd = gpc_object.get_security_descriptor(mem_ctx);

    struct security_descriptor *gpt_sd;
    const NTSTATUS create_sd_status = gp_create_gpt_security_descriptor(mem_ctx, gpc_sd, &gpt_sd);

    if (!NT_STATUS_IS_OK(create_sd_status)) {
        qDebug() << "Failed to create gpt sd";
        talloc_free(mem_ctx);

        return QString();
    }

    security_descriptor_sort_dacl(gpt_sd);

    QList<QString> all_elements;

    all_elements.append(QString("REVISION:%1").arg(gpt_sd->revision));

    const QString owner_sid_string = dom_sid_string(mem_ctx, gpt_sd->owner_sid);
    all_elements.append(QString("OWNER:%1").arg(owner_sid_string));

    const QString group_sid_string = dom_sid_string(mem_ctx, gpt_sd->group_sid);
    all_elements.append(QString("GROUP:%1").arg(group_sid_string));

    // NOTE: don't need sacl

    for (uint32_t i = 0; i < gpt_sd->dacl->num_aces; i++) {
        struct security_ace ace = gpt_sd->dacl->aces[i];

        char access_mask_string[100];
        static const char *hex_format = "0x%08x";
        static const char *dec_format = "%d";
        const char *format = [&]() {
            switch (format_enum) {
                case AceMaskFormat_Hexadecimal: return hex_format;
                case AceMaskFormat_Decimal: return dec_format;
            }

            return hex_format;
        }();
        snprintf(access_mask_string, sizeof(access_mask_string), format, ace.access_mask);

        const char *trustee_string = dom_sid_string(mem_ctx, &ace.trustee);

        all_elements.append(QString("ACL:%1:%2/%3/%4").arg(trustee_string, QString::number(ace.type), QString::number(ace.flags), access_mask_string));
    }

    // NOTE: can get duplicate ace's because ace's are
    // modified for gpt format, so remove duplicates
    QList<QString> without_duplicates;
    for (const QString &element : all_elements) {
        if (!without_duplicates.contains(element)) {
            without_duplicates.append(element);
        }
    }

    const QString out = without_duplicates.join(",");

    talloc_free(mem_ctx);

    return out;
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
