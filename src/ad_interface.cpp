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
        // emit get_domain_hosts_failed(domain, site, get_error_str());

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

        return AdResult(true, "");
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

bool AdInterface::attribute_value_exists(const QString &dn, const QString &attribute, const QString &value) {
    QList<QString> values = attribute_get_multi(dn, attribute);

    if (values.contains(value)) {
        return true;
    } else {
        return false;
    }
}

AdResult AdInterface::attribute_replace(const QString &dn, const QString &attribute, const QString &value) {
    const QString old_value = attribute_get(dn, attribute);
    
    const QByteArray dn_array = dn.toLatin1();
    const char *dn_cstr = dn_array.constData();

    const QByteArray attribute_array = attribute.toLatin1();
    const char *attribute_cstr = attribute_array.constData();

    const QByteArray value_array = value.toLatin1();
    const char *value_cstr = value_array.constData();

    int result = connection->attribute_replace(dn_cstr, attribute_cstr, value_cstr);

    const QString name = extract_name_from_dn(dn);

    if (result == AD_SUCCESS) {
        success_status_message(QString(tr("Changed attribute \"%1\" of \"%2\" from \"%3\" to \"%4\"")).arg(attribute, name, old_value, value));

        update_cache({dn});

        return AdResult(true, "");
    } else {
        const QString context = QString(tr("Failed to change attribute \"%1\" of object \"%2\" from \"%3\" to \"%4\"")).arg(attribute, name, old_value, value);
        const QString error_string = default_error_string(result);

        error_status_message(context, error_string);

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

        return AdResult(true, "");
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

        return AdResult(true, "");
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

        return AdResult(true, "");
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

        return AdResult(true, "");
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

        return AdResult(true, "");
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

        return AdResult(true, "");
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

        return AdResult(true, "");
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

AdResult AdInterface::user_set_disabled(const QString &dn, bool disabled) {
    const QByteArray dn_array = dn.toLatin1();
    const char *dn_cstr = dn_array.constData();

    QString control = attribute_get(dn, ATTRIBUTE_USER_ACCOUNT_CONTROL);
    if (control.isEmpty()) {
        control = "0";
    }

    int control_int = control.toInt();
    if (disabled) {
        control_int |= 2;
    } else {
        control_int ^= 2;
    }

    const QString control_updated = QString::number(control_int);
    const QByteArray control_updated_array = control_updated.toLatin1();
    const char *control_updated_cstr = control_updated_array.constData();

    const int result = connection->attribute_replace(dn_cstr, ATTRIBUTE_USER_ACCOUNT_CONTROL, control_updated_cstr);

    const QString name = extract_name_from_dn(dn);
    
    if (result == AD_SUCCESS) {
        QString context;
        if (disabled) {
            context = QString(tr("Disabled user - \"%1\"")).arg(name);
        } else {
            context = QString(tr("Enabled user - \"%1\"")).arg(name);
        }
        success_status_message(context);

        update_cache({dn});

        return AdResult(true, "");
    } else {
        QString context;
        if (disabled) {
            context = QString(tr("Failed to disable user - \"%1\"")).arg(name);
        } else {
            context = QString(tr("Failed to enable user - \"%1\"")).arg(name);
        }

        const QString error_string = default_error_string(result);

        error_status_message(context, error_string);

        return AdResult(false, error_string);
    }
}

bool AdInterface::is_user(const QString &dn) {
    return attribute_value_exists(dn, "objectClass", "user");
}

bool AdInterface::is_group(const QString &dn) {
    return attribute_value_exists(dn, "objectClass", "group");
}

bool AdInterface::is_container(const QString &dn) {
    return attribute_value_exists(dn, "objectClass", "container");
}

bool AdInterface::is_ou(const QString &dn) {
    return attribute_value_exists(dn, "objectClass", "organizationalUnit");
}

bool AdInterface::is_policy(const QString &dn) {
    return attribute_value_exists(dn, "objectClass", "groupPolicyContainer");
}

bool AdInterface::is_container_like(const QString &dn) {
    // TODO: check that this includes all fitting objectClasses
    const QList<QString> containerlike_objectClasses = {"organizationalUnit", "builtinDomain", "domain"};
    for (auto c : containerlike_objectClasses) {
        if (AdInterface::instance()->attribute_value_exists(dn, "objectClass", c)) {
            return true;
        }
    }

    return false;
}

bool AdInterface::user_is_disabled(const QString &dn) {
    const QString control = attribute_get(dn, ATTRIBUTE_USER_ACCOUNT_CONTROL);
    if (control.isEmpty()) {
        return false;
    }

    const int control_int = control.toInt();
    const bool disabled = ((control_int & 2) != 0);
    return disabled;
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

void AdInterface::success_status_message(const QString &msg) {
    emit status_message(msg, AdInterfaceMessageType_Success);
}

void AdInterface::error_status_message(const QString &context, const QString &error) {
    const QString msg = QString(tr("%1. Error: \"%2\"")).arg(context, error);

    emit status_message(msg, AdInterfaceMessageType_Error);
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

AdResult::AdResult(bool success_arg, const QString &msg_arg) {
    success = success_arg;
    msg = msg_arg;
}
