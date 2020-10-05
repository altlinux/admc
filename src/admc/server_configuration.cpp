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

#include "server_configuration.h"
#include "ad_interface.h"
#include "settings.h"
#include "utils.h"

#include <QHash>
#include <QLocale>
#include <QDebug>
#include <algorithm>

#define ATTRIBUTE_ATTRIBUTE_DISPLAY_NAMES         "attributeDisplayNames"
#define ATTRIBUTE_EXTRA_COLUMNS         "extraColumns"
#define ATTRIBUTE_FILTER_CONTAINERS     "msDS-FilterContainers"
#define ATTRIBUTE_LDAP_DISPLAY_NAME     "lDAPDisplayName"
#define ATTRIBUTE_ADMIN_DISPLAY_NAME    "adminDisplayName"
#define ATTRIBUTE_POSSIBLE_SUPERIORS    "systemPossSuperiors"
#define ATTRIBUTE_ATTRIBUTE_SYNTAX      "attributeSyntax"
#define ATTRIBUTE_OM_SYNTAX             "oMSyntax"
#define CLASS_DISPLAY_NAME              "classDisplayName"
#define TREAT_AS_LEAF                   "treatAsLeaf"
#define ATTRIBUTE_MAY_CONTAIN           "mayContain"
#define ATTRIBUTE_SYSTEM_MAY_CONTAIN    "systemMayContain"

QString get_display_specifier_class(const QString &display_specifier);
QString ldap_name_to_ad_name(const QString &ldap_class_name);

QString get_locale_dir() {
    static QString locale_dir =
    []() {
        const QString locale_code =
        []() {
            const QLocale saved_locale = Settings::instance()->get_variant(VariantSetting_Locale).toLocale();

            if (saved_locale.language() == QLocale::Russian) {
                return "419";
            } else {
                    // English
                return "409";
            }
        }();

        const QString configuration_dn = AdInterface::instance()->configuration_dn();

        return QString("CN=%1,CN=DisplaySpecifiers,%2").arg(locale_code, configuration_dn);
    }();

    return locale_dir;
}

QString get_attribute_display_name(const QString &attribute, const QString &objectClass) {
    // { objectClass => { attribute => display_name } }
    static QHash<QString, QHash<QString, QString>> attribute_display_names =
    []() {
        QHash<QString, QHash<QString, QString>> attribute_display_names_out;

        const QString locale_dir = get_locale_dir();
        const QList<QString> search_attributes = {ATTRIBUTE_ATTRIBUTE_DISPLAY_NAMES, ATTRIBUTE_EXTRA_COLUMNS};
        const QHash<QString, Attributes> search_results = AdInterface::instance()->search("", search_attributes, SearchScope_Children, locale_dir);

        for (const QString &dn : search_results.keys()) {
            const Attributes attributes = search_results[dn];

            const QList<QString> display_names =
            [dn, attributes]() {
                QList<QString> out = attribute_get_strings(attributes, ATTRIBUTE_ATTRIBUTE_DISPLAY_NAMES);

                // NOTE: default display specifier contains some extra display names that are used for contents columns
                if (dn.contains("default-Display")) {
                    const QList<QString> extra_display_names = attribute_get_strings(attributes, ATTRIBUTE_EXTRA_COLUMNS);

                    out.append(extra_display_names);
                }

                return out;
            }();

            const QString specifier_class = get_display_specifier_class(dn);

            for (const auto display_name : display_names) {
                const QList<QString> display_name_split = display_name.split(",");
                const QString attribute_name = display_name_split[0];
                const QString attribute_display_name = display_name_split[1];

                attribute_display_names_out[specifier_class][attribute_name] = attribute_display_name;
            }
        }

        return attribute_display_names_out;
    }();

    // TODO: all objects have name attribute, but not all have cn. There is no display name for name though, only for cn.
    if (attribute == ATTRIBUTE_NAME) {
        return get_attribute_display_name(ATTRIBUTE_CN, objectClass);
    }

    // NOTE: try to find display name in default display specifier last
    const QList<QString> objectClasses = {objectClass, CLASS_DEFAULT};

    for (const QString &the_class : objectClasses) {
        if (attribute_display_names.contains(the_class) && attribute_display_names[the_class].contains(attribute)) {
            const QString display_name = attribute_display_names[the_class][attribute];

            return display_name;
        }
    }

    // NOTE: display specifier doesn't contain all display names, so need to hardcode some of them here
    static const QHash<QString, QString> fallback_strings = {
        {ATTRIBUTE_DISTINGUISHED_NAME, QObject::tr("DN")},
        {ATTRIBUTE_OBJECT_CLASS, QObject::tr("Object class")},
        {ATTRIBUTE_WHEN_CREATED, QObject::tr("Created")},
        {ATTRIBUTE_WHEN_CHANGED, QObject::tr("Changed")},
        {ATTRIBUTE_USN_CREATED, QObject::tr("USN created")},
        {ATTRIBUTE_USN_CHANGED, QObject::tr("USN changed")},
        {ATTRIBUTE_ACCOUNT_EXPIRES, QObject::tr("Account expires")},
        {ATTRIBUTE_OBJECT_CATEGORY, QObject::tr("Type")},
    };

    if (fallback_strings.contains(attribute)) {
        return fallback_strings[attribute];
    } else {
        return attribute;
    }
}

