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

#include "ad_utils.h"
#include "ad_config.h"
#include "ad_object.h"
#include "attribute_display.h"
#include "status.h"
#include "utils.h"

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
#include <QCoreApplication>

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

QList<QString> get_domain_hosts(const QString &domain, const QString &site);
QList<QString> query_server_for_hosts(const char *dname);
bool ad_connect(const char* uri, LDAP **ld_out);
int sasl_interact_gssapi(LDAP *ld, unsigned flags, void *indefaults, void *in);

void get_auth_data_fn(const char * pServer, const char * pShare, char * pWorkgroup, int maxLenWorkgroup, char * pUsername, int maxLenUsername, char * pPassword, int maxLenPassword) {

}

AdSignals *ADSIGNALS() {
    static AdSignals instance;
    return &instance;
}

AdInterface::AdInterface(QObject *parent)
: QObject(parent) {
    ld = NULL;
    smbc = NULL;
    m_is_connected = false;

    // Get default domain from krb5
    const QString domain =
    []() {
        krb5_error_code result;
        krb5_context context;
        char *realm_cstr = NULL;

        auto cleanup =
        [&]() {
            krb5_free_default_realm(context, realm_cstr);
            krb5_free_context(context);
        };

        result = krb5_init_context(&context);
        if (result) {
            qDebug() << "Failed to init krb5 context";

            cleanup();
            return QString();
        }

        result = krb5_get_default_realm(context, &realm_cstr);
        if (result) {
            qDebug() << "Failed to get default realm";

            cleanup();
            return QString();
        }

        const QString out = QString(realm_cstr);

        cleanup();

        return out;
    }();

    qDebug() << "domain=" << domain;

    const QList<QString> hosts = get_domain_hosts(domain, QString());
    if (hosts.isEmpty()) {
        qDebug() << "No hosts found";

        error_status_message(tr("Failed to connect"), tr("Not connected to a domain network"));

        return;
    }
    qDebug() << "hosts=" << hosts;

    // TODO: for now selecting first host, which seems to be fine but investigate what should be selected.
    m_host = hosts[0];

    const QString uri = "ldap://" + m_host;

    m_domain = domain;

    // Transform domain to search base
    // "DOMAIN.COM" => "DC=domain,DC=com"
    m_domain_head = m_domain;
    m_domain_head = m_domain_head.toLower();
    m_domain_head = "DC=" + m_domain_head;
    m_domain_head = m_domain_head.replace(".", ",DC=");

    m_configuration_dn = "CN=Configuration," + m_domain_head;
    m_schema_dn = "CN=Schema," + m_configuration_dn;

    const bool success = ad_connect(cstr(uri), &ld);

    // TODO: can move this do a dtor, but for now this is
    // easier to do than ovewriting qobject's dtor
    connect(
        this, &QObject::destroyed,
        [this]() {
            smbc_free_context(smbc, 0);

            if (m_is_connected) {
                ldap_unbind_ext(ld, NULL, NULL);
            } else {
                ldap_memfree(ld);
            }
        });

    if (success) {
        // TODO: can this context expire, for example from a disconnect?
        // NOTE: this doesn't leak memory. False positive.
        smbc_init(get_auth_data_fn, 0);
        smbc = smbc_new_context();
        smbc_setOptionUseKerberos(smbc, true);
        smbc_setOptionFallbackAfterKerberos(smbc, true);
        if (!smbc_init_context(smbc)) {
            smbc_free_context(smbc, 0);
            qDebug() << "Could not initialize smbc context";
        }
        smbc_set_context(smbc);

        m_is_connected = true;
    } else {
        error_status_message(tr("Failed to connect"), tr("Authentication failed"));
    }
}

bool AdInterface::is_connected() const {
    return m_is_connected;
}

QString AdInterface::domain() const {
    return m_domain;
}

QString AdInterface::domain_head() const {
    return m_domain_head;
}

QString AdInterface::configuration_dn() const {
    return m_configuration_dn;
}

