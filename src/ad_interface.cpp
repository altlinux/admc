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
#include "config.h"

#include "active_directory.h"
#include "admc.h"
#include "ad_connection.h"

#include <QSet>

// TODO: replace C active_directory.h with C++ version or other better version

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

// -----------------------------------------------------------------
// FAKE STUFF
// -----------------------------------------------------------------

AdInterface ad_interface;


// -----------------------------------------------------------------
// REAL STUFF
// -----------------------------------------------------------------

QMap<QString, QMap<QString, QList<QString>>> attributes_map;
QSet<QString> attributes_loaded;

bool ad_interface_login() {
    ADMC* app = ADMC::get_instance();
    adldap::AdConnection* conn = app->get_connection();
    conn->connect(SEARCH_BASE, HEAD_DN);
    return conn->is_connected();
}

QString get_error_str() {
    ADMC* app = ADMC::get_instance();
    adldap::AdConnection* conn = app->get_connection();
    return QString(conn->get_errstr());
}

// TODO: confirm that this encoding is ok
const char *qstring_to_cstr(const QString &qstr) {
    return qstr.toLatin1().constData();
}

QList<QString> load_children(const QString &dn) {
    const QByteArray dn_array = dn.toLatin1();
    const char *dn_cstr = dn_array.constData();
    char **children_raw = ad_list(dn_cstr, HEAD_DN);

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
        // TODO: is this still a fail if there are no children?
        emit ad_interface.load_children_failed(dn, get_error_str());

        return QList<QString>();
    }
}

void load_attributes(const QString &dn) {
    const QByteArray dn_array = dn.toLatin1();
    const char *dn_cstr = dn_array.constData();
    char** attributes_raw = ad_get_attribute(dn_cstr, "*", HEAD_DN);

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

        emit ad_interface.load_attributes_complete(dn);
    } else {
        emit ad_interface.load_attributes_failed(dn, get_error_str());
    }
}