QString get_class_display_name(const QString &objectClass) {
    static QHash<QString, QString> class_display_names =
    []() {
        QHash<QString, QString> out;

        const QString locale_dir = get_locale_dir();
        const QList<QString> search_attributes = {CLASS_DISPLAY_NAME};
        const QHash<QString, Attributes> search_results = AdInterface::instance()->search("", search_attributes, SearchScope_Children, locale_dir);

        for (const QString &dn : search_results.keys()) {
            // TODO: duplicated code. Probably load this together with other display specifier info.
            const QString specifier_class = get_display_specifier_class(dn);

            const Attributes attributes = search_results[dn];
            const QString class_display_name = attribute_get_string(attributes, CLASS_DISPLAY_NAME);

            out[specifier_class] = class_display_name;
        }

        return out;
    }();

    if (class_display_names.contains(objectClass)) {
        return class_display_names[objectClass];
    } else {
        return objectClass;
    }
}

QList<QString> get_extra_contents_columns() {
    static const QList<QString> columns =
    []() {
        const QList<QString> columns_values =
        []() {
            const QString locale_dir = get_locale_dir();
            const QString default_display_specifier = QString("CN=default-Display,%1").arg(locale_dir);
            QList<QString> columns_out = AdInterface::instance()->attribute_request_strings(default_display_specifier, ATTRIBUTE_EXTRA_COLUMNS);
            std::reverse(columns_out.begin(), columns_out.end());

            return columns_out;
        }();

        QList<QString> out;
        for (const QString &value : columns_values) {
            const QList<QString> column_split = value.split(',');
            const QString attribute = column_split[0];

            out.append(attribute);
        }

        return out;
    }();

    return columns;
}

QList<QString> get_containers_filter_classes() {
    static const QList<QString> classes =
    []() {
        const QString locale_dir = get_locale_dir();
        const QString display_specifier = QString("CN=DS-UI-Default-Settings,%1").arg(locale_dir);
        QList<QString> ms_classes = AdInterface::instance()->attribute_request_strings(display_specifier, ATTRIBUTE_FILTER_CONTAINERS);

        // NOTE: ATTRIBUTE_FILTER_CONTAINERS contains classes in non-LDAP format ("Organizational-Unit" vs "organizationalUnit"). Convert to LDAP format by getting ATTRIBUTE_LDAP_DISPLAY_NAME from class' schema.
        const QString schema_dn = AdInterface::instance()->schema_dn();

        QList<QString> out;
        for (const auto ms_class : ms_classes) {
            const QString class_dn = QString("CN=%1,%2").arg(ms_class, schema_dn);
            const Attributes attributes = AdInterface::instance()->attribute_request(class_dn, {ATTRIBUTE_LDAP_DISPLAY_NAME});
            const QString ldap_class = attribute_get_string(attributes, ATTRIBUTE_LDAP_DISPLAY_NAME);

            if (!ldap_class.isEmpty()) {
                out.append(ldap_class);
            }
        }

        // TODO: why is this not included???
        out.append(CLASS_DOMAIN);

        return out;
    }();

    return classes;
}

