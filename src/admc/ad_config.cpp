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

#include "ad_config.h"
#include "ad_interface.h"
#include "ad_object.h"
#include "settings.h"
#include "utils.h"
#include "filter.h"

#include <QLocale>
#include <QDebug>
#include <algorithm>

#define ATTRIBUTE_ATTRIBUTE_DISPLAY_NAMES "attributeDisplayNames"
#define ATTRIBUTE_EXTRA_COLUMNS         "extraColumns"
#define ATTRIBUTE_FILTER_CONTAINERS     "msDS-FilterContainers"
#define ATTRIBUTE_LDAP_DISPLAY_NAME     "lDAPDisplayName"
#define ATTRIBUTE_ADMIN_DISPLAY_NAME    "adminDisplayName"
#define ATTRIBUTE_POSSIBLE_SUPERIORS    "systemPossSuperiors"
#define ATTRIBUTE_ATTRIBUTE_SYNTAX      "attributeSyntax"
#define ATTRIBUTE_OM_SYNTAX             "oMSyntax"
#define ATTRIBUTE_CLASS_DISPLAY_NAME    "classDisplayName"
#define TREAT_AS_LEAF                   "treatAsLeaf"
#define ATTRIBUTE_MAY_CONTAIN           "mayContain"
#define ATTRIBUTE_SYSTEM_MAY_CONTAIN    "systemMayContain"
#define ATTRIBUTE_IS_SINGLE_VALUED      "isSingleValued"

QString get_display_specifier_class(const QString &display_specifier);
QString get_locale_dir();