QString AdInterface::schema_dn() const {
    return m_schema_dn;
}

QString AdInterface::host() const {
    return m_host;
}

void AdInterface::stop_search() {
    stop_search_flag = true;
}

// Helper f-n for search()
// NOTE: cookie is starts as NULL. Then after each while
// loop, it is set to the value returned by
// ldap_search_ext_s(). At the end cookie is set back to
// NULL.
bool search_paged(LDAP *ld, const char *filter, char **attributes, const int scope, const char *search_base, QHash<QString, AdObject> *out, struct berval **cookie) {
    int result;
    LDAPMessage *res = NULL;
    LDAPControl *page_control = NULL;
    LDAPControl **returned_controls = NULL;
    struct berval *prev_cookie = *cookie;
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
    result = ldap_search_ext_s(ld, search_base, scope, filter, attributes, attrsonly, server_controls, NULL, NULL, LDAP_NO_LIMIT, &res);
    if ((result != LDAP_SUCCESS) && (result != LDAP_PARTIAL_RESULTS)) {
        qDebug() << "Error in paged ldap_search_ext_s: " << ldap_err2string(result);

        cleanup();
        return false;
    }

    // Collect results for this search
    for (LDAPMessage *entry = ldap_first_entry(ld, res); entry != NULL; entry = ldap_next_entry(ld, entry)) {
        char *dn_cstr = ldap_get_dn(ld, entry);
        const QString dn(dn_cstr);
        ldap_memfree(dn_cstr);

        AdObjectAttributes object_attributes;

        BerElement *berptr;
        for (char *attr = ldap_first_attribute(ld, entry, &berptr); attr != NULL; attr = ldap_next_attribute(ld, entry, berptr)) {
            struct berval **values_ldap = ldap_get_values_len(ld, entry, attr);

            const QList<QByteArray> values_bytes =
            [=]() {
                QList<QByteArray> values_bytes_out;

                if (values_ldap != NULL) {
                    const int values_count = ldap_count_values_len(values_ldap);
                    for (int i = 0; i < values_count; i++) {
                        struct berval value_berval = *values_ldap[i];
                        const QByteArray value_bytes(value_berval.bv_val, value_berval.bv_len);

                        values_bytes_out.append(value_bytes);
                    }
                }

                return values_bytes_out;
            }();

            const QString attribute(attr);

            object_attributes[attribute] = values_bytes;

            ldap_value_free_len(values_ldap);
            ldap_memfree(attr);
        }
        ber_free(berptr, 0);

        AdObject object;
        object.load(dn, object_attributes);

        out->insert(dn, object);
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
        *cookie = ber_bvdup(new_cookie);
    } else {
        *cookie = NULL;
    }

    cleanup();
    return true;
}

QHash<QString, AdObject> AdInterface::search(const QString &filter_arg, const QList<QString> &attributes_arg, const SearchScope scope_arg, const QString &search_base_arg) {
    stop_search_flag = false;

    QHash<QString, AdObject> out;

    const char *search_base =
    [this, search_base_arg]() {
        if (search_base_arg.isEmpty()) {
            return cstr(m_domain_head);
        } else {
            return cstr(search_base_arg);
        }
    }();

    const char *filter =
    [filter_arg]() {
        if (filter_arg.isEmpty()) {
            // NOTE: need to pass NULL instead of empty
            // string to denote "no filter"
            return (const char *) NULL;
        } else {
            return cstr(filter_arg);
        }
    }();

    const int scope =
    [scope_arg]() {
        switch (scope_arg) {
            case SearchScope_Object: return LDAP_SCOPE_BASE;
            case SearchScope_Children: return LDAP_SCOPE_ONELEVEL;
            case SearchScope_All: return LDAP_SCOPE_SUBTREE;
            case SearchScope_Descendants: return LDAP_SCOPE_CHILDREN;
        }
        return 0;
    }();

    // Convert attributes list to NULL-terminated array
    char **attributes =
    [attributes_arg]() {
        char **attributes_out;
        if (attributes_arg.isEmpty()) {
            // Pass NULL so LDAP gets all attributes
            attributes_out = NULL;
        } else {
            attributes_out = (char **) malloc((attributes_arg.size() + 1) * sizeof(char *));

            if (attributes_out != NULL) {
                for (int i = 0; i < attributes_arg.size(); i++) {
                    const QString attribute = attributes_arg[i];
                    attributes_out[i] = strdup(cstr(attribute));
                }
                attributes_out[attributes_arg.size()] = NULL;
            }
        }

        return attributes_out;
    }();

    struct berval *cookie = NULL;

    // Search until received all pages
    while (true) {
        if (stop_search_flag) {
            out.clear();
            break;
        }

        const bool search_success = search_paged(ld, filter, attributes, scope, search_base, &out, &cookie);
        if (!search_success) {
            out.clear();
            break;
        }

        const bool more_pages = (cookie != NULL);
        if (more_pages) {
            // NOTE: process events to unfreeze UI during long searches
            QCoreApplication::processEvents();
        } else {
            break;
        }
    };

    if (attributes != NULL) {
        for (int i = 0; attributes[i] != NULL; i++) {
            free(attributes[i]);
        }
        free(attributes);
    }

    return out;
}