QList<QString> get_possible_superiors(const Attributes &attributes) {
    const QString category = attribute_get_value(attributes, ATTRIBUTE_OBJECT_CATEGORY);

    static QHash<QString, QList<QString>> possible_superiors_map;

    if (!possible_superiors_map.contains(category)) {
        const QList<QString> possible_superiors = AdInterface::instance()->attribute_request_strings(category, ATTRIBUTE_POSSIBLE_SUPERIORS);

        possible_superiors_map[category] = possible_superiors;
    }

    return possible_superiors_map[category];
}

// Display specifier DN is "CN=class-Display,CN=..."
// Get "class" from that
QString get_display_specifier_class(const QString &display_specifier) {
    const QString rdn = display_specifier.split(",")[0];
    const QString removed_cn = QString(rdn).remove("CN=", Qt::CaseInsensitive);
    const QString specifier_class = removed_cn.split('-')[0];

    return specifier_class;
}

QString ldap_name_to_ad_name(const QString &ldap_name) {
    static QHash<QString, QString> ldap_to_ad;

    if (!ldap_to_ad.contains(ldap_name)) {
        const QString schema_dn = AdInterface::instance()->schema_dn();

        const QString filter = filter_EQUALS(ATTRIBUTE_LDAP_DISPLAY_NAME, ldap_name);
        const QList<QString> search_results = AdInterface::instance()->search_dns(filter, schema_dn);

        if (!search_results.isEmpty()) {
            const QString schema_object = search_results[0];
            const QString ad_class_name = AdInterface::instance()->attribute_request_value(schema_object, ATTRIBUTE_ADMIN_DISPLAY_NAME);

            ldap_to_ad[ldap_name] = ad_class_name;
        } else {
            ldap_to_ad[ldap_name] = "";
        }
    }

    return ldap_to_ad[ldap_name];
}

// TODO: Object's objectClass list appears to already contain the full inheritance chain. Confirm that this applies to all objects, because otherwise would need to manually go down the inheritance chain to get all possible attributes.
QList<QString> get_possible_attributes(const Attributes &attributes) {
    static QHash<QString, QList<QString>> class_possible_attributes;

    const QList<QString> object_classes = attribute_get_strings(attributes, ATTRIBUTE_OBJECT_CLASS);

    QList<QString> possible_attributes;
    for (const QString object_class : object_classes) {
        if (!class_possible_attributes.contains(object_class)) {
            // Load possible attributes from schema
            const QString ad_class_name = ldap_name_to_ad_name(object_class);
            const QString schema_dn = AdInterface::instance()->schema_dn();
            const QString class_dn = QString("CN=%1,%2").arg(ad_class_name, schema_dn);
            const Attributes class_attributes = AdInterface::instance()->attribute_request_all(class_dn);

            if (class_attributes.isEmpty()) {
                continue;
            }

            const QList<QString> may_contain = attribute_get_strings(class_attributes, ATTRIBUTE_MAY_CONTAIN);
            const QList<QString> system_may_contain = attribute_get_strings(class_attributes, ATTRIBUTE_SYSTEM_MAY_CONTAIN);

            QList<QString> total_contain;
            total_contain.append(may_contain);
            total_contain.append(system_may_contain);

            class_possible_attributes[object_class] = total_contain;
        }

        const QList<QString> for_this_class = class_possible_attributes[object_class];
        possible_attributes.append(for_this_class);
    }

    return possible_attributes;
}

