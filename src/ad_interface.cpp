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
#include "admc.h"

#include <QSet>

AdInterface::AdInterface(QObject *parent)
: QObject(parent)
{
    connection = new adldap::AdConnection();
}

AdInterface::~AdInterface() {
    delete connection;
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

void AdInterface::ad_interface_login(const QString &base, const QString &head) {
    connection->connect(base.toStdString(), head.toStdString());

    if (connection->is_connected()) {
        emit ad_interface_login_complete(base, head);
    } else {
        emit ad_interface_login_failed(base, head);
    }
}

QString AdInterface::get_error_str() {
    return QString(connection->get_errstr());
}

QList<QString> AdInterface::load_children(const QString &dn) {
    const QByteArray dn_array = dn.toLatin1();
    const char *dn_cstr = dn_array.constData();

    char **children_raw = connection->list(dn_cstr);

    if (children_raw != NULL) {
        auto children = QList<QString>();

        for (int i = 0; children_raw[i] != NULL; i++) {
            auto child = QString(children_raw[i]);
            children.push_back(child);
        }

        for (int i = 0; children_raw[i] != NULL; i++) {
            free(children_raw[i]);
        }
        free(children_raw);

        return children;
    } else {
        if (connection->get_errcode() != AD_SUCCESS) {
            emit load_children_failed(dn, get_error_str());
        }

        return QList<QString>();
    }
}

void AdInterface::load_attributes(const QString &dn) {
    const QByteArray dn_array = dn.toLatin1();
    const char *dn_cstr = dn_array.constData();

    char** attributes_raw = connection->get_attribute(dn_cstr, "*");

    if (attributes_raw != NULL) {
        attributes_map[dn] = QMap<QString, QList<QString>>();

        // Load attributes map
        // attributes_raw is in the form of:
        // char** array of {key, value, value, key, value ...}
        // transform it into:
        // map of {key => {value, value ...}, key => {value, value ...} ...}
        for (int i = 0; attributes_raw[i + 2] != NULL; i += 2) {
            auto attribute = QString(attributes_raw[i]);
            auto value = QString(attributes_raw[i + 1]);

            // Make values list if doesn't exist yet
            if (!attributes_map[dn].contains(attribute)) {
                attributes_map[dn][attribute] = QList<QString>();
            }

            attributes_map[dn][attribute].push_back(value);
        }

        // Free attributes_raw
        for (int i = 0; attributes_raw[i] != NULL; i++) {
            free(attributes_raw[i]);
        }
        free(attributes_raw);

        attributes_loaded.insert(dn);

        emit load_attributes_complete(dn);
    } else if (connection->get_errcode() != AD_SUCCESS) {
        emit load_attributes_failed(dn, get_error_str());
    }
}

QMap<QString, QList<QString>> AdInterface::get_attributes(const QString &dn) {
    // First check whether load_attributes was ever called on this dn
    // If it hasn't, attempt to load attributes
    // After that return whatever attributes are now loaded for this dn
    if (!attributes_loaded.contains(dn)) {
        load_attributes(dn);
    }

    if (!attributes_map.contains(dn)) {
        return QMap<QString, QList<QString>>();
    } else {
        return attributes_map[dn];
    }
}

QList<QString> AdInterface::get_attribute_multi(const QString &dn, const QString &attribute) {
    QMap<QString, QList<QString>> attributes = get_attributes(dn);

    if (attributes.contains(attribute)) {
        return attributes[attribute];
    } else {
        return QList<QString>();
    }
}

QString AdInterface::get_attribute(const QString &dn, const QString &attribute) {
    QList<QString> values = get_attribute_multi(dn, attribute);

    if (values.size() > 0) {
        // Return first value only
        return values[0];
    } else {
        return "";
    }
}

bool AdInterface::attribute_value_exists(const QString &dn, const QString &attribute, const QString &value) {
    QList<QString> values = get_attribute_multi(dn, attribute);

    if (values.contains(value)) {
        return true;
    } else {
        return false;
    }
}

bool AdInterface::set_attribute(const QString &dn, const QString &attribute, const QString &value) {
    int result = AD_INVALID_DN;

    const QString old_value = get_attribute(dn, attribute);
    
    const QByteArray dn_array = dn.toLatin1();
    const char *dn_cstr = dn_array.constData();

    const QByteArray attribute_array = attribute.toLatin1();
    const char *attribute_cstr = attribute_array.constData();

    const QByteArray value_array = value.toLatin1();
    const char *value_cstr = value_array.constData();

    result = connection->mod_replace(dn_cstr, attribute_cstr, value_cstr);

    if (result == AD_SUCCESS) {
        // Reload attributes to get new value
        load_attributes(dn);
        
        emit set_attribute_complete(dn, attribute, old_value, value);

        return true;
    } else {
        emit set_attribute_failed(dn, attribute, old_value, value, get_error_str());

        return false;
    }
}

// TODO: can probably make a create_anything() function with enum parameter
bool AdInterface::create_entry(const QString &name, const QString &dn, NewEntryType type) {
    int result = AD_INVALID_DN;
    
    const QByteArray name_array = name.toLatin1();
    const char *name_cstr = name_array.constData();

    const QByteArray dn_array = dn.toLatin1();
    const char *dn_cstr = dn_array.constData();

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
            result = connection->ou_create(name_cstr, dn_cstr);
            break;
        }
        case Group: {
            result = connection->group_create(name_cstr, dn_cstr);
            break;
        }
        case COUNT: break;
    }

    if (result == AD_SUCCESS) {
        emit create_entry_complete(dn, type);

        return true;
    } else {
        emit create_entry_failed(dn, type, get_error_str());

        return false;
    }
}

