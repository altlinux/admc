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
#include "server_configuration.h"
#include "ad_object.h"

#include <ldap.h>
#include <lber.h>

#include <QSet>
#include <QMessageBox>
#include <QTextCodec>
#include <QDebug>

#define MILLIS_TO_100_NANOS 10000

#define DATETIME_DISPLAY_FORMAT   "dd.MM.yy hh:mm"

#define SEARCH_ALL_ATTRIBUTES "SEARCH_ALL_ATTRIBUTES"

// TODO: confirm these are fine. I think need to make seconds option for UTC? Don't know if QDatetime can handle that.
#define GENERALIZED_TIME_FORMAT_STRING "yyyyMMddhhmmss.zZ"
#define UTC_TIME_FORMAT_STRING "yyMMddhhmmss.zZ"

#define LDAP_SEARCH_NO_ATTRIBUTES "1.1"

QString object_sid_to_display_string(const QByteArray &bytes);

AdInterface *AdInterface::instance() {
    static AdInterface ad_interface;
    return &ad_interface;
}

AdInterface::AdInterface()
: QObject()
{

}

bool AdInterface::login(const QString &host_arg, const QString &domain) {
    m_host = host_arg;

    const QString uri = "ldap://" + m_host;

    // Transform domain to search base
    // "DOMAIN.COM" => "DC=domain,DC=com"
    m_search_base = domain;
    m_search_base = m_search_base.toLower();
    m_search_base = "DC=" + m_search_base;
    m_search_base = m_search_base.replace(".", ",DC=");

    m_configuration_dn = "CN=Configuration," + m_search_base;
    m_schema_dn = "CN=Schema," + m_configuration_dn;

    const QByteArray uri_array = uri.toUtf8();
    const char *uri_cstr = uri_array.constData();

    const int result = ad_login(uri_cstr, &ld);

    if (result == AD_SUCCESS) {
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

QList<QString> AdInterface::search_dns(const QString &filter, const QString &custom_search_base) {
    const QHash<QString, AdObject> search_results = search(filter, QList<QString>(), SearchScope_All, custom_search_base);
    const QList<QString> dns = search_results.keys();


    return dns;
}

AdObject AdInterface::request_attributes(const QString &dn, const QList<QString> &attributes) {
    const QHash<QString, AdObject> search_results = search("", attributes, SearchScope_Object, dn);

    if (search_results.contains(dn)) {
        return search_results[dn];
    } else {
        return AdObject();
    }
}

AdObject AdInterface::request_all(const QString &dn) {
    const AdObject result = request_attributes(dn, {SEARCH_ALL_ATTRIBUTES});

    return result;
}

QList<QByteArray> AdInterface::request_values(const QString &dn, const QString &attribute) {
    const AdObject object = request_attributes(dn, {attribute});
    
    return object.get_values(attribute);
}

QByteArray AdInterface::request_value(const QString &dn, const QString &attribute) {
    const AdObject object = request_attributes(dn, {attribute});
    
    return object.get_value(attribute);
}

QList<QString> AdInterface::request_strings(const QString &dn, const QString &attribute) {
    const AdObject object = request_attributes(dn, {attribute});

    return object.get_strings(attribute);
}

QString AdInterface::request_string(const QString &dn, const QString &attribute) {
    const AdObject object = request_attributes(dn, {attribute});
    
    return object.get_string(attribute);
}

QList<int> AdInterface::request_ints(const QString &dn, const QString &attribute) {
    const AdObject object = request_attributes(dn, {attribute});

    return object.get_ints(attribute);
}

int AdInterface::request_int(const QString &dn, const QString &attribute) {
    const AdObject object = request_attributes(dn, {attribute});
    
    return object.get_int(attribute);
}

bool AdInterface::attribute_add_string(const QString &dn, const QString &attribute, const QString &value, const DoStatusMsg do_msg) {
    const QByteArray value_bytes = value.toUtf8();
    
    return attribute_add(dn, attribute, value_bytes, do_msg);
}

bool AdInterface::attribute_replace_string(const QString &dn, const QString &attribute, const QString &value, const DoStatusMsg do_msg) {
    const QByteArray value_bytes = value.toUtf8();

    return attribute_replace(dn, attribute, value_bytes, do_msg);
}

bool AdInterface::attribute_delete_string(const QString &dn, const QString &attribute, const QString &value, const DoStatusMsg do_msg) {
    const QByteArray value_bytes = value.toUtf8();
    
    return attribute_delete(dn, attribute, value_bytes, do_msg);
}

bool AdInterface::attribute_add(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg) {
    const QByteArray dn_array = dn.toUtf8();
    const char *dn_cstr = dn_array.constData();

    const QByteArray attribute_array = attribute.toUtf8();
    const char *attribute_cstr = attribute_array.constData();

    const char *value_cstr = value.constData();

    const int result = ad_attribute_add(ld, dn_cstr, attribute_cstr, value_cstr, value.size());

    const QString name = extract_name_from_dn(dn);

    const QString new_value_display = attribute_get_display_value(attribute, value);
    ;

    if (result == AD_SUCCESS) {
        const QString context = QString(tr("Added value \"%1\" for attribute \"%2\" of object \"%3\"")).arg(new_value_display, attribute, name);

        success_status_message(context, do_msg);

        emit_modified();

        return true;
    } else {
        const QString context = QString(tr("Failed to add value \"%1\" for attribute \"%2\" of object \"%3\"")).arg(new_value_display, attribute, name);
        const QString error = default_error();

        error_status_message(context, error, do_msg);

        return false;
    }
}

bool AdInterface::attribute_replace(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg) {
    const QByteArray old_value = request_value(dn, attribute);

    const QString string(value);

    if (old_value.isEmpty() && value.isEmpty()) {
        // do nothing
        return true;
    }

    const char *old_value_cstr = old_value.constData();
    
    const QByteArray dn_array = dn.toUtf8();
    const char *dn_cstr = dn_array.constData();

    const QByteArray attribute_array = attribute.toUtf8();
    const char *attribute_cstr = attribute_array.constData();

    const char *value_cstr = value.constData();

    int result;
    if (value.isEmpty()) {
        result = ad_attribute_delete(ld, dn_cstr, attribute_cstr, old_value_cstr, old_value.size());
    } else {
        result = ad_attribute_replace(ld, dn_cstr, attribute_cstr, value_cstr, value.size());
    }

    const QString name = extract_name_from_dn(dn);

    const QString old_value_display = attribute_get_display_value(attribute, old_value);
    const QString new_value_display = attribute_get_display_value(attribute, value);

    if (result == AD_SUCCESS) {
        success_status_message(QString(tr("Changed attribute \"%1\" of \"%2\" from \"%3\" to \"%4\"")).arg(attribute, name, old_value_display, new_value_display), do_msg);

        emit_modified();

        return true;
    } else {
        const QString context = QString(tr("Failed to change attribute \"%1\" of object \"%2\" from \"%3\" to \"%4\"")).arg(attribute, name, old_value_display, new_value_display);
        const QString error = default_error();

        error_status_message(context, error, do_msg);

        return false;
    }
}

bool AdInterface::attribute_delete(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg) {
    const QByteArray dn_array = dn.toUtf8();
    const char *dn_cstr = dn_array.constData();

    const QByteArray attribute_array = attribute.toUtf8();
    const char *attribute_cstr = attribute_array.constData();

    const char *value_cstr = value.constData();

    const int result = ad_attribute_delete(ld, dn_cstr, attribute_cstr, value_cstr, value.size());

    const QString name = extract_name_from_dn(dn);

    const QString value_display = attribute_get_display_value(attribute, value);

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

bool AdInterface::attribute_replace_int(const QString &dn, const QString &attribute, const int value) {
    const QString value_string = QString::number(value);
    const bool result = attribute_replace_string(dn, attribute, value_string);

    return result;
}

bool AdInterface::attribute_replace_datetime(const QString &dn, const QString &attribute, const QDateTime &datetime) {
    const QString datetime_string = datetime_to_string(attribute, datetime);
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

    const QString name = extract_name_from_dn(dn);
    
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
    
    const QString object_name = extract_name_from_dn(dn);
    const QString container_name = extract_name_from_dn(new_container);

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

bool AdInterface::group_add_user(const QString &group_dn, const QString &user_dn) {
    const bool success = attribute_add_string(group_dn, ATTRIBUTE_MEMBER, user_dn);

    const QString user_name = extract_name_from_dn(user_dn);
    const QString group_name = extract_name_from_dn(group_dn);
    
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

    const QString user_name = extract_name_from_dn(user_dn);
    const QString group_name = extract_name_from_dn(group_dn);

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
    int group_type = request_int(dn, ATTRIBUTE_GROUP_TYPE);

    // Unset all scope bits, because scope bits are exclusive
    for (int i = 0; i < GroupScope_COUNT; i++) {
        const GroupScope this_scope = (GroupScope) i;
        const int this_scope_bit = group_scope_to_bit(this_scope);

        group_type = bit_set(group_type, this_scope_bit, false);
    }

    // Set given scope bit
    const int scope_bit = group_scope_to_bit(scope);
    group_type = bit_set(group_type, scope_bit, true);

    const QString name = extract_name_from_dn(dn);
    const QString scope_string = group_scope_to_string(scope);
    
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
    const int group_type = request_int(dn, ATTRIBUTE_GROUP_TYPE);

    const bool set_security_bit = type == GroupType_Security;

    const int update_group_type = bit_set(group_type, GROUP_TYPE_BIT_SECURITY, set_security_bit);
    const QString update_group_type_string = QString::number(update_group_type);

    const QString name = extract_name_from_dn(dn);
    const QString type_string = group_type_to_string(type);
    
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

    const QString old_name = extract_name_from_dn(dn);

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

    const bool success = attribute_replace(dn, ATTRIBUTE_PASSWORD, password_bytes, DoStatusMsg_No);

    const QString name = extract_name_from_dn(dn);
    
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
            const QString control =
            [this, dn]() {
                QString out = request_string(dn, ATTRIBUTE_USER_ACCOUNT_CONTROL);
                if (out.isEmpty()) {
                    out = "0";
                }

                return out;
            }();

            const int bit = account_option_to_bit(option);

            int control_int = control.toInt();
            control_int = bit_set(control_int, bit, set);

            const QString control_updated = QString::number(control_int);

            success = attribute_replace_string(dn, ATTRIBUTE_USER_ACCOUNT_CONTROL, control_updated, DoStatusMsg_No);
        }
    }

    const QString name = extract_name_from_dn(dn);
    
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
                    const QString description = get_account_option_description(option);

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
                    const QString description = get_account_option_description(option);

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

    const QString name = extract_name_from_dn(dn);
    
    if (result) {
        success_status_message(QString(tr("Unlocked user \"%1\"")).arg(name));
        
        return true;
    } else {
        const QString context = QString(tr("Failed to unlock user \"%1\"")).arg(name);

        error_status_message(context, "");

        return result;
    }
}

bool attribute_get_account_option(const AdObject &object  , AccountOption option) {
    switch (option) {
        case AccountOption_PasswordExpired: {
            const QString pwdLastSet_value = object.get_string(ATTRIBUTE_PWD_LAST_SET);
            const bool expired = (pwdLastSet_value == AD_PWD_LAST_SET_EXPIRED);

            return expired;
        }
        default: {
            // Account option is a UAC bit
            if (object.contains(ATTRIBUTE_USER_ACCOUNT_CONTROL)) {
                return false;
            } else {
                const int control = object.get_int(ATTRIBUTE_USER_ACCOUNT_CONTROL);
                const int bit = account_option_to_bit(option);

                const bool set = ((control & bit) != 0);

                return set;
            }
        }
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

    const AdObject dropped = AD()->request_attributes(dn, {ATTRIBUTE_OBJECT_CLASS});
    const AdObject target = AD()->request_attributes(target_dn, {ATTRIBUTE_OBJECT_CLASS});

    const bool dropped_is_user = dropped.is_user();
    const bool target_is_group = target.is_user();

    if (dropped_is_user && target_is_group) {
        return DropType_AddToGroup;
    } else {
        const QList<QString> possible_superiors = get_possible_superiors(dropped);

        const bool can_move =
        [target, possible_superiors]() {
            for (const auto object_class : possible_superiors) {
                if (target.is_class(object_class)) {
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

// "CN=foo,CN=bar,DC=domain,DC=com"
// =>
// "foo"
QString extract_name_from_dn(const QString &dn) {
    int equals_i = dn.indexOf('=') + 1;
    int comma_i = dn.indexOf(',');
    int segment_length = comma_i - equals_i;

    QString name = dn.mid(equals_i, segment_length);

    return name;
}

// "CN=foo,CN=bar,DC=domain,DC=com"
// =>
// "CN=bar,DC=domain,DC=com"
QString extract_parent_dn_from_dn(const QString &dn) {
    int comma_i = dn.indexOf(',');

    QString parent_dn = dn.mid(comma_i + 1);

    return parent_dn;
}

AdInterface *AD() {
    return AdInterface::instance();
}

QString filter_EQUALS(const QString &attribute, const QString &value) {
    auto filter = QString("(%1=%2)").arg(attribute, value);
    return filter;
}

QString filter_AND(const QString &a, const QString &b) {
    auto filter = QString("(&%1%2)").arg(a, b);
    return filter;
}
QString filter_OR(const QString &a, const QString &b) {
    auto filter = QString("(|%1%2)").arg(a, b);
    return filter;
}
QString filter_NOT(const QString &a) {
    auto filter = QString("(!%1)").arg(a);
    return filter;
}

QString get_account_option_description(const AccountOption &option) {
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

int account_option_to_bit(const AccountOption &option) {
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

bool datetime_is_never(const QString &attribute, const QString &value) {
    const AttributeType type = get_attribute_type(attribute);

    if (type == AttributeType_LargeIntegerDatetime) {
        const bool is_never = (value == AD_LARGEINTEGERTIME_NEVER_1 || value == AD_LARGEINTEGERTIME_NEVER_2);
        
        return is_never;
    } else {
        return false;
    }
}

bool attribute_is_datetime(const QString &attribute) {
    static const QList<AttributeType> datetime_types = {
        AttributeType_LargeIntegerDatetime,
        AttributeType_UTCTime,
        AttributeType_GeneralizedTime
    };
    
    const AttributeType type = get_attribute_type(attribute);

    return datetime_types.contains(type);
}

QString datetime_to_string(const QString &attribute, const QDateTime &datetime) {
    const AttributeType type = get_attribute_type(attribute);

    switch (type) {
        case AttributeType_LargeIntegerDatetime: {
            const QDateTime ntfs_epoch(QDate(1601, 1, 1));
            const qint64 millis = ntfs_epoch.msecsTo(datetime);
            const qint64 hundred_nanos = millis * MILLIS_TO_100_NANOS;
            
            return QString::number(hundred_nanos);

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

QDateTime datetime_raw_to_datetime(const QString &attribute, const QString &raw_value) {
    const AttributeType type = get_attribute_type(attribute);

    switch (type) {
        case AttributeType_LargeIntegerDatetime: {
            // TODO: couldn't find epoch in qt, but maybe its hidden somewhere
            QDateTime datetime(QDate(1601, 1, 1));
            const qint64 hundred_nanos = raw_value.toLongLong();
            const qint64 millis = hundred_nanos / MILLIS_TO_100_NANOS;
            datetime = datetime.addMSecs(millis);
            return datetime;
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

int group_scope_to_bit(GroupScope scope) {
    switch (scope) {
        case GroupScope_Global: return 0x00000002;
        case GroupScope_DomainLocal: return 0x00000004;
        case GroupScope_Universal: return 0x00000008;
        case GroupScope_COUNT: return 0;
    }
    return 0;
}

QString group_scope_to_string(GroupScope scope) {
    switch (scope) {
        case GroupScope_Global: return AdInterface::tr("Global");
        case GroupScope_DomainLocal: return AdInterface::tr("DomainLocal");
        case GroupScope_Universal: return AdInterface::tr("Universal");
        case GroupScope_COUNT: return "COUNT";
    }
    return "";
}

QString group_type_to_string(GroupType type) {
    switch (type) {
        case GroupType_Security: return AdInterface::tr("Security");
        case GroupType_Distribution: return AdInterface::tr("Distribution");
        case GroupType_COUNT: return "COUNT";
    }
    return "";
}

// TODO: flesh this out
QString attribute_get_display_value(const QString &attribute, const QByteArray &value_bytes) {
    const AttributeType attribute_type = get_attribute_type(attribute);

    const QString value_string(value_bytes);

    if (attribute_is_datetime(attribute)) {
        if (datetime_is_never(attribute, value_string)) {
            return "(never)";
        }
        const QDateTime datetime = datetime_raw_to_datetime(attribute, value_string);
        const QString display = datetime.toString(DATETIME_DISPLAY_FORMAT);

        return display;
    } else if (attribute_type == AttributeType_Sid) {
        // TODO: convert to sid here
        return object_sid_to_display_string(value_bytes);
    } else {
        return value_string;
    }
}

QString attribute_value_to_display_value(const QString &attribute, const QString &value) {
    const QByteArray bytes = value.toUtf8();

    return attribute_get_display_value(attribute, bytes);
}

// TODO: replace with some library if possible. Maybe one of samba's libs has this.
// NOTE: https://ldapwiki.com/wiki/ObjectSID
QString object_sid_to_display_string(const QByteArray &sid) {
    QString string = "S-";
    
    // byte[0] - revision level
    const int revision = sid[0];
    string += QString::number(revision);
    
    // byte[1] - count of sub-authorities
    const int sub_authority_count = sid[1] & 0xFF;
    
    // byte(2-7) - authority 48 bit Big-Endian
    long authority = 0;
    for (int i = 2; i <= 7; i++) {
        authority |= ((long)sid[i]) << (8 * (5 - (i - 2)));
    }
    string += "-" + QString::number(authority);
    // TODO: not sure if authority is supposed to be formatted as hex or not. Currently it matches how ADUC displays it.
    
    // byte(8-...)
    // sub-authorities 32 bit Little-Endian
    int offset = 8;
    const int bytes = 4;
    for (int i = 0; i < sub_authority_count; i++) {
        long sub_authority = 0;
        for (int j = 0; j < bytes; j++) {
            sub_authority |= (long)(sid[offset + j] & 0xFF) << (8 * j);
        }

        string += "-" + QString::number(sub_authority);

        offset += bytes;
    }

    return string;
}