QMap<QString, QList<QString>> get_attributes(const QString &dn) {
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

QList<QString> get_attribute_multi(const QString &dn, const QString &attribute) {
    QMap<QString, QList<QString>> attributes = get_attributes(dn);

    if (attributes.contains(attribute)) {
        return attributes[attribute];
    } else {
        return QList<QString>();
    }
}

QString get_attribute(const QString &dn, const QString &attribute) {
    QList<QString> values = get_attribute_multi(dn, attribute);

    if (values.size() > 0) {
        // Return first value only
        return values[0];
    } else {
        return "";
    }
}

bool attribute_value_exists(const QString &dn, const QString &attribute, const QString &value) {
    QList<QString> values = get_attribute_multi(dn, attribute);

    if (values.contains(value)) {
        return true;
    } else {
        return false;
    }
}

bool set_attribute(const QString &dn, const QString &attribute, const QString &value) {
    int result = AD_INVALID_DN;
    ADMC* app = ADMC::get_instance();
    adldap::AdConnection* adconn = app->get_connection();

    const QString old_value = get_attribute(dn, attribute);
    
    const QByteArray dn_array = dn.toLatin1();
    const char *dn_cstr = dn_array.constData();

    const QByteArray attribute_array = attribute.toLatin1();
    const char *attribute_cstr = attribute_array.constData();

    const QByteArray value_array = value.toLatin1();
    const char *value_cstr = value_array.constData();

    result = adconn->mod_replace(dn_cstr, attribute_cstr, value_cstr);

    if (result == AD_SUCCESS) {
        // Reload attributes to get new value
        load_attributes(dn);
        
        emit ad_interface.set_attribute_complete(dn, attribute, old_value, value);

        return true;
    } else {
        emit ad_interface.set_attribute_failed(dn, attribute, old_value, value, get_error_str());

        return false;
    }
}

// TODO: can probably make a create_anything() function with enum parameter
bool create_entry(const QString &name, const QString &dn, NewEntryType type) {
    int result = AD_INVALID_DN;
    ADMC* app = ADMC::get_instance();
    adldap::AdConnection* adconn = app->get_connection();
    
    const QByteArray name_array = name.toLatin1();
    const char *name_cstr = name_array.constData();

    const QByteArray dn_array = dn.toLatin1();
    const char *dn_cstr = dn_array.constData();

    switch (type) {
        case User: {
            result = adconn->create_user(name_cstr, dn_cstr);
            break;
        }
        case Computer: {
            result = adconn->create_computer(name_cstr, dn_cstr);
            break;
        }
        case OU: {
            result = adconn->ou_create(name_cstr, dn_cstr);
            break;
        }
        case Group: {
            result = adconn->group_create(name_cstr, dn_cstr);
            break;
        }
        case COUNT: break;
    }

    if (result == AD_SUCCESS) {
        emit ad_interface.create_entry_complete(dn, type);

        return true;
    } else {
        emit ad_interface.create_entry_failed(dn, type, adconn->get_errstr());

        return false;
    }
}

// Used to update membership when changes happen to entry
void reload_attributes_of_entry_groups(const QString &dn) {
    QList<QString> groups = get_attribute_multi(dn, "memberOf");

    for (auto group : groups) {
        // Only reload if loaded already
        if (attributes_map.contains(group)) {
            load_attributes(group);
        }
    }
}

void delete_entry(const QString &dn) {
    int result = AD_INVALID_DN;
    ADMC* app = ADMC::get_instance();
    adldap::AdConnection* adconn = app->get_connection();

    const QByteArray dn_array = dn.toLatin1();
    const char *dn_cstr = dn_array.constData();

    result = adconn->object_delete(dn_cstr);

    if (result == AD_SUCCESS) {
        reload_attributes_of_entry_groups(dn);

        attributes_map.remove(dn);
        attributes_loaded.remove(dn);

        emit ad_interface.delete_entry_complete(dn);
    } else {
        emit ad_interface.delete_entry_failed(dn, adconn->get_errstr());
    }
}

void move_user(const QString &user_dn, const QString &container_dn) {
    int result = AD_INVALID_DN;
    ADMC* app = ADMC::get_instance();
    adldap::AdConnection* adconn = app->get_connection();

    QString user_name = extract_name_from_dn(user_dn);
    QString new_dn = "CN=" + user_name + "," + container_dn;

    const QByteArray user_dn_array = user_dn.toLatin1();
    const char *user_dn_cstr = user_dn_array.constData();

    const QByteArray container_dn_array = container_dn.toLatin1();
    const char *container_dn_cstr = container_dn_array.constData();

    result = adconn->move_user(user_dn_cstr, container_dn_cstr);

    if (result == AD_SUCCESS) {
        // Unload attributes at old dn
        attributes_map.remove(user_dn);
        attributes_loaded.remove(user_dn);

        load_attributes(new_dn);
        reload_attributes_of_entry_groups(new_dn);

        emit ad_interface.move_user_complete(user_dn, container_dn, new_dn);
    } else {
        emit ad_interface.move_user_failed(user_dn, container_dn, new_dn, adconn->get_errstr());
    }
}

void add_user_to_group(const QString &group_dn, const QString &user_dn) {
    // TODO: currently getting object class violation error
    int result = AD_INVALID_DN;
    ADMC* app = ADMC::get_instance();
    adldap::AdConnection* adconn = app->get_connection();

    const QByteArray group_dn_array = group_dn.toLatin1();
    const char *group_dn_cstr = group_dn_array.constData();

    const QByteArray user_dn_array = user_dn.toLatin1();
    const char *user_dn_cstr = user_dn_array.constData();

    result = adconn->group_add_user(group_dn_cstr, user_dn_cstr);

    if (result == AD_SUCCESS) {
        // Reload attributes of group and user because group
        // operations affect attributes of both
        load_attributes(group_dn);
        load_attributes(user_dn);

        emit ad_interface.add_user_to_group_complete(group_dn, user_dn);
    } else {
        emit ad_interface.add_user_to_group_failed(group_dn, user_dn, adconn->get_errstr());
    }
}
