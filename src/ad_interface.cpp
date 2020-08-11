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
#include "ad_connection.h"
#include "ldap.h"

#include <QSet>

#define MILLIS_TO_100_NANOS 10000

#define AD_PWD_LAST_SET_EXPIRED "0"
#define AD_PWD_LAST_SET_RESET   "-1"

#define DATETIME_DISPLAY_FORMAT   "dd.MM.yy hh:mm"

enum DatetimeFormat {
    DatetimeFormat_LargeInteger,
    DatetimeFormat_ISO8601,
    DatetimeFormat_None
};

DatetimeFormat get_attribute_time_format(const QString &attribute);

AdInterface::~AdInterface() {
    delete connection;
}

AdInterface *AdInterface::instance() {
    static AdInterface ad_interface;
    return &ad_interface;
}

AdInterface::AdInterface()
: QObject()
{
    connection = new adldap::AdConnection();
}

QList<QString> AdInterface::get_domain_hosts(const QString &domain, const QString &site) {
    const QByteArray domain_array = domain.toLatin1();
    const char *domain_cstr = domain_array.constData();

    const QByteArray site_array = site.toLatin1();
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

AdResult AdInterface::login(const QString &host, const QString &domain) {
    const QString uri = "ldap://" + host;
    const std::string uri_std = uri.toStdString();
    const std::string domain_std = domain.toStdString();

    const int result = connection->connect(uri_std, domain_std);

    if (result == AD_SUCCESS) {
        success_status_message(QString(tr("Logged in to \"%1\" at \"%2\"")).arg(host, domain));

        emit logged_in();

        return AdResult(true);
    } else {
        const QString context = QString(tr("Failed to login to \"%1\" at \"%2\"")).arg(host, domain);
        const QString error_string = default_error_string(result);

        error_status_message(context, error_string);

        return AdResult(false, error_string);
    }
}

QString AdInterface::get_search_base() {
    return QString::fromStdString(connection->get_search_base());
}

QString AdInterface::get_uri() {
    return QString::fromStdString(connection->get_uri());
}

QList<QString> AdInterface::list(const QString &dn) {
    const QByteArray dn_array = dn.toLatin1();
    const char *dn_cstr = dn_array.constData();

    char **children_raw;
    const int result = connection->list(dn_cstr, &children_raw);

    if (result == AD_SUCCESS) {
        auto children = QList<QString>();

        for (int i = 0; children_raw[i] != NULL; i++) {
            auto child = QString(children_raw[i]);
            children.push_back(child);
        }

        ad_array_free(children_raw);

        return children;
    } else {
        if (should_emit_status_message(result)) {
            const QString name = extract_name_from_dn(dn);
            const QString context = QString(tr("Failed to load children of \"%1\"")).arg(name);
            const QString error_string = default_error_string(result);

            error_status_message(context, error_string);
        }

        return QList<QString>();
    }
}

QList<QString> AdInterface::search(const QString &filter) {
    const QByteArray filter_array = filter.toLatin1();
    const char *filter_cstr = filter_array.constData();

    char **results_raw;
    const int result_search = connection->search(filter_cstr, &results_raw);
    if (result_search == AD_SUCCESS) {
        auto results = QList<QString>();

        for (int i = 0; results_raw[i] != NULL; i++) {
            auto result = QString(results_raw[i]);
            results.push_back(result);
        }

        ad_array_free(results_raw);

        return results;
    } else {
        if (should_emit_status_message(result_search)) {
            const QString context = QString(tr("Failed to search for \"%1\"")).arg(filter);
            const QString error_string = default_error_string(result_search);

            error_status_message(context, error_string);
        }

        return QList<QString>();
    }
}

Attributes AdInterface::get_all_attributes(const QString &dn) {
    if (dn == "") {
        return Attributes();
    }

    // Load attributes if it's not in cache
    if (attributes_cache.contains(dn)) {
        return attributes_cache[dn];
    } else {
        const QByteArray dn_array = dn.toLatin1();
        const char *dn_cstr = dn_array.constData();

        char ***attributes_ad;
        const int result_attribute_get = connection->get_all_attributes(dn_cstr, &attributes_ad);

        if (result_attribute_get == AD_SUCCESS) {
            Attributes attributes;
            // attributes_ad is in the form of:
            // char** array of {{keyA, value1, value2, ...},
            // {keyB, value1, value2, ...}, ...}
            // transform it into:
            // map of {key => {value, value ...}, key => {value, value ...} ...}
            for (int i = 0; attributes_ad[i] != NULL; i++) {
                char **attributes_and_values = attributes_ad[i];

                char *attribute_cstr = attributes_and_values[0];
                auto attribute = QString(attribute_cstr);
                attributes[attribute] = QList<QString>();

                for (int j = 1; attributes_and_values[j] != NULL; j++) {
                    char *value_cstr = attributes_and_values[j];
                    auto value = QString(value_cstr);
                    attributes[attribute].push_back(value);
                }
            }

            ad_2d_array_free(attributes_ad);

            attributes_cache[dn] = attributes;
            return attributes_cache[dn];
        } else {
            if (should_emit_status_message(result_attribute_get)) {
                const QString name = extract_name_from_dn(dn);
                const QString context = QString(tr("Failed to get attributes of \"%1\"")).arg(name);
                const QString error_string = default_error_string(result_attribute_get);

                error_status_message(context, error_string);
            }

            return Attributes();
        }
    }
}

QList<QString> AdInterface::attribute_get_multi(const QString &dn, const QString &attribute) {
    QMap<QString, QList<QString>> attributes = get_all_attributes(dn);

    if (attributes.contains(attribute)) {
        return attributes[attribute];
    } else {
        return QList<QString>();
    }
}

QString AdInterface::attribute_get(const QString &dn, const QString &attribute) {
    QList<QString> values = attribute_get_multi(dn, attribute);

    if (values.size() > 0) {
        // Return first value only
        return values[0];
    } else {
        return "";
    }
}

QDateTime AdInterface::attribute_datetime_get(const QString &dn, const QString &attribute) {
    const QString raw_value = attribute_get(dn, attribute);
    const QDateTime datetime = datetime_raw_to_datetime(attribute, raw_value);

    return datetime;
}

AdResult AdInterface::attribute_datetime_replace(const QString &dn, const QString &attribute, const QDateTime &datetime) {
    const DatetimeFormat format = get_attribute_time_format(attribute);

    QString datetime_string;
    switch (format) {
        case DatetimeFormat_LargeInteger: {
            const QDateTime ntfs_epoch(QDate(1601, 1, 1));
            const qint64 millis = ntfs_epoch.msecsTo(datetime);
            const qint64 hundred_nanos = millis * MILLIS_TO_100_NANOS;
            
            datetime_string = QString::number(hundred_nanos);

            break;
        }
        case DatetimeFormat_ISO8601: {
            datetime_string = datetime.toString(ISO8601_FORMAT_STRING);

            break;
        }
        case DatetimeFormat_None: return AdResult(false, "");
    }

    const AdResult result = attribute_replace(dn, attribute, datetime_string);

    return result;
}

AdResult AdInterface::attribute_replace(const QString &dn, const QString &attribute, const QString &value, EmitStatusMessage emit_message) {
    QString old_value_string;

    if (attribute_is_datetime(attribute)) {
        const QString old_value_raw = attribute_get(dn, attribute);
        old_value_string = datetime_raw_to_string(attribute, old_value_raw);
    } else {
        old_value_string = attribute_get(dn, attribute);
    }
    
    const QByteArray dn_array = dn.toLatin1();
    const char *dn_cstr = dn_array.constData();

    const QByteArray attribute_array = attribute.toLatin1();
    const char *attribute_cstr = attribute_array.constData();

    const QByteArray value_array = value.toLatin1();
    const char *value_cstr = value_array.constData();

    int result = connection->attribute_replace(dn_cstr, attribute_cstr, value_cstr);

    const QString name = extract_name_from_dn(dn);

    QString new_value_string;
    if (attribute_is_datetime(attribute)) {
        new_value_string = datetime_raw_to_string(attribute, value);
    } else {
        new_value_string = value;
    }

    if (result == AD_SUCCESS) {
        success_status_message(QString(tr("Changed attribute \"%1\" of \"%2\" from \"%3\" to \"%4\"")).arg(attribute, name, old_value_string, new_value_string), emit_message);

        update_cache({dn});

        return AdResult(true);
    } else {
        const QString context = QString(tr("Failed to change attribute \"%1\" of object \"%2\" from \"%3\" to \"%4\"")).arg(attribute, name, old_value_string, new_value_string);
        const QString error_string = default_error_string(result);

        error_status_message(context, error_string, emit_message);

        return AdResult(false, error_string);
    }
}

// TODO: can probably make a create_anything() function with enum parameter
AdResult AdInterface::object_create(const QString &name, const QString &dn, NewObjectType type) {

    const QByteArray name_array = name.toLatin1();
    const char *name_cstr = name_array.constData();

    const QByteArray dn_array = dn.toLatin1();
    const char *dn_cstr = dn_array.constData();

    int result = AD_ERROR;
    switch (type) {
        case User: {
            result = connection->create_user(name_cstr, dn_cstr);
            break;
        }
        case Computer: {
            result = connection->create_computer(name_cstr, dn_cstr);
            break;
        }
        case OU: {
            result = connection->create_ou(name_cstr, dn_cstr);
            break;
        }
        case Group: {
            result = connection->create_group(name_cstr, dn_cstr);
            break;
        }
        case COUNT: break;
    }

    if (result == AD_SUCCESS) {
        success_status_message(QString(tr("Created \"%1\"")).arg(name));

        update_cache({dn});

        return AdResult(true);
    } else {
        const QString context = QString(tr("Failed to create \"%1\"")).arg(name);
        const QString error_string = default_error_string(result);

        error_status_message(context, error_string);

        return AdResult(false, error_string);
    }
}

AdResult AdInterface::object_delete(const QString &dn) {
    const QByteArray dn_array = dn.toLatin1();
    const char *dn_cstr = dn_array.constData();

    int result = connection->object_delete(dn_cstr);

    const QString name = extract_name_from_dn(dn);
    
    if (result == AD_SUCCESS) {
        success_status_message(QString(tr("Deleted object \"%1\"")).arg(name));

        update_cache({dn});

        return AdResult(true);
    } else {
        const QString context = QString(tr("Failed to delete object \"%1\"")).arg(name);
        const QString error_string = default_error_string(result);

        error_status_message(context, error_string);

        return AdResult(false, error_string);
    }
}

AdResult AdInterface::object_move(const QString &dn, const QString &new_container) {
    QList<QString> dn_split = dn.split(',');
    QString new_dn = dn_split[0] + "," + new_container;

    const QByteArray dn_array = dn.toLatin1();
    const char *dn_cstr = dn_array.constData();

    const QByteArray new_container_array = new_container.toLatin1();
    const char *new_container_cstr = new_container_array.constData();

    int result = AD_ERROR;
    const bool object_is_user = is_user(dn);
    if (object_is_user) {
        result = connection->move_user(dn_cstr, new_container_cstr);
    } else {
        result = connection->move(dn_cstr, new_container_cstr);
    }

    // TODO: drag and drop handles checking move compatibility but need
    // to do this here as well for CLI?
    
    const QString object_name = extract_name_from_dn(dn);
    const QString container_name = extract_name_from_dn(new_container);
    
    if (result == AD_SUCCESS) {
        success_status_message(QString(tr("Moved \"%1\" to \"%2\"")).arg(object_name, container_name));

        update_cache({dn});

        return AdResult(true);
    } else {
        const QString context = QString(tr("Failed to move \"%1\" to \"%2\"")).arg(object_name, container_name);
        const QString error_string = default_error_string(result);

        error_status_message(context, error_string);

        return AdResult(false, error_string);
    }
}

AdResult AdInterface::group_add_user(const QString &group_dn, const QString &user_dn) {
    const QByteArray group_dn_array = group_dn.toLatin1();
    const char *group_dn_cstr = group_dn_array.constData();

    const QByteArray user_dn_array = user_dn.toLatin1();
    const char *user_dn_cstr = user_dn_array.constData();

    int result = connection->group_add_user(group_dn_cstr, user_dn_cstr);

    const QString user_name = extract_name_from_dn(user_dn);
    const QString group_name = extract_name_from_dn(group_dn);
    
    if (result == AD_SUCCESS) {
        success_status_message(QString(tr("Added user \"%1\" to group \"%2\"")).arg(user_name, group_name));

        update_cache({group_dn, user_dn});

        return AdResult(true);
    } else {
        const QString context = QString(tr("Failed to add user \"%1\" to group \"%2\"")).arg(user_name, group_name);
        const QString error_string = default_error_string(result);

        error_status_message(context, error_string);

        return AdResult(false, error_string);
    }
}

AdResult AdInterface::group_remove_user(const QString &group_dn, const QString &user_dn) {
    const QByteArray group_dn_array = group_dn.toLatin1();
    const char *group_dn_cstr = group_dn_array.constData();

    const QByteArray user_dn_array = user_dn.toLatin1();
    const char *user_dn_cstr = user_dn_array.constData();

    int result = connection->group_remove_user(group_dn_cstr, user_dn_cstr);

    const QString user_name = extract_name_from_dn(user_dn);
    const QString group_name = extract_name_from_dn(group_dn);

    if (result == AD_SUCCESS) {
        success_status_message(QString(tr("Removed user \"%1\" from group \"%2\"")).arg(user_name, group_name));

        update_cache({group_dn, user_dn});

        return AdResult(true);
    } else {
        const QString context = QString(tr("Failed to remove user \"%1\" from group \"%2\"")).arg(user_name, group_name);
        const QString error_string = default_error_string(result);

        error_status_message(context, error_string);

        return AdResult(false, error_string);
    }
}

AdResult AdInterface::object_rename(const QString &dn, const QString &new_name) {
    // Compose new_rdn and new_dn
    const QStringList exploded_dn = dn.split(',');
    const QString old_rdn = exploded_dn[0];
    const int prefix_i = old_rdn.indexOf('=') + 1;
    const QString prefix = old_rdn.left(prefix_i);
    const QString new_rdn = prefix + new_name;
    QStringList new_exploded_dn(exploded_dn);
    new_exploded_dn.replace(0, new_rdn);
    const QString new_dn = new_exploded_dn.join(',');

    const QByteArray dn_array = dn.toLatin1();
    const char *dn_cstr = dn_array.constData();
    const QByteArray new_name_array = new_name.toLatin1();
    const char *new_name_cstr = new_name_array.constData();
    const QByteArray new_rdn_array = new_rdn.toLatin1();
    const char *new_rdn_cstr = new_rdn_array.constData();

    int result = AD_ERROR;
    if (is_user(dn)) {
        result = connection->rename_user(dn_cstr, new_name_cstr);
    } else if (is_group(dn)) {
        result = connection->rename_group(dn_cstr, new_name_cstr);
    } else {
        result = connection->rename(dn_cstr, new_rdn_cstr);
    }

    const QString old_name = extract_name_from_dn(dn);

    if (result == AD_SUCCESS) {
        success_status_message(QString(tr("Renamed \"%1\" to \"%2\"")).arg(old_name, new_name));

        update_cache({dn});

        return AdResult(true);
    } else {
        const QString context = QString(tr("Failed to rename \"%1\" to \"%2\"")).arg(old_name, new_name);
        const QString error_string = default_error_string(result);

        error_status_message(context, error_string);

        return AdResult(false, error_string);
    }
}

AdResult AdInterface::set_pass(const QString &dn, const QString &password) {
    const QByteArray dn_array = dn.toLatin1();
    const char *dn_cstr = dn_array.constData();
    const QByteArray password_array = password.toLatin1();
    const char *password_cstr = password_array.constData();

    int result = connection->user_set_pass(dn_cstr, password_cstr);

    const QString name = extract_name_from_dn(dn);
    
    if (result == AD_SUCCESS) {
        success_status_message(QString(tr("Set pass of \"%1\"")).arg(name));

        update_cache({dn});

        return AdResult(true);
    } else {
        const QString context = QString(tr("Failed to set pass of \"%1\"")).arg(name);

        QString error_string;
        const int ldap_result = connection->get_ldap_result();
        if (result == AD_LDAP_ERROR && ldap_result == LDAP_CONSTRAINT_VIOLATION) {
            error_string = tr("Password doesn't match rules");
        } else {
            error_string = default_error_string(result);
        }

        error_status_message(context, error_string);

        return AdResult(false, error_string);
    }
}

// TODO:
// "User cannot change password" - CAN'T just set PASSWD_CANT_CHANGE. See: https://docs.microsoft.com/en-us/windows/win32/adsi/modifying-user-cannot-change-password-ldap-provider?redirectedfrom=MSDN
// "This account supports 128bit encryption" (and for 256bit)
// "Use Kerberos DES encryption types for this account"
AdResult AdInterface::user_set_account_option(const QString &dn, AccountOption option, bool set) {
    if (dn.isEmpty()) {
        return AdResult(false, "");
    }

    AdResult result(false);

    switch (option) {
        case AccountOption_PasswordExpired: {
            QString pwdLastSet_value;
            if (set) {
                pwdLastSet_value = AD_PWD_LAST_SET_EXPIRED;
            } else {
                pwdLastSet_value = AD_PWD_LAST_SET_RESET;
            }

            result = attribute_replace(dn, ATTRIBUTE_PWD_LAST_SET, pwdLastSet_value, EmitStatusMessage_No);

            break;
        }
        default: {
            QString control = attribute_get(dn, ATTRIBUTE_USER_ACCOUNT_CONTROL);
            if (control.isEmpty()) {
                control = "0";
            }

            const int bit = get_account_option_bit(option);

            int control_int = control.toInt();
            if (set) {
                control_int |= bit;
            } else {
                control_int ^= bit;
            }

            const QString control_updated = QString::number(control_int);

            result = attribute_replace(dn, ATTRIBUTE_USER_ACCOUNT_CONTROL, control_updated, EmitStatusMessage_No);
        }
    }

    const QString name = extract_name_from_dn(dn);
    
    if (result.success) {
        auto get_success_context =
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
        };
        const QString success_context = get_success_context();
        
        success_status_message(success_context);

        update_cache({dn});

        return AdResult(true);
    } else {
        auto get_success_context =
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
        };
        const QString error_context = get_success_context();

        error_status_message(error_context, result.error);

        return AdResult(false, result.error);
    }
}

