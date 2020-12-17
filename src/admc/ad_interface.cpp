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
#include "active_directory.h"
#include "ad_utils.h"
#include "ad_config.h"
#include "ad_object.h"
#include "attribute_display.h"
#include "status.h"
#include "utils.h"

#include <ldap.h>
#include <lber.h>
#include <libsmbclient.h>
#include <uuid/uuid.h>
#include <krb5.h>

#include <QTextCodec>
#include <QDebug>
#include <QCoreApplication>

QList<QString> get_domain_hosts(const QString &domain, const QString &site);

AdInterface *AdInterface::instance() {
    static AdInterface ad_interface;
    return &ad_interface;
}

void get_auth_data_fn(const char * pServer, const char * pShare, char * pWorkgroup, int maxLenWorkgroup, char * pUsername, int maxLenUsername, char * pPassword, int maxLenPassword) {

}

bool AdInterface::connect() {
    // Get default domain from krb5
    const QString domain =
    []() {
        krb5_error_code result;
        krb5_context context;

        result = krb5_init_context(&context);
        if (result) {
            qDebug() << "Failed to init krb5 context";
            return QString();
        }

        char *realm_cstr = NULL;
        result = krb5_get_default_realm(context, &realm_cstr);
        if (result) {
            qDebug() << "Failed to get default realm";

            krb5_free_context(context);

            return QString();
        }

        const QString out = QString(realm_cstr);

        krb5_free_default_realm(context, realm_cstr);

        return out;
    }();

    qDebug() << "domain=" << domain;

    const QList<QString> hosts = get_domain_hosts(domain, QString());
    if (hosts.isEmpty()) {
        qDebug() << "No hosts found";

        error_status_message(tr("Failed to connect"), tr("Not connected to a domain network"));

        return false;
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

    const int result = ad_connect(cstr(uri), &ld);

    if (result == AD_SUCCESS) {
        m_config = new AdConfig(this);

        // TODO: can this context expire, for example from a disconnect?
        smbc_init(get_auth_data_fn, 0);
        smbc = smbc_new_context();
        smbc_setOptionUseKerberos(smbc, true);
        smbc_setOptionFallbackAfterKerberos(smbc, true);
        if (!smbc_init_context(smbc)) {
            smbc_free_context(smbc, 0);
            printf("Could not initialize smbc context\n");
        }
        smbc_set_context(smbc);

        emit connected();

        return true;
    } else {
        error_status_message(tr("Failed to connect"), tr("Authentication failed"));

        return false;
    }
}

AdConfig *AdInterface::config() const {
    return m_config;
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

QHash<QString, AdObject> AdInterface::search(const QString &filter, const QList<QString> &attributes, const SearchScope scope_enum, const QString &search_base) {
    QHash<QString, AdObject> out;

    const QString base =
    [this, search_base]() {
        if (search_base.isEmpty()) {
            return m_domain_head;
        } else {
            return search_base;
        }
    }();

    const char *filter_cstr =
    [filter]() {
        if (filter.isEmpty()) {
            return (const char *) NULL;
        } else {
            return cstr(filter);
        }
    }();

    const int scope =
    [scope_enum]() {
        switch (scope_enum) {
            case SearchScope_Object: return LDAP_SCOPE_BASE;
            case SearchScope_Children: return LDAP_SCOPE_ONELEVEL;
            case SearchScope_All: return LDAP_SCOPE_SUBTREE;
            case SearchScope_Descendants: return LDAP_SCOPE_CHILDREN;
        }
        return 0;
    }();

    // Convert attributes list to NULL-terminated array
    char **attrs =
    [attributes]() {
        char **attrs_out;
        if (attributes.isEmpty()) {
            // Pass NULL so LDAP gets all attributes
            attrs_out = NULL;
        } else {
            attrs_out = (char **) malloc((attributes.size() + 1) * sizeof(char *));
            for (int i = 0; i < attributes.size(); i++) {
                const QString attribute = attributes[i];
                attrs_out[i] = strdup(cstr(attribute));
            }
            attrs_out[attributes.size()] = NULL;
        }

        return attrs_out;
    }();

    struct berval *prev_cookie = NULL;

    int result;

    // Search until received all pages
    while (true) {
        // Create page control
        LDAPControl *page_control = NULL;
        const ber_int_t page_size = 1000;
        const int is_critical = 1;
        result = ldap_create_page_control(ld, page_size, prev_cookie, is_critical, &page_control);
        if (result != LDAP_SUCCESS) {
            qDebug() << "Failed to create page control: " << ldap_err2string(result);
            break;
        }
        LDAPControl *server_controls[2] = {page_control, NULL};
        
        // Perform search
        LDAPMessage *res;
        const int attrsonly = 0;
        result = ldap_search_ext_s(ld, cstr(base), scope, filter_cstr, attrs, attrsonly, server_controls, NULL, NULL, LDAP_NO_LIMIT, &res);
        if ((result != LDAP_SUCCESS) && (result != LDAP_PARTIAL_RESULTS)) {
            qDebug() << "Error in paged ldap_search_ext_s: " << ldap_err2string(result);
            break;
        }

        ldap_control_free(server_controls[0]);

        // Collect results for this search
        for (LDAPMessage *entry = ldap_first_entry(ld, res); entry != NULL; entry = ldap_next_entry(ld, entry)) {
            char *dn_cstr = ldap_get_dn(ld, entry);
            const QString dn(dn_cstr);
            ldap_memfree(dn_cstr);

            AdObjectAttributes object_attributes;

            BerElement *berptr;
            for (char *attr = ldap_first_attribute(ld, entry, &berptr); attr != NULL; attr = ldap_next_attribute(ld, entry, berptr)) {
                struct berval **values_ldap = ldap_get_values_len(ld, entry, attr);
                if (values_ldap == NULL) {
                    ldap_value_free_len(values_ldap);

                    continue;
                }

                const QList<QByteArray> values_bytes =
                [=]() {
                    QList<QByteArray> values_bytes_out;

                    const int values_count = ldap_count_values_len(values_ldap);
                    for (int i = 0; i < values_count; i++) {
                        struct berval value_berval = *values_ldap[i];
                        const QByteArray value_bytes(value_berval.bv_val, value_berval.bv_len);

                        values_bytes_out.append(value_bytes);
                    }

                    return values_bytes_out;
                }();

                const QString attribute(attr);

                object_attributes[attribute] = values_bytes;

                ldap_value_free_len(values_ldap);
                ldap_memfree(attr);
            }
            ber_free(berptr, 0);

            out[dn] = AdObject(dn, object_attributes);
        }

        // Parse the results to retrieve returned controls
        int errcodep;
        LDAPControl **returned_controls = NULL;
        result = ldap_parse_result(ld, res, &errcodep, NULL, NULL, NULL, &returned_controls, false);
        if (result != LDAP_SUCCESS) {
            qDebug() << "Failed to parse result: " << ldap_err2string(result);
            break;
        }

        // Get page response control
        LDAPControl *pageresponse_control = ldap_control_find(LDAP_CONTROL_PAGEDRESULTS, returned_controls, NULL);
        if (pageresponse_control == NULL) {
            qDebug() << "Failed to find PAGEDRESULTS control";
            break;
        }

        // Parse page response control to determine whether
        // there are more pages
        struct berval new_cookie;
        ber_int_t total_count;
        result = ldap_parse_pageresponse_control(ld, pageresponse_control, &total_count, &new_cookie);
        if (result != LDAP_SUCCESS) {
            qDebug() << "Failed to parse pageresponse control: " << ldap_err2string(result);
            break;
        }
        ber_bvfree(prev_cookie);
        prev_cookie = ber_bvdup(&new_cookie);

        ldap_controls_free(returned_controls);
        returned_controls = NULL;

        ldap_msgfree(res);

        // There are more pages if the cookie is not empty
        const bool more_pages = (prev_cookie->bv_len > 0);
        if (more_pages) {
            // NOTE: process events to unfreeze UI during long searches
            QCoreApplication::processEvents();
        } else {
            break;
        }
    };

    ad_array_free(attrs);

    ber_bvfree(prev_cookie);

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
    int result = AD_SUCCESS;
    
    const AdObject object = search_object(dn, {attribute});
    const QList<QByteArray> old_values = object.get_values(attribute);

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

    const int result_modify = ldap_modify_ext_s(ld, cstr(dn), attrs, NULL, NULL);
    if (result_modify != LDAP_SUCCESS) {
        result = AD_LDAP_ERROR;
    }

    const QString name = dn_get_name(dn);
    const QString values_display = attribute_display_values(attribute, values);
    const QString old_values_display = attribute_display_values(attribute, old_values);

    if (result == AD_SUCCESS) {
        success_status_message(QString(tr("Changed attribute \"%1\" of object \"%2\" from \"%3\" to \"%4\"")).arg(attribute, name, old_values_display, values_display), do_msg);

        emit object_changed(dn);

        return true;
    } else {
        const QString context = QString(tr("Failed to change attribute \"%1\" of object \"%2\" from \"%3\" to \"%4\"")).arg(attribute, name, old_values_display, values_display);
        const QString error = default_error();

        error_status_message(context, error, do_msg);

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
    const int result = ad_attribute_add_value(ld, cstr(dn), cstr(attribute), value.constData(), value.size());

    const QString name = dn_get_name(dn);

    const QString new_display_value = attribute_display_value(attribute, value);
    ;

    if (result == AD_SUCCESS) {
        const QString context = QString(tr("Added value \"%1\" for attribute \"%2\" of object \"%3\"")).arg(new_display_value, attribute, name);

        success_status_message(context, do_msg);

        emit object_changed(dn);

        return true;
    } else {
        const QString context = QString(tr("Failed to add value \"%1\" for attribute \"%2\" of object \"%3\"")).arg(new_display_value, attribute, name);
        const QString error = default_error();

        error_status_message(context, error, do_msg);

        return false;
    }
}

bool AdInterface::attribute_delete_value(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg) {
    const int result = ad_attribute_delete_value(ld, cstr(dn), cstr(attribute), value.constData(), value.size());

    const QString name = dn_get_name(dn);

    const QString value_display = attribute_display_value(attribute, value);

    if (result == AD_SUCCESS) {
        const QString context = QString(tr("Deleted value \"%1\" for attribute \"%2\" of object \"%3\"")).arg(value_display, attribute, name);

        success_status_message(context, do_msg);

        emit object_changed(dn);

        return true;
    } else {
        const QString context = QString(tr("Failed to delete value \"%1\" for attribute \"%2\" of object \"%3\"")).arg(value_display, attribute, name);
        const QString error = default_error();

        error_status_message(context, error, do_msg);

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

    const int result = ad_add(ld, cstr(dn), classes);

    if (result == AD_SUCCESS) {
        success_status_message(QString(tr("Created object \"%1\"")).arg(dn));

        emit object_added(dn);

        return true;
    } else {
        const QString context = QString(tr("Failed to create object \"%1\"")).arg(dn);
        const QString error = default_error();

        error_status_message(context, error);

        return false;
    }
}

bool AdInterface::object_delete(const QString &dn) {
    int result = ad_delete(ld, cstr(dn));

    const QString name = dn_get_name(dn);
    
    if (result == AD_SUCCESS) {
        success_status_message(QString(tr("Deleted object \"%1\"")).arg(name));

        emit object_deleted(dn);

        return true;
    } else {
        const QString context = QString(tr("Failed to delete object \"%1\"")).arg(name);
        const QString error = default_error();

        error_status_message(context, error);

        return false;
    }
}

bool AdInterface::object_move(const QString &dn, const QString &new_container) {
    QList<QString> dn_split = dn.split(',');
    QString new_dn = dn_split[0] + "," + new_container;

    const int result = ad_move(ld, cstr(dn), cstr(new_container));

    // TODO: drag and drop handles checking move compatibility but need
    // to do this here as well for CLI?
    
    const QString object_name = dn_get_name(dn);
    const QString container_name = dn_get_name(new_container);

    if (result == AD_SUCCESS) {
        success_status_message(QString(tr("Moved object \"%1\" to \"%2\"")).arg(object_name, container_name));

        emit object_deleted(dn);
        emit object_added(new_dn);

        return true;
    } else {
        const QString context = QString(tr("Failed to move object \"%1\" to \"%2\"")).arg(object_name, container_name);
        const QString error = default_error();

        error_status_message(context, error);

        return false;
    }
}

bool AdInterface::object_rename(const QString &dn, const QString &new_name) {
    const QString new_dn = dn_rename(dn, new_name);
    const QString new_rdn = new_dn.split(",")[0];

    int result = ad_rename(ld, cstr(dn), cstr(new_rdn));

    const QString old_name = dn_get_name(dn);

    if (result == AD_SUCCESS) {
        success_status_message(QString(tr("Renamed object \"%1\" to \"%2\"")).arg(old_name, new_name));

        emit object_deleted(dn);
        emit object_added(new_dn);

        return true;
    } else {
        const QString context = QString(tr("Failed to rename object \"%1\" to \"%2\"")).arg(old_name, new_name);
        const QString error = default_error();

        error_status_message(context, error);

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

        error_status_message(context, "");

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

        error_status_message(context, "");

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
        error_status_message(context, "");

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
        error_status_message(context, "");

        return false;
    }
}


bool AdInterface::user_set_primary_group(const QString &group_dn, const QString &user_dn) {
    const AdObject group_object = AD()->search_object(group_dn, {ATTRIBUTE_OBJECT_SID, ATTRIBUTE_MEMBER});

    // NOTE: need to add user to group before it can become primary
    const QList<QString> group_members = group_object.get_strings(ATTRIBUTE_MEMBER);
    const bool user_is_in_group = group_members.contains(user_dn);
    if (!user_is_in_group) {
        group_add_member(group_dn, user_dn);
    }

    const QByteArray group_sid = group_object.get_value(ATTRIBUTE_OBJECT_SID);
    const QString group_rid = extract_rid_from_sid(group_sid);

    const bool success = AD()->attribute_replace_string(user_dn, ATTRIBUTE_PRIMARY_GROUP_ID, group_rid, DoStatusMsg_No);

    const QString user_name = dn_get_name(user_dn);
    const QString group_name = dn_get_name(group_dn);

    if (success) {
        success_status_message(QString(tr("Set primary group for user \"%1\" to \"%2\"")).arg(user_name, group_name));

        return true;
    } else {
        const QString context = QString(tr("Failed to set primary group for user \"%1\" to \"%2\"")).arg(user_name, group_name);

        error_status_message(context, "");

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
            const int ldap_result = ad_get_ldap_result(ld);
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

        error_status_message(context, "");

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

        error_status_message(context, "");

        return result;
    }
}

enum DropType {
    DropType_Move,
    DropType_AddToGroup,
    DropType_None
};

// Determine what kind of drop type is dropping this object onto target
// If drop type is none, then can't drop this object on this target
DropType get_drop_type(const QString &dn, const QString &target_dn) {
    if (dn == target_dn) {
        return DropType_None;
    }

    const AdObject dropped = AD()->search_object(dn, {ATTRIBUTE_OBJECT_CLASS});
    const AdObject target = AD()->search_object(target_dn, {ATTRIBUTE_OBJECT_CLASS});

    const bool dropped_is_user = dropped.is_class(CLASS_USER);
    const bool target_is_group = target.is_class(CLASS_USER);

    if (dropped_is_user && target_is_group) {
        return DropType_AddToGroup;
    } else {
        const QList<QString> dropped_classes = dropped.get_strings(ATTRIBUTE_OBJECT_CLASS);
        const QList<QString> dropped_superiors = ADCONFIG()->get_possible_superiors(dropped_classes);

        const bool can_move =
        [target, dropped_superiors]() {
            const QList<QString> target_classes = target.get_strings(ATTRIBUTE_OBJECT_CLASS);
            for (const auto object_class : dropped_superiors) {
                if (target_classes.contains(object_class)) {
                    return true;
                }
            }

            return false;
        }();

        if (can_move) {
            return DropType_Move;
        } else {
            return DropType_None;
        }
    }
}

bool AdInterface::object_can_drop(const QString &dn, const QString &target_dn) {
    const DropType drop_type = get_drop_type(dn, target_dn);

    if (drop_type == DropType_None) {
        return false;
    } else {
        return true;
    }
}

// General "drop" operation that can either move, link or change membership depending on which types of objects are involved
void AdInterface::object_drop(const QString &dn, const QString &target_dn) {
    DropType drop_type = get_drop_type(dn, target_dn);

    switch (drop_type) {
        case DropType_Move: {
            AD()->object_move(dn, target_dn);
            break;
        }
        case DropType_AddToGroup: {
            AD()->group_add_member(target_dn, dn);
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
    const int result_write_ini = smbc_write(ini_file, ini_contents, sizeof(ini_contents) / sizeof(char));
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

    // TODO: security descriptor. Samba does this: get nTSecurityDescriptor attribute from the gpo object as raw bytes, then sets the security descriptor of the sysvol dir to that value. Currently not doing that, so security descriptor of sysvol dir is the default one, which is ... bad? Not sure. Problem is, libsmbclient only provides a way to set security descriptor using key/value strings, not the raw bytes basically. So to do it that way would need to decode object security descriptor. Samba source has that implemented internally but not exposed as a usable lib. Probably SHOULD set security descriptor because who knows what the default is, what if it's just randomly modifiable by any user. Also, the security descriptor functionality will be needed for "Security" details tab in ADMC at some point. So should either implement all of that stuff (it's a lot...) or hopefully find some lib to use (unlikely). On Windows you would just use Microsoft's lib.

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

        const QHash<QString, AdObject> children = AD()->search("", {}, SearchScope_Children, dn);
        recurse_stack.append(children.keys());
    }

    for (const auto dn : delete_queue) {
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

AdInterface::AdInterface()
: QObject()
{

}

void AdInterface::success_status_message(const QString &msg, const DoStatusMsg do_msg) {
    if (do_msg == DoStatusMsg_No) {
        return;
    }

    STATUS()->message(msg, StatusType_Success);
}

void AdInterface::error_status_message(const QString &context, const QString &error, const DoStatusMsg do_msg) {
    if (do_msg == DoStatusMsg_No) {
        return;
    }

    QString msg = context;
    if (!error.isEmpty()) {
        msg += QString(tr(". Error: \"%1\"")).arg(error);;
    }

    STATUS()->message(msg, StatusType_Error);
}

QString AdInterface::default_error() const {
    const int ldap_result = ad_get_ldap_result(ld);
    switch (ldap_result) {
        case LDAP_NO_SUCH_OBJECT: return tr("No such object");
        case LDAP_CONSTRAINT_VIOLATION: return tr("Constraint violation");
        case LDAP_UNWILLING_TO_PERFORM: return tr("Server is unwilling to perform");
        case LDAP_ALREADY_EXISTS: return tr("Already exists");
        default: {
            char *ldap_err = ldap_err2string(ldap_result);
            const QString ldap_err_qstr(ldap_err);
            return QString(tr("LDAP error: %1")).arg(ldap_err_qstr);
        }
    }
}

AdInterface *AD() {
    return AdInterface::instance();
}

QList<QString> get_domain_hosts(const QString &domain, const QString &site) {
    char **hosts_raw = NULL;
    int hosts_result = ad_get_domain_hosts(cstr(domain), cstr(site), &hosts_raw);

    if (hosts_result == AD_SUCCESS) {
        auto hosts = QList<QString>();

        for (int i = 0; hosts_raw[i] != NULL; i++) {
            auto host = QString(hosts_raw[i]);
            hosts.append(host);
        }
        ad_array_free(hosts_raw);

        return hosts;
    } else {
        return QList<QString>();
    }
}
