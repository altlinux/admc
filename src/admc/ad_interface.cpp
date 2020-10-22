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
#include "status.h"
#include "utils.h"
#include "ad_config.h"
#include "ad_object.h"
#include "attribute_display.h"

#include <ldap.h>
#include <lber.h>
#include <libsmbclient.h>
#include <uuid/uuid.h>

#include <QSet>
#include <QMessageBox>
#include <QTextCodec>
#include <QDebug>

// TODO: confirm these are fine. I think need to make seconds option for UTC? Don't know if QDatetime can handle that.
#define GENERALIZED_TIME_FORMAT_STRING "yyyyMMddhhmmss.zZ"
#define UTC_TIME_FORMAT_STRING "yyMMddhhmmss.zZ"

#define LDAP_SEARCH_NO_ATTRIBUTES "1.1"

QList<QString> get_domain_hosts(const QString &domain, const QString &site);

AdInterface *AdInterface::instance() {
    static AdInterface ad_interface;
    return &ad_interface;
}

void get_auth_data_fn(const char * pServer, const char * pShare, char * pWorkgroup, int maxLenWorkgroup, char * pUsername, int maxLenUsername, char * pPassword, int maxLenPassword) {

}

bool AdInterface::login(const QString &domain, const QString &site) {
    const QList<QString> hosts = get_domain_hosts(domain, site);
    if (hosts.isEmpty()) {
        return false;
    }

    // TODO: for now selecting first host, which seems to be fine but investigate what should be selected.
    m_host = hosts[0];

    const QString uri = "ldap://" + m_host;

    m_domain = domain;

    // Transform domain to search base
    // "DOMAIN.COM" => "DC=domain,DC=com"
    m_search_base = m_domain;
    m_search_base = m_search_base.toLower();
    m_search_base = "DC=" + m_search_base;
    m_search_base = m_search_base.replace(".", ",DC=");

    m_configuration_dn = "CN=Configuration," + m_search_base;
    m_schema_dn = "CN=Schema," + m_configuration_dn;

    const QByteArray uri_array = uri.toUtf8();
    const char *uri_cstr = uri_array.constData();

    const int result = ad_login(uri_cstr, &ld);

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

        return true;
    } else {
        return false;
    }
}

void AdInterface::start_batch() {
    if (batch_in_progress) {
        printf("Called start_batch() while batch is in progress!\n");
        return;
    }

    batch_in_progress = true;
}

void AdInterface::end_batch() {
    if (!batch_in_progress) {
        printf("Called end_batch() before start_batch()!\n");
        return;
    }

    batch_in_progress = false;

    emit_modified();
}

AdConfig *AdInterface::config() const {
    return m_config;
}

QString AdInterface::domain() const {
    return m_domain;
}