AdResult AdInterface::user_unlock(const QString &dn) {
    const AdResult result = AdInterface::instance()->attribute_replace(dn, ATTRIBUTE_LOCKOUT_TIME, LOCKOUT_UNLOCKED_VALUE, EmitStatusMessage_No);

    const QString name = extract_name_from_dn(dn);
    
    if (result.success) {
        success_status_message(QString(tr("Unlocked user \"%1\"")).arg(name));
        
        return AdResult(true);
    } else {
        const QString context = QString(tr("Failed to unlock user \"%1\"")).arg(name);

        error_status_message(context, result.error);

        return result;
    }
}

bool AdInterface::is_class(const QString &dn, const QString &object_class) {
    const QList<QString> classes = attribute_get_multi(dn, ATTRIBUTE_OBJECT_CLASS);
    const bool is_class = classes.contains(object_class);

    return is_class;
}

bool AdInterface::is_user(const QString &dn) {
    return is_class(dn, CLASS_USER);
}

bool AdInterface::is_group(const QString &dn) {
    return is_class(dn, CLASS_GROUP);
}

bool AdInterface::is_container(const QString &dn) {
    return is_class(dn, CLASS_CONTAINER);
}

bool AdInterface::is_ou(const QString &dn) {
    return is_class(dn, CLASS_OU);
}

