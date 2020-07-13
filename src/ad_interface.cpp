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

void AdInterface::login(const QString &base, const QString &head) {
    const int result = connection->connect(base.toStdString(), head.toStdString());

    if (result == AD_SUCCESS) {
        message(QString("Logged in to \"%1\" with head dn at \"%2\"").arg(base, head));

        emit logged_in();
    } else {
        message(QString("Failed to login to \"%1\" with head dn at \"%2\"").arg(base, head));
    }
}

QString AdInterface::get_error_str() {
    return QString(connection->get_error());
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
        message(QString("Failed to load children of \"%1\". Error: \"%2\"").arg(dn, get_error_str()));

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
        message(QString("Failed to search for \"%1\". Error: \"%2\"").arg(filter, get_error_str()));

        return QList<QString>();
    }
}

Attributes AdInterface::get_all_attributes(const QString &dn) {
    if (dn == "") {
        return Attributes();
    }

    // Load attributes if it's not in cache
    if (!attributes_cache.contains(dn)) {
        const QByteArray dn_array = dn.toLatin1();
        const char *dn_cstr = dn_array.constData();

        char** attributes_raw;
        const int result_attribute_get = connection->attribute_get(dn_cstr, "*", &attributes_raw);
        if (result_attribute_get == AD_SUCCESS) {
            Attributes attributes;
            // attributes_raw is in the form of:
            // char** array of {key, value, value, key, value ...}
            // transform it into:
            // map of {key => {value, value ...}, key => {value, value ...} ...}
            for (int i = 0; attributes_raw[i + 2] != NULL; i += 2) {
                auto attribute = QString(attributes_raw[i]);
                auto value = QString(attributes_raw[i + 1]);

                // Make values list if doesn't exist yet
                if (!attributes.contains(attribute)) {
                    attributes[attribute] = QList<QString>();
                }

                attributes[attribute].push_back(value);
            }

            ad_array_free(attributes_raw);

            attributes_cache[dn] = attributes;
        } else {
            message(QString("Failed to get attributes of \"%1\"").arg(dn));

            return Attributes();
        }
    }

    return attributes_cache[dn];
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

bool AdInterface::attribute_replace(const QString &dn, const QString &attribute, const QString &value) {
    int result = AD_INVALID_DN;

    const QString old_value = attribute_get(dn, attribute);
    
    const QByteArray dn_array = dn.toLatin1();
    const char *dn_cstr = dn_array.constData();

    const QByteArray attribute_array = attribute.toLatin1();
    const char *attribute_cstr = attribute_array.constData();

    const QByteArray value_array = value.toLatin1();
    const char *value_cstr = value_array.constData();

    result = connection->attribute_replace(dn_cstr, attribute_cstr, value_cstr);

    if (result == AD_SUCCESS) {
        message(QString("Changed attribute \"%1\" of \"%2\" from \"%3\" to \"%4\"").arg(attribute, dn, old_value, value));

        update_cache();

        return true;
    } else {
        message(QString("Failed to change attribute \"%1\" of entry \"%2\" from \"%3\" to \"%4\". Error: \"%5\"").arg(attribute, dn, old_value, value, get_error_str()));

        return false;
    }
}

// TODO: can probably make a create_anything() function with enum parameter
bool AdInterface::object_create(const QString &name, const QString &dn, NewEntryType type) {
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
            result = connection->create_ou(name_cstr, dn_cstr);
            break;
        }
        case Group: {
            result = connection->create_group(name_cstr, dn_cstr);
            break;
        }
        case COUNT: break;
    }

    const QString type_str = new_entry_type_to_string[type];

    if (result == AD_SUCCESS) {
        message(QString("Created entry \"%1\" of type \"%2\"").arg(dn, type_str));

        update_cache();

        return true;
    } else {
        message(QString("Failed to create entry \"%1\" of type \"%2\". Error: \"%3\"").arg(dn, type_str, get_error_str()));

        return false;
    }
}

void AdInterface::object_delete(const QString &dn) {
    int result = AD_INVALID_DN;

    const QByteArray dn_array = dn.toLatin1();
    const char *dn_cstr = dn_array.constData();

    result = connection->object_delete(dn_cstr);

    if (result == AD_SUCCESS) {
        message(QString("Deleted entry \"%1\"").arg(dn));

        update_cache();
    } else {
        message(QString("Failed to delete entry \"%1\". Error: \"%2\"").arg(dn, get_error_str()));
    }
}