QString AdInterface::search_base() const {
    return m_search_base;
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

QHash<QString, AdObject> AdInterface::search(const QString &filter, const QList<QString> &attributes, const SearchScope scope_enum, const QString &custom_search_base) {
    // int result = AD_SUCCESS;
    QHash<QString, AdObject> out;

    LDAPMessage *res;
    char **attrs;


    const QString base =
    [this, custom_search_base]() {
        if (custom_search_base.isEmpty()) {
            return m_search_base;
        } else {
            return custom_search_base;
        }
    }();

    const QByteArray filter_array = filter.toUtf8();
    const char *filter_cstr =
    [filter_array]() {
        if (filter_array.isEmpty()) {
            return (const char *) NULL;
        } else {
            return filter_array.constData();
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

    const QByteArray base_array = base.toUtf8();
    const char *base_cstr = base_array.constData();

    // Convert attributes list to NULL-terminated array
    attrs =
    [attributes]() {
        char **attrs_out;
        if (attributes.contains(SEARCH_ALL_ATTRIBUTES)) {
            // TODO: should do this better but kinda hard but not sure how to do this with as little details leaking out, like making caller put in LDAP_SEARCH_NO_ATTRIBUTES into QList is no good.
            attrs_out = NULL;
        } else if (attributes.isEmpty()) {
            attrs_out = (char **) malloc(2 * sizeof(char *));
            attrs_out[0] = strdup(LDAP_SEARCH_NO_ATTRIBUTES);
            attrs_out[1] = NULL;
        } else {
            attrs_out = (char **) malloc((attributes.size() + 1) * sizeof(char *));
            for (int i = 0; i < attributes.size(); i++) {
                const QString attribute = attributes[i];
                const QByteArray attribute_array = attribute.toUtf8();
                const char *attribute_cstr = attribute_array.constData();
                attrs_out[i] = strdup(attribute_cstr);
            }
            attrs_out[attributes.size()] = NULL;
        }

        return attrs_out;
    }();

    const int result_search = ldap_search_ext_s(ld, base_cstr, scope, filter_cstr, attrs, 0, NULL, NULL, NULL, LDAP_NO_LIMIT, &res);
    if (result_search != LDAP_SUCCESS) {
        // result = result_search;

        goto end;
    }

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

    end: {
        ldap_msgfree(res);
        ad_array_free(attrs);


        return out;
    }
}

AdObject AdInterface::search_object(const QString &dn, const QList<QString> &attributes) {
    const QHash<QString, AdObject> search_results = search("", attributes, SearchScope_Object, dn);

    if (search_results.contains(dn)) {
        return search_results[dn];
    } else {
        return AdObject();
    }
}

bool AdInterface::attribute_add_string(const QString &dn, const QString &attribute, const QString &value, const DoStatusMsg do_msg) {
    const QByteArray value_bytes = value.toUtf8();
    
    return attribute_add(dn, attribute, value_bytes, do_msg);
}

bool AdInterface::attribute_replace_string(const QString &dn, const QString &attribute, const QString &value, const DoStatusMsg do_msg) {
    const QByteArray value_bytes = value.toUtf8();

    return attribute_replace_value(dn, attribute, value_bytes, do_msg);
}

bool AdInterface::attribute_delete_string(const QString &dn, const QString &attribute, const QString &value, const DoStatusMsg do_msg) {
    const QByteArray value_bytes = value.toUtf8();
    
    return attribute_delete_value(dn, attribute, value_bytes, do_msg);
}

bool AdInterface::attribute_add(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg) {
    const QByteArray dn_array = dn.toUtf8();
    const char *dn_cstr = dn_array.constData();

    const QByteArray attribute_array = attribute.toUtf8();
    const char *attribute_cstr = attribute_array.constData();

    const char *value_cstr = value.constData();

    const int result = ad_attribute_add(ld, dn_cstr, attribute_cstr, value_cstr, value.size());

    const QString name = dn_get_rdn(dn);

    const QString new_display_value = attribute_display_value(attribute, value);
    ;

    if (result == AD_SUCCESS) {
        const QString context = QString(tr("Added value \"%1\" for attribute \"%2\" of object \"%3\"")).arg(new_display_value, attribute, name);

        success_status_message(context, do_msg);

        emit_modified();

        return true;
    } else {
        const QString context = QString(tr("Failed to add value \"%1\" for attribute \"%2\" of object \"%3\"")).arg(new_display_value, attribute, name);
        const QString error = default_error();

        error_status_message(context, error, do_msg);

        return false;
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

    const QByteArray dn_array = dn.toUtf8();
    const char *dn_cstr = dn_array.constData();

    const QByteArray attribute_array = attribute.toUtf8();
    const char *attribute_cstr = attribute_array.constData();

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
    attr.mod_type = (char *)attribute_cstr;
    attr.mod_bvalues = bvalues;
    
    LDAPMod *attrs[] = {&attr, NULL};

    const int result_modify = ldap_modify_ext_s(ld, dn_cstr, attrs, NULL, NULL);
    if (result_modify != LDAP_SUCCESS) {
        result = AD_LDAP_ERROR;
    }

    const QString name = dn_get_rdn(dn);
    const QString values_display = attribute_display_values(attribute, values);
    const QString old_values_display = attribute_display_values(attribute, old_values);

    if (result == AD_SUCCESS) {
        success_status_message(QString(tr("Changed attribute \"%1\" of \"%2\" from \"%3\" to \"%4\"")).arg(attribute, name, old_values_display, values_display), do_msg);

        emit_modified();

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

bool AdInterface::attribute_delete_value(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg) {
    const QByteArray dn_array = dn.toUtf8();
    const char *dn_cstr = dn_array.constData();

    const QByteArray attribute_array = attribute.toUtf8();
    const char *attribute_cstr = attribute_array.constData();

    const char *value_cstr = value.constData();

    const int result = ad_attribute_delete_value(ld, dn_cstr, attribute_cstr, value_cstr, value.size());

    const QString name = dn_get_rdn(dn);

    const QString value_display = attribute_display_value(attribute, value);

    if (result == AD_SUCCESS) {
        const QString context = QString(tr("Deleted value \"%1\" for attribute \"%2\" of object \"%3\"")).arg(value_display, attribute, name);

        success_status_message(context, do_msg);

        emit_modified();

        return true;
    } else {
        const QString context = QString(tr("Failed to delete value \"%1\" for attribute \"%2\" of object \"%3\"")).arg(value_display, attribute, name);
        const QString error = default_error();

        error_status_message(context, error, do_msg);

        return false;
    }
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

bool AdInterface::object_add(const QString &dn, const char **classes) {
    const QByteArray dn_array = dn.toUtf8();
    const char *dn_cstr = dn_array.constData();

    const int result = ad_add(ld, dn_cstr, classes);

    if (result == AD_SUCCESS) {
        success_status_message(QString(tr("Created \"%1\"")).arg(dn));

        emit_modified();

        return true;
    } else {
        const QString context = QString(tr("Failed to create \"%1\"")).arg(dn);
        const QString error = default_error();

        error_status_message(context, error);

        return false;
    }
}

bool AdInterface::object_delete(const QString &dn) {
    const QByteArray dn_array = dn.toUtf8();
    const char *dn_cstr = dn_array.constData();

    int result = ad_delete(ld, dn_cstr);

    const QString name = dn_get_rdn(dn);
    
    if (result == AD_SUCCESS) {
        success_status_message(QString(tr("Deleted object \"%1\"")).arg(name));

        emit_modified();

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

    const QByteArray dn_array = dn.toUtf8();
    const char *dn_cstr = dn_array.constData();

    const QByteArray new_container_array = new_container.toUtf8();
    const char *new_container_cstr = new_container_array.constData();

    const int result = ad_move(ld, dn_cstr, new_container_cstr);

    // TODO: drag and drop handles checking move compatibility but need
    // to do this here as well for CLI?
    
    const QString object_name = dn_get_rdn(dn);
    const QString container_name = dn_get_rdn(new_container);

    if (result == AD_SUCCESS) {
        success_status_message(QString(tr("Moved \"%1\" to \"%2\"")).arg(object_name, container_name));

        emit_modified();

        return true;
    } else {
        const QString context = QString(tr("Failed to move \"%1\" to \"%2\"")).arg(object_name, container_name);
        const QString error = default_error();

        error_status_message(context, error);

        return false;
    }
}

bool AdInterface::object_rename(const QString &dn, const QString &new_name) {
    // Compose new_rdn and new_dn
    const QStringList exploded_dn = dn.split(',');
    const QString old_rdn = exploded_dn[0];
    const int prefix_i = old_rdn.indexOf('=') + 1;
    const QString prefix = old_rdn.left(prefix_i);
    const QString new_rdn = prefix + new_name;
    QStringList new_exploded_dn(exploded_dn);
    new_exploded_dn.replace(0, new_rdn);
    const QString new_dn = new_exploded_dn.join(',');

    const QByteArray dn_array = dn.toUtf8();
    const char *dn_cstr = dn_array.constData();
    const QByteArray new_rdn_array = new_rdn.toUtf8();
    const char *new_rdn_cstr = new_rdn_array.constData();

    int result = ad_rename(ld, dn_cstr, new_rdn_cstr);

    const QString old_name = dn_get_rdn(dn);

    if (result == AD_SUCCESS) {
        success_status_message(QString(tr("Renamed \"%1\" to \"%2\"")).arg(old_name, new_name));

        emit_modified();

        return true;
    } else {
        const QString context = QString(tr("Failed to rename \"%1\" to \"%2\"")).arg(old_name, new_name);
        const QString error = default_error();

        error_status_message(context, error);

        return false;
    }
}

bool AdInterface::group_add_user(const QString &group_dn, const QString &user_dn) {
    const bool success = attribute_add_string(group_dn, ATTRIBUTE_MEMBER, user_dn);

    const QString user_name = dn_get_rdn(user_dn);
    const QString group_name = dn_get_rdn(group_dn);
    
    if (success) {
        success_status_message(QString(tr("Added user \"%1\" to group \"%2\"")).arg(user_name, group_name));

        emit_modified();

        return true;
    } else {
        const QString context = QString(tr("Failed to add user \"%1\" to group \"%2\"")).arg(user_name, group_name);

        error_status_message(context, "");

        return false;
    }
}

bool AdInterface::group_remove_user(const QString &group_dn, const QString &user_dn) {
    const bool success = attribute_delete_string(group_dn, ATTRIBUTE_MEMBER, user_dn);

    const QString user_name = dn_get_rdn(user_dn);
    const QString group_name = dn_get_rdn(group_dn);

    if (success) {
        success_status_message(QString(tr("Removed user \"%1\" from group \"%2\"")).arg(user_name, group_name));

        emit_modified();

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

    const QString name = dn_get_rdn(dn);
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

    const QString name = dn_get_rdn(dn);
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

    const QString name = dn_get_rdn(dn);
    
    if (success) {
        success_status_message(QString(tr("Set pass of \"%1\"")).arg(name));

        emit_modified();

        return true;
    } else {
        const QString context = QString(tr("Failed to set pass of \"%1\"")).arg(name);

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
                if (object.contains(ATTRIBUTE_USER_ACCOUNT_CONTROL)) {
                    return object.get_int(ATTRIBUTE_USER_ACCOUNT_CONTROL);
                } else {
                    return 0;
                }
            }();

            const int bit = account_option_bit(option);
            const int updated_uac = bit_set(uac, bit, set);

            success = attribute_replace_int(dn, ATTRIBUTE_USER_ACCOUNT_CONTROL, updated_uac, DoStatusMsg_No);
        }
    }

    const QString name = dn_get_rdn(dn);
    
    if (success) {
        const QString success_context =
        [option, set, name]() {
            switch (option) {
                case AccountOption_Disabled: {
                    if (set) {
                        return QString(tr("Disabled account for user - \"%1\"")).arg(name);
                    } else {
                        return QString(tr("Enabled account user - \"%1\"")).arg(name);
                    }
                }
                default: {
                    const QString description = account_option_string(option);

                    if (set) {
                        return QString(tr("Turned ON account option \"%1\" of user \"%2\"")).arg(description, name);
                    } else {
                        return QString(tr("Turned OFF account option \"%1\" of user \"%2\"")).arg(description, name);
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
                        return QString(tr("Failed to disable account for user - \"%1\"")).arg(name);
                    } else {
                        return QString(tr("Enabled account for user - \"%1\"")).arg(name);
                    }
                }
                default: {
                    const QString description = account_option_string(option);

                    if (set) {
                        return QString(tr("Failed to turn ON account option \"%1\" of user \"%2\"")).arg(description, name);
                    } else {
                        return QString(tr("Failed to turn OFF account option \"%1\" of user \"%2\"")).arg(description, name);
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

    const QString name = dn_get_rdn(dn);
    
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

    const AdObject dropped = AD()->search_object(dn, {ATTRIBUTE_OBJECT_CLASS, ATTRIBUTE_OBJECT_CATEGORY});
    const AdObject target = AD()->search_object(target_dn, {ATTRIBUTE_OBJECT_CLASS});

    const bool dropped_is_user = dropped.is_class(CLASS_USER);
    const bool target_is_group = target.is_class(CLASS_USER);

    if (dropped_is_user && target_is_group) {
        return DropType_AddToGroup;
    } else {
        const QString dropped_category = dropped.get_string(ATTRIBUTE_OBJECT_CATEGORY);
        const QList<QString> possible_superiors = ADCONFIG()->get_possible_superiors(dropped_category);

        const bool can_move =
        [target, possible_superiors]() {
            const QList<QString> object_classes = target.get_strings(ATTRIBUTE_OBJECT_CLASS);
            for (const auto object_class : possible_superiors) {
                if (object_classes.contains(object_class)) {
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
            AD()->group_add_user(target_dn, dn);
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
    // TODO: is it ok to not swap bytes here, like it's done in octet_to_display_value()? Probably yes.
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
    const QByteArray main_dir_bytes = main_dir.toUtf8();
    const char *main_dir_cstr = main_dir_bytes.constData();
    const int result_mkdir_main = smbc_mkdir(main_dir_cstr, 0);
    if (result_mkdir_main != 0) {
        // TODO: handle errors
        return false;
    }

    const QString machine_dir = main_dir + "/Machine";
    const QByteArray machine_dir_bytes = machine_dir.toUtf8();
    const char *machine_dir_cstr = machine_dir_bytes.constData();
    const int result_mkdir_machine = smbc_mkdir(machine_dir_cstr, 0);
    if (result_mkdir_machine != 0) {
        // TODO: handle errors
        return false;
    }

    const QString user_dir = main_dir + "/User";
    const QByteArray user_dir_bytes = user_dir.toUtf8();
    const char *user_dir_cstr = user_dir_bytes.constData();
    const int result_mkdir_user = smbc_mkdir(user_dir_cstr, 0);
    if (result_mkdir_user != 0) {
        // TODO: handle errors
        return false;
    }

    const QString init_file_path = main_dir + "/GPT.INI";
    const QByteArray init_file_path_bytes = init_file_path.toUtf8();
    const char *init_file_path_cstr = init_file_path_bytes.constData();
    const int ini_file = smbc_open(init_file_path_cstr, O_WRONLY | O_CREAT, 0);

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
    const char *gpc_classes[] = {CLASS_TOP, CLASS_CONTAINER, CLASS_GP_CONTAINER, NULL};
    const char *container_classes[] = {CLASS_TOP, CLASS_CONTAINER, NULL};

    const QString dn = QString("CN=%1,CN=Policies,CN=System,%2").arg(uuid, search_base());
    const bool result_add = object_add(dn, gpc_classes);
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
    const bool result_add_user = object_add(user_dn, container_classes);
    attribute_replace_string(dn, ATTRIBUTE_SHOW_IN_ADVANCED_VIEW_ONLY, "TRUE");
    if (!result_add_user) {
        return false;
    }

    // Machine object
    const QString machine_dn = "CN=Machine," + dn;
    const bool result_add_machine = object_add(machine_dn, container_classes);
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
    const QByteArray url_bytes = url.toUtf8();
    const char *url_cstr = url_bytes.constData();
    const int result_rmdir = smbc_rmdir(url_cstr);
    if (result_rmdir < 0) {
        return false;
    }

    return true;
}

void AdInterface::command(QStringList args) {
    QString command = args[0];

    QMap<QString, int> arg_count_map = {
        {"get-attribute", 2},
        {"get-attribute-multi", 2},
    };

    const int arg_count = arg_count_map[command];
    if (args.size() - 1 != arg_count) {
        printf("Command \"%s\" needs %d arguments!\n", qPrintable(command), arg_count);

        return;
    }
}

AdInterface::AdInterface()
: QObject()
{

}

void AdInterface::emit_modified() {
    if (batch_in_progress) {
        return;
    } else {
        suppress_not_found_error = true;
        emit modified();
        suppress_not_found_error = false;
    }
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
        msg += QString(tr(". Error: \"%2\"")).arg(error);;
    }

    Status::instance()->message(msg, StatusType_Error);
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
    const QByteArray domain_array = domain.toUtf8();
    const char *domain_cstr = domain_array.constData();

    const QByteArray site_array = site.toUtf8();
    const char *site_cstr = site_array.constData();

    char **hosts_raw = NULL;
    int hosts_result = ad_get_domain_hosts(domain_cstr, site_cstr, &hosts_raw);

    if (hosts_result == AD_SUCCESS) {
        auto hosts = QList<QString>();

        for (int i = 0; hosts_raw[i] != NULL; i++) {
            auto host = QString(hosts_raw[i]);
            hosts.push_back(host);
        }
        ad_array_free(hosts_raw);

        return hosts;
    } else {
        return QList<QString>();
    }
}

bool datetime_is_never(const QString &attribute, const QString &value) {
    const LargeIntegerSubtype subtype = ADCONFIG()->get_large_integer_subtype(attribute);

    if (subtype == LargeIntegerSubtype_Datetime) {
        const bool is_never = (value == AD_LARGE_INTEGER_DATETIME_NEVER_1 || value == AD_LARGE_INTEGER_DATETIME_NEVER_2);
        
        return is_never;
    } else {
        return false;
    }
}

QString datetime_qdatetime_to_string(const QString &attribute, const QDateTime &datetime) {
    const AttributeType type = ADCONFIG()->get_attribute_type(attribute);

    switch (type) {
        case AttributeType_LargeInteger: {
            const LargeIntegerSubtype subtype = ADCONFIG()->get_large_integer_subtype(attribute);
            if (subtype == LargeIntegerSubtype_Datetime) {
                const QDateTime ntfs_epoch(QDate(1601, 1, 1));
                const qint64 millis = ntfs_epoch.msecsTo(datetime);
                const qint64 hundred_nanos = millis * MILLIS_TO_100_NANOS;
                
                return QString::number(hundred_nanos);
            }

            break;
        }
        case AttributeType_UTCTime: {
            // TODO: i think this uses 2 digits for year and needs a different format
            return datetime.toString(UTC_TIME_FORMAT_STRING);

            break;
        }
        case AttributeType_GeneralizedTime: {
            return datetime.toString(GENERALIZED_TIME_FORMAT_STRING);

            break;
        }
        default: return "";
    }

    return "";
}

QDateTime datetime_string_to_qdatetime(const QString &attribute, const QString &raw_value) {
    const AttributeType type = ADCONFIG()->get_attribute_type(attribute);

    switch (type) {
        case AttributeType_LargeInteger: {
            const LargeIntegerSubtype subtype = ADCONFIG()->get_large_integer_subtype(attribute);
            if (subtype == LargeIntegerSubtype_Datetime) {
                // TODO: couldn't find epoch in qt, but maybe its hidden somewhere
                QDateTime datetime(QDate(1601, 1, 1));
                const qint64 hundred_nanos = raw_value.toLongLong();
                const qint64 millis = hundred_nanos / MILLIS_TO_100_NANOS;
                datetime = datetime.addMSecs(millis);
                return datetime;
            }

            break;
        }
        case AttributeType_GeneralizedTime: {
            return QDateTime::fromString(raw_value, GENERALIZED_TIME_FORMAT_STRING);
        }
        case AttributeType_UTCTime: {
            return QDateTime::fromString(raw_value, UTC_TIME_FORMAT_STRING);
        }
        default: {
            return QDateTime();
        }
    }

    return QDateTime();
}

QString account_option_string(const AccountOption &option) {
    switch (option) {
        case AccountOption_Disabled: return AdInterface::tr("Account disabled");
        case AccountOption_PasswordExpired: return AdInterface::tr("User must change password on next logon");
        case AccountOption_DontExpirePassword: return AdInterface::tr("Don't expire password");
        case AccountOption_UseDesKey: return AdInterface::tr("Store password using reversible encryption");
        case AccountOption_SmartcardRequired: return AdInterface::tr("Smartcard is required for interactive logon");
        case AccountOption_CantDelegate: return AdInterface::tr("Account is sensitive and cannot be delegated");
        case AccountOption_DontRequirePreauth: return AdInterface::tr("Don't require Kerberos preauthentication");
        case AccountOption_COUNT: return QString("AccountOption_COUNT");
    }

    return "";
}

int account_option_bit(const AccountOption &option) {
    // NOTE: not all account options can be directly mapped to bits
    switch (option) {
        case AccountOption_Disabled: 
        return 0x00000002;
        case AccountOption_DontExpirePassword: 
        return 0x00010000;
        case AccountOption_UseDesKey: 
        return 0x00200000;
        case AccountOption_SmartcardRequired: 
        return 0x00040000;
        case AccountOption_DontRequirePreauth: 
        return 0x00400000;
        case AccountOption_CantDelegate: 
        return 0x00100000;

        case AccountOption_PasswordExpired: return 0;
        case AccountOption_COUNT: return 0;
    }

    return 0;
}

int group_scope_bit(GroupScope scope) {
    switch (scope) {
        case GroupScope_Global: return 0x00000002;
        case GroupScope_DomainLocal: return 0x00000004;
        case GroupScope_Universal: return 0x00000008;
        case GroupScope_COUNT: return 0;
    }
    return 0;
}

QString group_scope_string(GroupScope scope) {
    switch (scope) {
        case GroupScope_Global: return AdInterface::tr("Global");
        case GroupScope_DomainLocal: return AdInterface::tr("DomainLocal");
        case GroupScope_Universal: return AdInterface::tr("Universal");
        case GroupScope_COUNT: return "COUNT";
    }
    return "";
}

QString group_type_string(GroupType type) {
    switch (type) {
        case GroupType_Security: return AdInterface::tr("Security");
        case GroupType_Distribution: return AdInterface::tr("Distribution");
        case GroupType_COUNT: return "COUNT";
    }
    return "";
}

QString sysvol_path_to_smb(const QString &sysvol_path) {
    QString out = sysvol_path;
    
    out.replace("\\", "/");

    // TODO: file sys path as it is, is like this:
    // "smb://domain.alt/sysvol/domain.alt/Policies/{D7E75BC7-138D-4EE1-8974-105E4A2DE560}"
    // But that fails to load the whole directory sometimes
    // Replacing domain at the start with current host fixes it
    // "smb://dc0.domain.alt/sysvol/domain.alt/Policies/{D7E75BC7-138D-4EE1-8974-105E4A2DE560}"
    // not sure if this is required and which host/DC is the correct one
    const QString host = AD()->host();

    const int sysvol_i = out.indexOf("sysvol");
    out.remove(0, sysvol_i);

    out = QString("smb://%1/%2").arg(host, out);

    return out;
}