AttributeType get_attribute_type(const QString &attribute) {
    static QHash<QString, AttributeType> attribute_type_map;

    if (!attribute_type_map.contains(attribute)) {
        const QString attribute_ad_name = ldap_name_to_ad_name(attribute);
        const QString schema_dn = AdInterface::instance()->schema_dn();
        const QString class_dn = QString("CN=%1,%2").arg(attribute_ad_name, schema_dn);
        const Attributes class_attributes = AdInterface::instance()->attribute_request_all(class_dn);

        if (!class_attributes.isEmpty()) {
            const QString attribute_syntax = attribute_get_value(class_attributes, ATTRIBUTE_ATTRIBUTE_SYNTAX);
            const QString om_syntax = attribute_get_value(class_attributes, ATTRIBUTE_OM_SYNTAX);

            // NOTE: replica of: https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-adts/7cda533e-d7a4-4aec-a517-91d02ff4a1aa
            // attribute_syntax -> om syntax list -> type
            // TODO: is there a lib for this?
            // TODO: there are Object(x) types, not sure if need those
            static QHash<QString, QHash<QString, AttributeType>> type_mapping = {
                {"2.5.5.8", {{"1", AttributeType_Boolean}}},
                {
                    "2.5.5.9",
                    {
                        {"10", AttributeType_Enumeration},
                        {"2", AttributeType_Integer}
                    }
                },
                {"2.5.5.16", {{"65", AttributeType_LargeInteger}}},
                {"2.5.5.3", {{"27", AttributeType_StringCase}}},
                {"2.5.5.5", {{"22", AttributeType_IA5}}},
                {"2.5.5.15", {{"66", AttributeType_NTSecDesc}}},
                {"2.5.5.6", {{"18", AttributeType_Numeric}}},
                {"2.5.5.2", {{"6", AttributeType_ObjectIdentifier}}},
                {"2.5.5.10", {{"4", AttributeType_Octet}}},
                {"2.5.5.5", {{"19", AttributeType_Printable}}},
                {"2.5.5.17", {{"4", AttributeType_Sid}}},
                {"2.5.5.4", {{"20", AttributeType_Teletex}}},
                {"2.5.5.12", {{"64", AttributeType_Unicode}}},
                {"2.5.5.17", {{"23", AttributeType_UTCTime}}},
                {"2.5.5.11", {{"24", AttributeType_GeneralizedTime}}},
            };

            // TODO: handle schema updates...?
            const AttributeType type =
            [=]() {
                const bool unknown_type = !type_mapping.contains(attribute_syntax) || !type_mapping[attribute_syntax].contains(om_syntax);
                if (unknown_type) {
                    return AttributeType_StringCase;
                }

                // NOTE: large integers can be both numbers and datetimes and there appears to be no way to distinguish them so have to manually remap here
                AttributeType out = type_mapping[attribute_syntax][om_syntax];
                const QList<QString> datetimes = {
                    ATTRIBUTE_ACCOUNT_EXPIRES,
                    ATTRIBUTE_LAST_LOGON,
                    ATTRIBUTE_LAST_LOGON_TIMESTAMP,
                    ATTRIBUTE_PWD_LAST_SET,
                    ATTRIBUTE_LOCKOUT_TIME,
                    ATTRIBUTE_BAD_PWD_TIME
                    // TODO:...
                };
                if (out == AttributeType_LargeInteger && datetimes.contains(attribute)) {
                    out = AttributeType_LargeIntegerDatetime; 
                }

                return out;
            }();

            attribute_type_map[attribute] = type;
        } else {
            attribute_type_map[attribute] = AttributeType_StringCase;
        }
    }

    const AttributeType type = attribute_type_map[attribute];

    return type;
}