bool AdInterface::is_policy(const QString &dn) {
    return is_class(dn, CLASS_GP_CONTAINER);
}

bool AdInterface::is_container_like(const QString &dn) {
    // TODO: check that this includes all fitting objectClasses
    const QList<QString> containerlike_objectClasses = {CLASS_OU, CLASS_BUILTIN_DOMAIN, CLASS_DOMAIN};
    for (auto c : containerlike_objectClasses) {
        if (is_class(dn, c)) {
            return true;
        }
    }

    return false;
}

bool AdInterface::user_get_account_option(const QString &dn, AccountOption option) {
    switch (option) {
        case AccountOption_PasswordExpired: {
            const QString pwdLastSet_value = attribute_get(dn, ATTRIBUTE_PWD_LAST_SET);
            const bool expired = (pwdLastSet_value == AD_PWD_LAST_SET_EXPIRED);

            return expired;
        }
        default: {
            // Account option is a UAC bit
            const QString control = attribute_get(dn, ATTRIBUTE_USER_ACCOUNT_CONTROL);
            if (control.isEmpty()) {
                return false;
            }

            const int bit = get_account_option_bit(option);

            const int control_int = control.toInt();
            const bool set = ((control_int & bit) != 0);

            return set;
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

    const bool dropped_is_user = AdInterface::instance()->is_user(dn);
    const bool dropped_is_group = AdInterface::instance()->is_group(dn);
    const bool dropped_is_ou = AdInterface::instance()->is_ou(dn);

    const bool target_is_user = AdInterface::instance()->is_user(target_dn);
    const bool target_is_group = AdInterface::instance()->is_group(target_dn);
    const bool target_is_ou = AdInterface::instance()->is_ou(target_dn);
    const bool target_is_container = AdInterface::instance()->is_container(target_dn);

    if (dropped_is_user) {
        if (target_is_ou || target_is_container) {
            return DropType_Move;
        } else if (target_is_group) {
            return DropType_AddToGroup;
        }
    } else if (dropped_is_group || dropped_is_ou) {
        if (!target_is_user && !target_is_group) {
            return DropType_Move;
        }
    }

    return DropType_None;
}

bool AdInterface::object_can_drop(const QString &dn, const QString &target_dn) {
    DropType drop_type = get_drop_type(dn, target_dn);

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
            AdInterface::instance()->object_move(dn, target_dn);
            break;
        }
        case DropType_AddToGroup: {
            AdInterface::instance()->group_add_user(target_dn, dn);
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
        {"list", 1},
        {"get-attribute", 2},
        {"get-attribute-multi", 2},
    };

    const int arg_count = arg_count_map[command];
    if (args.size() - 1 != arg_count) {
        printf("Command \"%s\" needs %d arguments!\n", qPrintable(command), arg_count);

        return;
    }

    if (command == "list") {
        QString dn = args[1];

        QList<QString> children = list(dn);

        for (auto e : children) {
            printf("%s\n", qPrintable(e));
        }
    } else if (command == "get-attribute") {
        QString dn = args[1];
        QString attribute = args[2];

        QString value = attribute_get(dn, attribute);

        printf("%s\n", qPrintable(value));
    } else if (command == "get-attribute-multi") {
        QString dn = args[1];
        QString attribute = args[2];

        QList<QString> values = attribute_get_multi(dn, attribute);

        for (auto e : values) {
            printf("%s\n", qPrintable(e));
        }
    }
}

void AdInterface::update_cache(const QList<QString> &changed_dns) {
    // Remove objects that are affected by DN changes from cache
    // Next time the app requests info about these objects, they
    // will be reloaded into cache
    QSet<QString> removed_dns;
    for (auto changed_dn : changed_dns) {
        for (const QString &dn : attributes_cache.keys()) {
            // Remove if dn contains changed dn (is it's descendant)
            if (dn.contains(changed_dn)) {
                removed_dns.insert(dn);

                continue;
            }

            // Remove if object's attributes contain changed dn
            for (auto &values : attributes_cache[dn]) {
                for (auto &value : values) {
                    if (value.contains(changed_dn)) {
                        removed_dns.insert(dn);

                        goto break_outer;
                    }
                }
            }

            break_outer:;
        }
    }

    for (auto removed_dn : removed_dns) {
        attributes_cache.remove(removed_dn);
    }

    // NOTE: Suppress "not found" errors because after modifications
    // widgets might attempt to use outdated DN's which will cause
    // such errors but there is no point in showing those errors
    suppress_not_found_error = true;
    emit modified();
    suppress_not_found_error = false;
}

bool AdInterface::should_emit_status_message(int result) {
    const int ldap_result = connection->get_ldap_result();

    if (suppress_not_found_error && result == AD_LDAP_ERROR && ldap_result == LDAP_NO_SUCH_OBJECT) {
        return false;
    } else {
        return true;
    }
}

void AdInterface::success_status_message(const QString &msg, EmitStatusMessage emit_message) {
    if (emit_message == EmitStatusMessage_Yes) {
        emit status_message(msg, AdInterfaceMessageType_Success);
    }
}

void AdInterface::error_status_message(const QString &context, const QString &error, EmitStatusMessage emit_message) {
    if (emit_message == EmitStatusMessage_Yes) {
        const QString msg = QString(tr("%1. Error: \"%2\"")).arg(context, error);

        emit status_message(msg, AdInterfaceMessageType_Error);
    }
}

QString AdInterface::default_error_string(int ad_result) const {
    if (ad_result == AD_LDAP_ERROR) {
        const int ldap_result = connection->get_ldap_result();
        switch (ldap_result) {
            case LDAP_NO_SUCH_OBJECT: return tr("No such object");
            case LDAP_CONSTRAINT_VIOLATION: return tr("Constraint violation");
            case LDAP_UNWILLING_TO_PERFORM: return tr("Server is unwilling to perform");
            case LDAP_ALREADY_EXISTS: return tr("Already exists");
            default: return tr("Unknown LDAP error");
        }
    } else {
        switch (ad_result) {
            case AD_SUCCESS: return tr("AD success");
            case AD_ERROR: return tr("Generic AD error");
            case AD_INVALID_DN: return tr("Invalid DN");
            default: return tr("Unknown AD error");
        }
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

int get_account_option_bit(const AccountOption &option) {
    // NOTE: not all account options have bits
    switch (option) {
        case AccountOption_Disabled: return 0x0002;
        case AccountOption_DontExpirePassword: return 0x10000;
        case AccountOption_UseDesKey: return 0x200000;
        case AccountOption_SmartcardRequired: return 0x40000;
        case AccountOption_DontRequirePreauth: return 0x800000;
        case AccountOption_CantDelegate: return 0x100000;

        case AccountOption_PasswordExpired: return 0;
        case AccountOption_COUNT: return 0;
    }

    return 0;
}

bool datetime_is_never(const QString &attribute, const QString &value) {
    const DatetimeFormat format = get_attribute_time_format(attribute);

    if (format == DatetimeFormat_LargeInteger) {
        const bool is_never = (value == AD_LARGEINTEGERTIME_NEVER_1 || value == AD_LARGEINTEGERTIME_NEVER_2);
        
        return is_never;
    } else {
        return false;
    }
}

bool attribute_is_datetime(const QString &attribute) {
    const DatetimeFormat format = get_attribute_time_format(attribute);
    return format != DatetimeFormat_None;
}

DatetimeFormat get_attribute_time_format(const QString &attribute) {
    static const QHash<QString, DatetimeFormat> datetime_formats = {
        {ATTRIBUTE_ACCOUNT_EXPIRES, DatetimeFormat_LargeInteger},
        {ATTRIBUTE_WHEN_CREATED, DatetimeFormat_ISO8601},
        {ATTRIBUTE_WHEN_CHANGED, DatetimeFormat_ISO8601},
    };

    return datetime_formats.value(attribute, DatetimeFormat_None);
}

QString datetime_raw_to_string(const QString &attribute, const QString &raw_value) {
    if (datetime_is_never(attribute, raw_value)) {
        return "(never)";
    }

    const QDateTime datetime = datetime_raw_to_datetime(attribute, raw_value);
    const QString string = datetime.toString(DATETIME_DISPLAY_FORMAT);

    return string;
}

QDateTime datetime_raw_to_datetime(const QString &attribute, const QString &raw_value) {
    const DatetimeFormat format = get_attribute_time_format(attribute);

    switch (format) {
        case DatetimeFormat_LargeInteger: {
            // TODO: couldn't find epoch in qt, but maybe its hidden somewhere
            QDateTime datetime(QDate(1601, 1, 1));
            const qint64 hundred_nanos = raw_value.toLongLong();
            const qint64 millis = hundred_nanos / MILLIS_TO_100_NANOS;
            datetime = datetime.addMSecs(millis);

            return datetime;
        }
        case DatetimeFormat_ISO8601: {
            return QDateTime::fromString(raw_value, ISO8601_FORMAT_STRING);
        }
        case DatetimeFormat_None: {
            return QDateTime();
        }
    }

    return QDateTime();
}

AdResult::AdResult(bool success_arg) {
    success = success_arg;
    error = "";
}

AdResult::AdResult(bool success_arg, const QString &error_arg) {
    success = success_arg;
    error = error_arg;
}