AdConfig::AdConfig(QObject *parent)
: QObject(parent)
{
    {
        const QString schema_dn = AD()->schema_dn();

        const QList<QString> attributes = {ATTRIBUTE_LDAP_DISPLAY_NAME, ATTRIBUTE_ADMIN_DISPLAY_NAME,
            ATTRIBUTE_IS_SINGLE_VALUED
        };
        const QHash<QString, AdObject> search_results = AD()->search("", attributes, SearchScope_Children, schema_dn);

        for (const AdObject object : search_results.values()) {
            const QString ad_name = object.get_string(ATTRIBUTE_ADMIN_DISPLAY_NAME);
            const QString ldap_name = object.get_string(ATTRIBUTE_LDAP_DISPLAY_NAME);
            const bool is_single_valued = object.get_bool(ATTRIBUTE_IS_SINGLE_VALUED);

            if (!ad_name.isEmpty() && !ldap_name.isEmpty()) {
                ldap_to_ad_names[ldap_name] = ad_name;
                ad_to_ldap_names[ad_name] = ldap_name;
                attribute_is_single_valued[ldap_name] = is_single_valued;
            }
        }
    }

    attribute_display_names =
    []() {
        QHash<QString, QHash<QString, QString>> out;

        const QString locale_dir = get_locale_dir();
        const QList<QString> search_attributes = {ATTRIBUTE_ATTRIBUTE_DISPLAY_NAMES, ATTRIBUTE_EXTRA_COLUMNS};
        const QHash<QString, AdObject> search_results = AD()->search("", search_attributes, SearchScope_Children, locale_dir);

        for (const QString &dn : search_results.keys()) {
            const AdObject object  = search_results[dn];

            const QList<QString> display_names =
            [dn, object]() {
                QList<QString> display_names_out = object.get_strings(ATTRIBUTE_ATTRIBUTE_DISPLAY_NAMES);

                // NOTE: default display specifier contains some extra display names that are used for contents columns
                if (dn.contains("default-Display")) {
                    const QList<QString> extra_display_names = object.get_strings(ATTRIBUTE_EXTRA_COLUMNS);

                    display_names_out.append(extra_display_names);
                }

                return display_names_out;
            }();

            const QString specifier_class = get_display_specifier_class(dn);

            for (const auto display_name_pair : display_names) {
                const QList<QString> split = display_name_pair.split(",");
                const QString attribute_name = split[0];
                const QString display_name = split[1];

                out[specifier_class][attribute_name] = display_name;
            }
        }

        return out;
    }();

    class_display_names =
    []() {
        QHash<QString, QString> out;

        const QString locale_dir = get_locale_dir();
        const QList<QString> search_attributes = {ATTRIBUTE_CLASS_DISPLAY_NAME};
        const QHash<QString, AdObject> search_results = AD()->search("", search_attributes, SearchScope_Children, locale_dir);

        for (const QString &dn : search_results.keys()) {
            // TODO: duplicated code. Probably load this together with other display specifier info.
            const QString specifier_class = get_display_specifier_class(dn);

            const AdObject object  = search_results[dn];
            const QString class_display_name = object.get_string(ATTRIBUTE_CLASS_DISPLAY_NAME);

            out[specifier_class] = class_display_name;
        }

        return out;
    }();

    extra_columns =
    []() {
        const QList<QString> columns_values =
        []() {
            const QString locale_dir = get_locale_dir();
            const QString default_display_specifier = QString("CN=default-Display,%1").arg(locale_dir);
            QList<QString> columns_out = AD()->request_strings(default_display_specifier, ATTRIBUTE_EXTRA_COLUMNS);
            std::reverse(columns_out.begin(), columns_out.end());

            return columns_out;
        }();

        // ATTRIBUTE_EXTRA_COLUMNS value is
        // "$attribute,$display_name"
        // Get attributes out of that
        QList<QString> out;
        for (const QString &value : columns_values) {
            const QList<QString> column_split = value.split(',');
            const QString attribute = column_split[0];

            out.append(attribute);
        }

        return out;
    }();

    filter_containers =
    [this]() {
        QList<QString> out;
        
        const QString locale_dir = get_locale_dir();
        const QString display_specifier = QString("CN=DS-UI-Default-Settings,%1").arg(locale_dir);
        QList<QString> filter_containers_ad = AD()->request_strings(display_specifier, ATTRIBUTE_FILTER_CONTAINERS);

        // ATTRIBUTE_FILTER_CONTAINERS contains ad class names
        // so convert to ldap class names
        for (const auto class_ad : filter_containers_ad) {
            const QString class_ldap = get_ad_to_ldap_name(class_ad);
            out.append(class_ldap);
        }

        // TODO: why is this not included???
        out.append(CLASS_DOMAIN);

        return out;
    }();

    possible_superiors =
    []() {
        QHash<QString, QList<QString>> out;

        const QString schema_dn = AD()->schema_dn();
        const QString filter = filter_EQUALS(ATTRIBUTE_POSSIBLE_SUPERIORS, "*");
        const QList<QString> attributes = {ATTRIBUTE_POSSIBLE_SUPERIORS};
        const QHash<QString, AdObject> search_results = AD()->search(filter, attributes, SearchScope_Children, schema_dn);

        for (const AdObject object : search_results.values()) {
            const QString category = object.get_dn();
            const QList<QString> category_superiors = object.get_strings(ATTRIBUTE_POSSIBLE_SUPERIORS);

            out[category] = category_superiors;
        }

        return out;
    }();

    possible_attributes =
    []() {
        QHash<QString, QList<QString>> out;

        const QString schema_dn = AD()->schema_dn();
        const QList<QString> attributes = {
            ATTRIBUTE_MAY_CONTAIN,
            ATTRIBUTE_SYSTEM_MAY_CONTAIN,
            ATTRIBUTE_LDAP_DISPLAY_NAME
        };
        const QHash<QString, AdObject> search_results = AD()->search("", attributes, SearchScope_Children, schema_dn);

        for (const AdObject &schema : search_results.values()) {
            const QString object_class = schema.get_string(ATTRIBUTE_LDAP_DISPLAY_NAME);
            const QList<QString> may_contain = schema.get_strings(ATTRIBUTE_MAY_CONTAIN);
            const QList<QString> system_may_contain = schema.get_strings(ATTRIBUTE_SYSTEM_MAY_CONTAIN);

            QList<QString> total_contain;
            total_contain.append(may_contain);
            total_contain.append(system_may_contain);

            out[object_class] = total_contain;
        }

        return out;
    }();

    attribute_types =
    [this]() {
        QHash<QString, AttributeType> out;

        const QString schema_dn = AD()->schema_dn();
        const QString filter = filter_EQUALS(ATTRIBUTE_POSSIBLE_SUPERIORS, "*");
        const QList<QString> attributes = {ATTRIBUTE_ATTRIBUTE_SYNTAX,
            ATTRIBUTE_OM_SYNTAX,
            ATTRIBUTE_LDAP_DISPLAY_NAME
        };
        const QHash<QString, AdObject> search_results = AD()->search("", attributes, SearchScope_Children, schema_dn);

        for (const AdObject &schema : search_results) {
            const QString attribute = schema.get_string(ATTRIBUTE_LDAP_DISPLAY_NAME);
            const QString syntax = schema.get_string(ATTRIBUTE_ATTRIBUTE_SYNTAX);
            const QString om_syntax = schema.get_string(ATTRIBUTE_OM_SYNTAX);

            if (attribute.isEmpty() || syntax.isEmpty() || om_syntax.isEmpty()) {
                continue;
            }

            // NOTE: replica of: https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-adts/7cda533e-d7a4-4aec-a517-91d02ff4a1aa
            // syntax -> om syntax list -> type
            // TODO: is there a lib for this?
            // TODO: there are Object(x) types, not sure if need those
            static QHash<QString, QHash<QString, AttributeType>> type_map = {
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
                {
                    "2.5.5.11",
                    {
                        {"23", AttributeType_UTCTime},
                        {"24", AttributeType_GeneralizedTime}
                    }
                }
            };

            const AttributeType type =
            [=]() {
                const bool unknown_type = (!type_map.contains(syntax) || !type_map[syntax].contains(om_syntax));
                if (unknown_type) {
                    return AttributeType_StringCase;
                }

                AttributeType type_out = type_map[syntax][om_syntax];

                // Manually remap large integer types to subtypes
                // TODO: figure out where to get this data
                // externally. So far haven't found anything.
                if (type_out == AttributeType_LargeInteger) {
                    static const QList<QString> datetimes = {
                        ATTRIBUTE_ACCOUNT_EXPIRES,
                        ATTRIBUTE_LAST_LOGON,
                        ATTRIBUTE_LAST_LOGON_TIMESTAMP,
                        ATTRIBUTE_PWD_LAST_SET,
                        ATTRIBUTE_LOCKOUT_TIME,
                        ATTRIBUTE_BAD_PWD_TIME
                    };
                    static const QList<QString> timespans = {
                        ATTRIBUTE_MAX_PWD_AGE,
                        ATTRIBUTE_MIN_PWD_AGE,
                        ATTRIBUTE_LOCKOUT_DURATION,
                    };

                    if (datetimes.contains(attribute)) {
                        return AttributeType_LargeIntegerDatetime;
                    } else if (timespans.contains(attribute)) {
                        return AttributeType_LargeIntegerTimespan;
                    } 
                }

                return type_out;
            }();

            out[attribute] = type;
        }

        return out;
    }();
}