void AdInterface::object_move(const QString &dn, const QString &new_container) {
    int result = AD_INVALID_DN;

    QList<QString> dn_split = dn.split(',');
    QString new_dn = dn_split[0] + "," + new_container;

    const QByteArray dn_array = dn.toLatin1();
    const char *dn_cstr = dn_array.constData();

    const QByteArray new_container_array = new_container.toLatin1();
    const char *new_container_cstr = new_container_array.constData();

    const bool entry_is_user = is_user(dn);
    if (entry_is_user) {
        result = connection->move_user(dn_cstr, new_container_cstr);
    } else {
        result = connection->move(dn_cstr, new_container_cstr);
    }

    // TODO: drag and drop handles checking move compatibility but need
    // to do this here as well for CLI?
    
    if (result == AD_SUCCESS) {
        message(QString("Moved \"%1\" to \"%2\"").arg(dn).arg(new_container));

        update_cache();
    } else {
        message(QString("Failed to move \"%1\" to \"%2\". Error: \"%3\"").arg(dn, new_container, get_error_str()));
    }
}

void AdInterface::group_add_user(const QString &group_dn, const QString &user_dn) {
    int result = AD_INVALID_DN;

    const QByteArray group_dn_array = group_dn.toLatin1();
    const char *group_dn_cstr = group_dn_array.constData();

    const QByteArray user_dn_array = user_dn.toLatin1();
    const char *user_dn_cstr = user_dn_array.constData();

    result = connection->group_add_user(group_dn_cstr, user_dn_cstr);

    if (result == AD_SUCCESS) {
        message(QString("Added user \"%1\" to group \"%2\"").arg(user_dn, group_dn));

        update_cache();
    } else {
        message(QString("Failed to add user \"%1\" to group \"%2\". Error: \"%3\"").arg(user_dn, group_dn, get_error_str()));
    }
}

void AdInterface::group_remove_user(const QString &group_dn, const QString &user_dn) {
    int result = AD_INVALID_DN;

    const QByteArray group_dn_array = group_dn.toLatin1();
    const char *group_dn_cstr = group_dn_array.constData();

    const QByteArray user_dn_array = user_dn.toLatin1();
    const char *user_dn_cstr = user_dn_array.constData();

    result = connection->group_remove_user(group_dn_cstr, user_dn_cstr);

    if (result == AD_SUCCESS) {
        message(QString("Removed user \"%1\" from group \"%2\"").arg(user_dn, group_dn));

        update_cache();
    } else {
        message(QString("Failed to remove user \"%1\" from group \"%2\". Error: \"%3\"").arg(user_dn, group_dn, get_error_str()));
    }
}

void AdInterface::object_rename(const QString &dn, const QString &new_name) {
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
        message(QString("Renamed \"%1\" to \"%2\"").arg(dn, new_name));

        update_cache();
    } else {
        message(QString("Failed to rename \"%1\" to \"%2\". Error: \"%3\"").arg(dn, new_name, get_error_str()));
    }
}

bool AdInterface::set_pass(const QString &dn, const QString &password) {
    const QByteArray dn_array = dn.toLatin1();
    const char *dn_cstr = dn_array.constData();
    const QByteArray password_array = password.toLatin1();
    const char *password_cstr = password_array.constData();

    int result = connection->setpass(dn_cstr, password_cstr);

    if (result == AD_SUCCESS) {
        message(QString("Set pass of \"%1\"").arg(dn));

        update_cache();

        return true;
    } else {
        message(QString("Failed to set pass of \"%1\". Error: \"%2\"").arg(dn, get_error_str()));
    
        return false;
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
        if (AD()->attribute_value_exists(dn, "objectClass", c)) {
            return true;
        }
    }

    return false;
}

enum DropType {
    DropType_Move,
    DropType_AddToGroup,
    DropType_None
};

// Determine what kind of drop type is dropping this entry onto target
// If drop type is none, then can't drop this entry on this target
DropType get_drop_type(const QString &dn, const QString &target_dn) {
    if (dn == target_dn) {
        return DropType_None;
    }

    const bool dropped_is_user = AD()->is_user(dn);
    const bool dropped_is_group = AD()->is_group(dn);
    const bool dropped_is_ou = AD()->is_ou(dn);

    const bool target_is_user = AD()->is_user(target_dn);
    const bool target_is_group = AD()->is_group(target_dn);
    const bool target_is_ou = AD()->is_ou(target_dn);
    const bool target_is_container = AD()->is_container(target_dn);

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

// General "drop" operation that can either move, link or change membership depending on which types of entries are involved
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

void AdInterface::update_cache() {
    attributes_cache.clear();

    emit modified();
}

AdInterface *AD() {
    ADMC *app = qobject_cast<ADMC *>(qApp);
    AdInterface *ad = app->ad_interface();
    return ad;
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