AdObject AdInterface::search_object(const QString &dn, const QList<QString> &attributes) {
    const QHash<QString, AdObject> search_results = search("", attributes, SearchScope_Object, dn);

    if (search_results.contains(dn)) {
        return search_results[dn];
    } else {
        return AdObject();
    }
}

bool AdInterface::attribute_replace_values(const QString &dn, const QString &attribute, const QList<QByteArray> &values, const DoStatusMsg do_msg) {
    const AdObject object = search_object(dn, {attribute});
    const QList<QByteArray> old_values = object.get_values(attribute);
    const QString name = dn_get_name(dn);
    const QString values_display = attribute_display_values(attribute, values);
    const QString old_values_display = attribute_display_values(attribute, old_values);

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

    const int result = ldap_modify_ext_s(ld, cstr(dn), attrs, NULL, NULL);

    if (result == LDAP_SUCCESS) {
        success_status_message(QString(tr("Changed attribute \"%1\" of object \"%2\" from \"%3\" to \"%4\"")).arg(attribute, name, old_values_display, values_display), do_msg);

        emit ADSIGNALS()->object_changed(dn);

        return true;
    } else {
        const QString context = QString(tr("Failed to change attribute \"%1\" of object \"%2\" from \"%3\" to \"%4\"")).arg(attribute, name, old_values_display, values_display);

        error_status_message(context, default_error(), do_msg);

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

    const int result = ldap_modify_ext_s(ld, cstr(dn), attrs, NULL, NULL);
    free(data_copy);

    const QString name = dn_get_name(dn);
    const QString new_display_value = attribute_display_value(attribute, value);

    if (result == LDAP_SUCCESS) {
        const QString context = QString(tr("Added value \"%1\" for attribute \"%2\" of object \"%3\"")).arg(new_display_value, attribute, name);

        success_status_message(context, do_msg);

        emit ADSIGNALS()->object_changed(dn);

        return true;
    } else {
        const QString context = QString(tr("Failed to add value \"%1\" for attribute \"%2\" of object \"%3\"")).arg(new_display_value, attribute, name);

        error_status_message(context, default_error(), do_msg);

        return false;
    }
}

bool AdInterface::attribute_delete_value(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg) {
    const QString name = dn_get_name(dn);
    const QString value_display = attribute_display_value(attribute, value);

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

    const int result = ldap_modify_ext_s(ld, cstr(dn), attrs, NULL, NULL);
    free(data_copy);

    if (result == LDAP_SUCCESS) {
        const QString context = QString(tr("Deleted value \"%1\" for attribute \"%2\" of object \"%3\"")).arg(value_display, attribute, name);

        success_status_message(context, do_msg);

        emit ADSIGNALS()->object_changed(dn);

        return true;
    } else {
        const QString context = QString(tr("Failed to delete value \"%1\" for attribute \"%2\" of object \"%3\"")).arg(value_display, attribute, name);

        error_status_message(context, default_error(), do_msg);

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
    const QString datetime_string = datetime_qdatetime_to_string(attribute, datetime);
    const bool result = attribute_replace_string(dn, attribute, datetime_string);

    return result;
}

bool AdInterface::object_add(const QString &dn, const QString &object_class) {
    const char *classes[2] = {cstr(object_class), NULL};

    LDAPMod attr;
    attr.mod_op = LDAP_MOD_ADD;
    attr.mod_type = (char *) "objectClass";
    attr.mod_values = (char **) classes;

    LDAPMod *attrs[] = {&attr, NULL};

    const int result = ldap_add_ext_s(ld, cstr(dn), attrs, NULL, NULL);

    if (result == LDAP_SUCCESS) {
        success_status_message(QString(tr("Created object \"%1\"")).arg(dn));

        emit ADSIGNALS()->object_added(dn);

        return true;
    } else {
        const QString context = QString(tr("Failed to create object \"%1\"")).arg(dn);

        error_status_message(context, default_error());

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

        error_status_message(context, error);
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
    
    result = ldap_delete_ext_s(ld, cstr(dn), server_controls, NULL);

    cleanup();

    if (result == LDAP_SUCCESS) {
        success_status_message(QString(tr("Deleted object \"%1\"")).arg(name));

        emit ADSIGNALS()->object_deleted(dn);

        return true;
    } else {
        error_message(default_error());

        return false;
    }
}

bool AdInterface::object_move(const QString &dn, const QString &new_container) {
    const QString rdn = dn.split(',')[0];
    const QString new_dn = rdn + "," + new_container;
    const QString object_name = dn_get_name(dn);
    const QString container_name = dn_get_name(new_container);

    const int result = ldap_rename_s(ld, cstr(dn), cstr(rdn), cstr(new_container), 1, NULL, NULL);

    if (result == LDAP_SUCCESS) {
        success_status_message(QString(tr("Moved object \"%1\" to \"%2\"")).arg(object_name, container_name));

        emit ADSIGNALS()->object_deleted(dn);
        emit ADSIGNALS()->object_added(new_dn);

        return true;
    } else {
        const QString context = QString(tr("Failed to move object \"%1\" to \"%2\"")).arg(object_name, container_name);

        error_status_message(context, default_error());

        return false;
    }
}

bool AdInterface::object_rename(const QString &dn, const QString &new_name) {
    const QString new_dn = dn_rename(dn, new_name);
    const QString new_rdn = new_dn.split(",")[0];
    const QString old_name = dn_get_name(dn);

    const int result = ldap_rename_s(ld, cstr(dn), cstr(new_rdn), NULL, 1, NULL, NULL);

    if (result == LDAP_SUCCESS) {
        success_status_message(QString(tr("Renamed object \"%1\" to \"%2\"")).arg(old_name, new_name));

        emit ADSIGNALS()->object_deleted(dn);
        emit ADSIGNALS()->object_added(new_dn);

        return true;
    } else {
        const QString context = QString(tr("Failed to rename object \"%1\" to \"%2\"")).arg(old_name, new_name);

        error_status_message(context, default_error());

        return false;
    }
}

bool AdInterface::group_add_member(const QString &group_dn, const QString &user_dn) {
    const QByteArray user_dn_bytes = user_dn.toUtf8();
    const bool success = attribute_add_value(group_dn, ATTRIBUTE_MEMBER, user_dn_bytes, DoStatusMsg_No);

    const QString user_name = dn_get_name(user_dn);
    const QString group_name = dn_get_name(group_dn);
    
    if (success) {
        success_status_message(QString(tr("Added user \"%1\" to group \"%2\"")).arg(user_name, group_name));

        return true;
    } else {
        const QString context = QString(tr("Failed to add user \"%1\" to group \"%2\"")).arg(user_name, group_name);

        error_status_message(context, default_error());

        return false;
    }
}

bool AdInterface::group_remove_member(const QString &group_dn, const QString &user_dn) {
    const QByteArray user_dn_bytes = user_dn.toUtf8();
    const bool success = attribute_delete_value(group_dn, ATTRIBUTE_MEMBER, user_dn_bytes, DoStatusMsg_No);

    const QString user_name = dn_get_name(user_dn);
    const QString group_name = dn_get_name(group_dn);

    if (success) {
        success_status_message(QString(tr("Removed user \"%1\" from group \"%2\"")).arg(user_name, group_name));

        return true;
    } else {
        const QString context = QString(tr("Failed to remove user \"%1\" from group \"%2\"")).arg(user_name, group_name);

        error_status_message(context, default_error());

        return false;
    }
}

// TODO: are there side-effects on group members from this?...
bool AdInterface::group_set_scope(const QString &dn, GroupScope scope) {
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
        success_status_message(QString(tr("Set scope for group \"%1\" to \"%2\"")).arg(name, scope_string));

        return true;
    } else {
        const QString context = QString(tr("Failed to set scope for group \"%1\" to \"%2\"")).arg(name, scope_string);
        
        error_status_message(context, default_error());

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
        success_status_message(QString(tr("Set type for group \"%1\" to \"%2\"")).arg(name, type_string));

        return true;
    } else {
        const QString context = QString(tr("Failed to set type for group \"%1\" to \"%2\"")).arg(name, type_string);
        error_status_message(context, default_error());

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
    const QString group_rid = extract_rid_from_sid(group_sid);

    const bool success = attribute_replace_string(user_dn, ATTRIBUTE_PRIMARY_GROUP_ID, group_rid, DoStatusMsg_No);

    const QString user_name = dn_get_name(user_dn);
    const QString group_name = dn_get_name(group_dn);

    if (success) {
        success_status_message(QString(tr("Set primary group for user \"%1\" to \"%2\"")).arg(user_name, group_name));

        return true;
    } else {
        const QString context = QString(tr("Failed to set primary group for user \"%1\" to \"%2\"")).arg(user_name, group_name);

        error_status_message(context, default_error());

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
    // TODO: gotta be a way to tell codec not to add BOM, but couldn't find it, only QTextStream has setGenerateBOM
    if (password_bytes[0] != '\"') {
        password_bytes.remove(0, 2);
    }

    const bool success = attribute_replace_value(dn, ATTRIBUTE_PASSWORD, password_bytes, DoStatusMsg_No);

    const QString name = dn_get_name(dn);
    
    if (success) {
        success_status_message(QString(tr("Set password for user \"%1\"")).arg(name));

        return true;
    } else {
        const QString context = QString(tr("Failed to set password for user \"%1\"")).arg(name);

        const QString error =
        [this]() {
            const int ldap_result = get_ldap_result();
            if (ldap_result == LDAP_CONSTRAINT_VIOLATION) {
                return tr("Password doesn't match rules");
            } else {
                return default_error();
            }
        }();

        error_status_message(context, error);

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
        
        success_status_message(success_context);

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

        error_status_message(context, default_error());

        return false;
    }
}

bool AdInterface::user_unlock(const QString &dn) {
    const bool result = attribute_replace_string(dn, ATTRIBUTE_LOCKOUT_TIME, LOCKOUT_UNLOCKED_VALUE);

    const QString name = dn_get_name(dn);
    
    if (result) {
        success_status_message(QString(tr("Unlocked user \"%1\"")).arg(name));
        
        return true;
    } else {
        const QString context = QString(tr("Failed to unlock user \"%1\"")).arg(name);

        error_status_message(context, default_error());

        return result;
    }
}

enum DropType {
    DropType_Move,
    DropType_AddToGroup,
    DropType_None
};

// Determine what kind of drop type is dropping this object
// onto target. If drop type is none, then can't drop this
// object on this target.
DropType get_drop_type(const AdObject &dropped, const AdObject &target) {
    if (dropped.get_dn() == target.get_dn()) {
        return DropType_None;
    }

    const bool dropped_is_user = dropped.contains_class(CLASS_USER);
    const bool dropped_is_group = dropped.contains_class(CLASS_GROUP);
    const bool target_is_group = target.contains_class(CLASS_GROUP);

    if (dropped_is_user && target_is_group) {
        return DropType_AddToGroup;
    } else if (dropped_is_group && target_is_group) {
        return DropType_AddToGroup;
    } else {
        const bool system_flags_forbid_move = dropped.get_system_flag(SystemFlagsBit_CannotMove);

        const QList<QString> dropped_classes = dropped.get_strings(ATTRIBUTE_OBJECT_CLASS);
        const QList<QString> dropped_superiors = ADCONFIG()->get_possible_superiors(dropped_classes);

        const bool target_is_valid_superior =
        [target, dropped_superiors]() {
            const QList<QString> target_classes = target.get_strings(ATTRIBUTE_OBJECT_CLASS);
            for (const auto &object_class : dropped_superiors) {
                if (target_classes.contains(object_class)) {
                    return true;
                }
            }

            return false;
        }();

        if (system_flags_forbid_move) {
            return DropType_None;
        } else if (target_is_valid_superior) {
            return DropType_Move;
        } else {
            return DropType_None;
        }
    }
}

bool object_can_drop(const AdObject &dropped, const AdObject &target) {
    const DropType drop_type = get_drop_type(dropped, target);

    if (drop_type == DropType_None) {
        return false;
    } else {
        return true;
    }
}

// General "drop" operation that can either move, link or change membership depending on which types of objects are involved
void AdInterface::object_drop(const AdObject &dropped, const AdObject &target) {
    DropType drop_type = get_drop_type(dropped, target);

    switch (drop_type) {
        case DropType_Move: {
            object_move(dropped.get_dn(), target.get_dn());
            break;
        }
        case DropType_AddToGroup: {
            group_add_member(target.get_dn(), dropped.get_dn());
            break;
        }
        case DropType_None: {
            break;
        }
    }
}

bool AdInterface::create_gpo(const QString &display_name) {
    //
    // Generate UUID used for directory and object names
    //
    // TODO: is it ok to not swap bytes here, like it's done in octet_display_value()? Probably yes.
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
    const QString main_dir = QString("smb://%1/sysvol/%2/Policies/%3").arg(host(), domain().toLower(), uuid);
    const int result_mkdir_main = smbc_mkdir(cstr(main_dir), 0);
    if (result_mkdir_main != 0) {
        // TODO: handle errors
        return false;
    }

    const QString machine_dir = main_dir + "/Machine";
    const int result_mkdir_machine = smbc_mkdir(cstr(machine_dir), 0);
    if (result_mkdir_machine != 0) {
        // TODO: handle errors
        return false;
    }

    const QString user_dir = main_dir + "/User";
    const int result_mkdir_user = smbc_mkdir(cstr(user_dir), 0);
    if (result_mkdir_user != 0) {
        // TODO: handle errors
        return false;
    }

    const QString init_file_path = main_dir + "/GPT.INI";
    const int ini_file = smbc_open(cstr(init_file_path), O_WRONLY | O_CREAT, 0);

    const char *ini_contents = "[General]\r\nVersion=0\r\n";
    const int result_write_ini = smbc_write(ini_file, ini_contents, strlen(ini_contents));
    if (result_write_ini < 0) {
        // TODO: handle errors
        return false;
    }

    //
    // Create AD object for gpo
    //
    // TODO: add all attributes during creation, need to directly create through ldap then
    const QString dn = QString("CN=%1,CN=Policies,CN=System,%2").arg(uuid, m_domain_head);
    const bool result_add = object_add(dn, CLASS_GP_CONTAINER);
    if (!result_add) {
        return false;
    }
    attribute_replace_string(dn, ATTRIBUTE_DISPLAY_NAME, display_name);
    // "\\domain.alt\sysvol\domain.alt\Policies\{FF7E0880-F3AD-4540-8F1D-4472CB4A7044}"
    const QString gPCFileSysPath = QString("\\\\%1\\sysvol\\%2\\Policies\\%3").arg(domain().toLower(), uuid);
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
        return false;
    }

    // Machine object
    const QString machine_dn = "CN=Machine," + dn;
    const bool result_add_machine = object_add(machine_dn, CLASS_CONTAINER);
    attribute_replace_string(dn, ATTRIBUTE_SHOW_IN_ADVANCED_VIEW_ONLY, "TRUE");
    if (!result_add_machine) {
        return false;
    }

    // TODO: security descriptor. Samba does this: get nTSecurityDescriptor attribute from the gpo object as raw bytes, then sets the security descriptor of the sysvol dir to that value. Currently not doing that, so security descriptor of sysvol dir is the default one, which is ... bad? Not sure. Problem is, libsmbclient only provides a way to set security descriptor using key/value strings, not the raw bytes basically. So to do it that way would need to decode object security descriptor. Samba source has that implemented internally but not exposed as a usable lib. Probably SHOULD set security descriptor because who knows what the default is, what if it's just randomly modifiable by any user. Also, the security descriptor functionality will be needed for "Security" tab in ADMC at some point. So should either implement all of that stuff (it's a lot...) or hopefully find some lib to use (unlikely). On Windows you would just use Microsoft's lib.

    return true;
}

bool AdInterface::delete_gpo(const QString &gpo_dn) {
    // NOTE: can't delete object's while they have children so have to clean up recursively
    QList<QString> delete_queue;
    QList<QString> recurse_stack;
    recurse_stack.append(gpo_dn);
    while (!recurse_stack.isEmpty()) {
        const QString dn = recurse_stack.last();
        recurse_stack.removeLast();
        delete_queue.insert(0, dn);

        const QHash<QString, AdObject> children = search("", {}, SearchScope_Children, dn);
        recurse_stack.append(children.keys());
    }

    for (const auto &dn : delete_queue) {
        object_delete(dn);
    }

    const AdObject object = search_object(gpo_dn, {ATTRIBUTE_GPC_FILE_SYS_PATH});
    const QString sysvol_path = object.get_string(ATTRIBUTE_GPC_FILE_SYS_PATH);
    const QString url = sysvol_path_to_smb(sysvol_path);
    const int result_rmdir = smbc_rmdir(cstr(url));
    if (result_rmdir < 0) {
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

    out = QString("smb://%1/%2").arg(m_host, out);

    return out;
}

void AdInterface::success_status_message(const QString &msg, const DoStatusMsg do_msg) {
    if (do_msg == DoStatusMsg_No) {
        return;
    }

    Status::instance()->message(msg, StatusType_Success);
}

void AdInterface::error_status_message(const QString &context, const QString &error, const DoStatusMsg do_msg) {
    if (do_msg == DoStatusMsg_No) {
        return;
    }

    QString msg = context;
    if (!error.isEmpty()) {
        msg += QString(tr(". Error: \"%1\"")).arg(error);;
    }

    Status::instance()->message(msg, StatusType_Error);
}

QString AdInterface::default_error() const {
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

int AdInterface::get_ldap_result() const {
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
 * dname is a combination of protocols (ldap, tcp), domain and site
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
        // TODO: need to save port field? maybe to incorporate into uri

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

bool ad_is_connected(const AdInterface &ad) {
    if (!ad.is_connected()) {
        const QString title = QObject::tr("Connection error");
        const QString text = QObject::tr("Failed to connect to server.");

        // TODO: would want a valid parent widget for
        // message box but ad_is_connected() can be called
        // from places where there isn't one available,
        // console_drag_model for example. Good news is that
        // the messagebox appears to be modal even without a
        // parent.
        QMessageBox::critical(nullptr, title, text);
    }

    return ad.is_connected();
}