AdConfig *ADCONFIG() {
    return AdInterface::instance()->config();
}

QString AdConfig::get_attribute_display_name(const QString &attribute, const QString &objectClass) const {
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

QString AdConfig::get_class_display_name(const QString &objectClass) const {
    if (class_display_names.contains(objectClass)) {
        return class_display_names[objectClass];
    } else {
        return objectClass;
    }
}

QList<QString> AdConfig::get_extra_columns() const {
    return extra_columns;
}

QList<QString> AdConfig::get_filter_containers() const {
    return filter_containers;
}

QList<QString> AdConfig::get_possible_superiors(const QString &object_category) const {
    if (possible_superiors.contains(object_category)) {
        return possible_superiors[object_category];
    } else {
        return QList<QString>();
    }
}

QString AdConfig::get_ldap_to_ad_name(const QString &ldap_name) const {
    if (ldap_to_ad_names.contains(ldap_name)) {
        return ldap_to_ad_names[ldap_name];
    } else {
        return ldap_name;
    }
}

QString AdConfig::get_ad_to_ldap_name(const QString &ad_name) const {
    if (ad_to_ldap_names.contains(ad_name)) {
        return ad_to_ldap_names[ad_name];
    } else {
        return ad_name;
    }
}

// TODO: Object's objectClass list appears to already contain the full inheritance chain. Confirm that this applies to all objects, because otherwise would need to manually go down the inheritance chain to get all possible attributes.
QList<QString> AdConfig::get_possible_attributes(const QList<QString> &object_classes) const {
    QList<QString> attributes;

    for (const auto object_class : object_classes) {
        if (possible_attributes.contains(object_class)) {
            const QList<QString> class_attributes = possible_attributes[object_class];

            attributes.append(class_attributes);
        }
    }

    return attributes;
}

AttributeType AdConfig::get_attribute_type(const QString &attribute) const {
    if (attribute_types.contains(attribute)) {
        return attribute_types[attribute];
    } else {
        return AttributeType_StringCase;
    }
}

bool AdConfig::get_attribute_is_single_valued(const QString &attribute) const {
    if (attribute_is_single_valued.contains(attribute)) {
        return attribute_is_single_valued[attribute];
    } else {
        return true;
    }
}

// Display specifier DN is "CN=class-Display,CN=..."
// Get "class" from that
QString get_display_specifier_class(const QString &display_specifier) {
    const QString rdn = display_specifier.split(",")[0];
    const QString removed_cn = QString(rdn).remove("CN=", Qt::CaseInsensitive);
    const QString specifier_class = removed_cn.split('-')[0];

    return specifier_class;
}

QString get_locale_dir() {
    static QString locale_dir =
    []() {
        const QString locale_code =
        []() {
            const QLocale saved_locale = SETTINGS()->get_variant(VariantSetting_Locale).toLocale();

            if (saved_locale.language() == QLocale::Russian) {
                return "419";
            } else {
                    // English
                return "409";
            }
        }();

        const QString configuration_dn = AD()->configuration_dn();

        return QString("CN=%1,CN=DisplaySpecifiers,%2").arg(locale_code, configuration_dn);
    }();

    return locale_dir;
}