// Used to update membership when changes happen to entry
void AdInterface::reload_attributes_of_entry_groups(const QString &dn) {
    QList<QString> groups = get_attribute_multi(dn, "memberOf");

    for (auto group : groups) {
        // Only reload if loaded already
        if (attributes_map.contains(group)) {
            load_attributes(group);
        }
    }
}

void AdInterface::delete_entry(const QString &dn) {
    int result = AD_INVALID_DN;

    const QByteArray dn_array = dn.toLatin1();
    const char *dn_cstr = dn_array.constData();

    result = connection->object_delete(dn_cstr);

    if (result == AD_SUCCESS) {
        reload_attributes_of_entry_groups(dn);

        attributes_map.remove(dn);
        attributes_loaded.remove(dn);

        emit delete_entry_complete(dn);
    } else {
        emit delete_entry_failed(dn, get_error_str());
    }
}

void AdInterface::move_user(const QString &user_dn, const QString &container_dn) {
    int result = AD_INVALID_DN;

    QString user_name = extract_name_from_dn(user_dn);
    QString new_dn = "CN=" + user_name + "," + container_dn;

    const QByteArray user_dn_array = user_dn.toLatin1();
    const char *user_dn_cstr = user_dn_array.constData();

    const QByteArray container_dn_array = container_dn.toLatin1();
    const char *container_dn_cstr = container_dn_array.constData();

    result = connection->move_user(user_dn_cstr, container_dn_cstr);

    if (result == AD_SUCCESS) {
        // Unload attributes at old dn
        attributes_map.remove(user_dn);
        attributes_loaded.remove(user_dn);

        load_attributes(new_dn);
        reload_attributes_of_entry_groups(new_dn);

        emit move_user_complete(user_dn, container_dn, new_dn);
    } else {
        emit move_user_failed(user_dn, container_dn, new_dn, get_error_str());
    }
}

void AdInterface::add_user_to_group(const QString &group_dn, const QString &user_dn) {
    int result = AD_INVALID_DN;

    const QByteArray group_dn_array = group_dn.toLatin1();
    const char *group_dn_cstr = group_dn_array.constData();

    const QByteArray user_dn_array = user_dn.toLatin1();
    const char *user_dn_cstr = user_dn_array.constData();

    result = connection->group_add_user(group_dn_cstr, user_dn_cstr);

    if (result == AD_SUCCESS) {
        // Reload attributes of group and user because group
        // operations affect attributes of both
        load_attributes(group_dn);
        load_attributes(user_dn);

        emit add_user_to_group_complete(group_dn, user_dn);
    } else {
        emit add_user_to_group_failed(group_dn, user_dn, get_error_str());
    }
}

void AdInterface::rename(const QString &dn, const QString &new_name) {
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

    int result = AD_INVALID_DN;
    if (is_user(dn)) {
        result = connection->rename_user(dn_cstr, new_name_cstr);
    } else if (is_group(dn)) {
        result = connection->rename_group(dn_cstr, new_name_cstr);
    } else {
        result = connection->rename(dn_cstr, new_rdn_cstr);
    }

    if (result == AD_SUCCESS) {
        load_attributes(new_dn);
        reload_attributes_of_entry_groups(new_dn);

        emit rename_complete(dn, new_name, new_dn);
    } else {
        emit rename_failed(dn, new_name, new_dn, get_error_str());
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

bool AdInterface::can_drop_entry(const QString &dn, const QString &parent_dn) {
    const bool dropped_is_user = AD()->is_user(dn);

    const bool parent_is_group = AD()->is_group(parent_dn);
    const bool parent_is_ou = AD()->is_ou(parent_dn);
    const bool parent_is_container = AD()->is_container(parent_dn);

    // TODO: support dropping non-users
    // TODO: support dropping policies
    if (parent_dn == "") {
        return false;
    } else if (dropped_is_user && (parent_is_group || parent_is_ou || parent_is_container)) {
        return true;
    } else {
        return false;
    }
}

// General "drop" operation that can either move, link or change membership depending on which types of entries are involved
void AdInterface::drop_entry(const QString &dn, const QString &parent_dn) {
    const bool dropped_is_user = AD()->is_user(dn);

    const bool parent_is_group = AD()->is_group(parent_dn);
    const bool parent_is_ou = AD()->is_ou(parent_dn);
    const bool parent_is_container = AD()->is_container(parent_dn);

    if (dropped_is_user && (parent_is_ou || parent_is_container)) {
        AD()->move_user(dn, parent_dn);
    } else if (dropped_is_user && parent_is_group) {
        AD()->add_user_to_group(parent_dn, dn);
    }
}

AdInterface *AD() {
    ADMC *app = qobject_cast<ADMC *>(qApp);
    AdInterface *ad = app->ad_interface();
    return ad;
}
