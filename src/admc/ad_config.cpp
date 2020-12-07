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
#include <QLineEdit>
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
#define ATTRIBUTE_SYSTEM_ONLY           "systemOnly"
#define ATTRIBUTE_RANGE_UPPER           "rangeUpper"
#define ATTRIBUTE_AUXILIARY_CLASS       "auxiliaryClass"
#define ATTRIBUTE_SYSTEM_AUXILIARY_CLASS "systemAuxiliaryClass"

#define CLASS_ATTRIBUTE_SCHEMA          "attributeSchema"
#define CLASS_CLASS_SCHEMA              "classSchema"

QString get_display_specifier_class(const QString &display_specifier);
QString get_locale_dir();

AdConfig::AdConfig(QObject *parent)
: QObject(parent)
{
    // Attribute schemas
    {
        const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_ATTRIBUTE_SCHEMA);

        const QList<QString> attributes = {
            ATTRIBUTE_LDAP_DISPLAY_NAME,
            ATTRIBUTE_ATTRIBUTE_SYNTAX,
            ATTRIBUTE_OM_SYNTAX,
            ATTRIBUTE_IS_SINGLE_VALUED,
            ATTRIBUTE_SYSTEM_ONLY,
            ATTRIBUTE_RANGE_UPPER,
        };

        const QString schema_dn = AD()->schema_dn();

        const QHash<QString, AdObject> search_results = AD()->search(filter, attributes, SearchScope_Children, schema_dn);

        for (const AdObject object : search_results.values()) {
            const QString attribute = object.get_string(ATTRIBUTE_LDAP_DISPLAY_NAME);
            attribute_schemas[attribute] = object;
        }
    }

    // Class schemas
    {
        const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_CLASS_SCHEMA);

        const QList<QString> attributes = {
            ATTRIBUTE_LDAP_DISPLAY_NAME,
            ATTRIBUTE_POSSIBLE_SUPERIORS,
            ATTRIBUTE_MAY_CONTAIN,
            ATTRIBUTE_SYSTEM_MAY_CONTAIN,
            ATTRIBUTE_AUXILIARY_CLASS,
            ATTRIBUTE_SYSTEM_AUXILIARY_CLASS,
        };

        const QString schema_dn = AD()->schema_dn();

        const QHash<QString, AdObject> search_results = AD()->search(filter, attributes, SearchScope_Children, schema_dn);

        for (const AdObject object : search_results.values()) {
            const QString object_class = object.get_string(ATTRIBUTE_LDAP_DISPLAY_NAME);
            class_schemas[object_class] = object;
        }
    }

    attribute_display_names =
    []() {
        QHash<QString, QHash<QString, QString>> out;

        const QString locale_dir = get_locale_dir();
        const QList<QString> search_attributes = {ATTRIBUTE_ATTRIBUTE_DISPLAY_NAMES};
        const QHash<QString, AdObject> search_results = AD()->search("", search_attributes, SearchScope_Children, locale_dir);

        for (const QString &dn : search_results.keys()) {
            const AdObject object = search_results[dn];

            const QList<QString> display_names = object.get_strings(ATTRIBUTE_ATTRIBUTE_DISPLAY_NAMES);

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

    // Columns
    {
        const QList<QString> columns_values =
        []() {
            const QString locale_dir = get_locale_dir();
            const QString dn = QString("CN=default-Display,%1").arg(locale_dir);
            const AdObject object = AD()->search_object(dn, {ATTRIBUTE_EXTRA_COLUMNS});

            // NOTE: order as stored in attribute is reversed. Order is not sorted alphabetically so can't just sort.
            QList<QString> extra_columns = object.get_strings(ATTRIBUTE_EXTRA_COLUMNS);
            std::reverse(extra_columns.begin(), extra_columns.end());

            return extra_columns;
        }();

        // ATTRIBUTE_EXTRA_COLUMNS value is
        // "$attribute,$display_name,..."
        // Get attributes out of that
        for (const QString &value : columns_values) {
            const QList<QString> column_split = value.split(',');

            if (column_split.size() < 2) {
                continue;
            }

            const QString attribute = column_split[0];
            const QString attribute_display_name = column_split[1];

            columns.append(attribute);
            column_display_names[attribute] = attribute_display_name;
        }

        // Insert some columns manually
        auto add_custom =
        [this](const Attribute &attribute, const QString &display_name) {
            columns.prepend(attribute);
            column_display_names[attribute] = display_name;
        };

        add_custom(ATTRIBUTE_DISTINGUISHED_NAME, tr("Distinguished name"));
        add_custom(ATTRIBUTE_DESCRIPTION, tr("Description"));
        add_custom(ATTRIBUTE_OBJECT_CLASS, tr("Class"));
        add_custom(ATTRIBUTE_NAME, tr("Name"));
    }

    filter_containers =
    [this]() {
        QList<QString> out;
        
        const QString locale_dir = get_locale_dir();
        const QString ui_settings_dn = QString("CN=DS-UI-Default-Settings,%1").arg(locale_dir);
        const AdObject object = AD()->search_object(ui_settings_dn, {ATTRIBUTE_FILTER_CONTAINERS});

        // TODO: dns-Zone category is mispelled in
        // ATTRIBUTE_FILTER_CONTAINERS, no idea why, might
        // just be on this domain version
        const QList<QString> categories =
        [object]() {
            QList<QString> categories_out = object.get_strings(ATTRIBUTE_FILTER_CONTAINERS);
            categories_out.replaceInStrings("dns-Zone", "Dns-Zone");

            return categories_out;
        }();

        // NOTE: ATTRIBUTE_FILTER_CONTAINERS contains object
        // *categories* not classes, so need to get object
        // class from category object
        for (const auto object_category : categories) {
            const QString category_dn = QString("CN=%1,%2").arg(object_category, AD()->schema_dn());
            const AdObject category_object = AD()->search_object(category_dn, {ATTRIBUTE_LDAP_DISPLAY_NAME});
            const QString object_class = category_object.get_string(ATTRIBUTE_LDAP_DISPLAY_NAME);

            out.append(object_class);
        }

        return out;
    }();

    find_attributes =
    [this]() {
        QHash<QString, QList<QString>> out;

        const QString locale_dir = get_locale_dir();
        const QList<QString> search_attributes = {ATTRIBUTE_ATTRIBUTE_DISPLAY_NAMES};
        const QHash<QString, AdObject> search_results = AD()->search("", search_attributes, SearchScope_Children, locale_dir);

        for (const QString &dn : search_results.keys()) {
            const AdObject object  = search_results[dn];

            const QList<QString> display_names = object.get_strings(ATTRIBUTE_ATTRIBUTE_DISPLAY_NAMES);

            const QString specifier_class = get_display_specifier_class(dn);

            QList<QString> attributes;
            for (const auto display_name_pair : display_names) {
                const QList<QString> split = display_name_pair.split(",");
                const QString attribute_name = split[0];

                attributes.append(attribute_name);
            }

            out[specifier_class] = attributes;
        }

        return out;
    }();
}

AdConfig *ADCONFIG() {
    return AdInterface::instance()->config();
}

QString AdConfig::get_attribute_display_name(const Attribute &attribute, const ObjectClass &objectClass) const {
    if (attribute_display_names.contains(objectClass) && attribute_display_names[objectClass].contains(attribute)) {
        const QString display_name = attribute_display_names[objectClass][attribute];

        return display_name;
    }

    // NOTE: display specifier doesn't cover all attributes for all classes, so need to hardcode some of them here
    static const QHash<Attribute, QString> fallback_display_names = {
        {ATTRIBUTE_NAME, QObject::tr("Name")},
        {ATTRIBUTE_DISTINGUISHED_NAME, QObject::tr("Distinguished name")},
        {ATTRIBUTE_OBJECT_CLASS, QObject::tr("Object class")},
        {ATTRIBUTE_WHEN_CREATED, QObject::tr("Created")},
        {ATTRIBUTE_WHEN_CHANGED, QObject::tr("Changed")},
        {ATTRIBUTE_USN_CREATED, QObject::tr("USN created")},
        {ATTRIBUTE_USN_CHANGED, QObject::tr("USN changed")},
        {ATTRIBUTE_ACCOUNT_EXPIRES, QObject::tr("Account expires")},
        {ATTRIBUTE_OBJECT_CATEGORY, QObject::tr("Type")},
        {ATTRIBUTE_PROFILE_PATH, QObject::tr("Profile path")},
        {ATTRIBUTE_SCRIPT_PATH, QObject::tr("Logon script")},
    };

    return fallback_display_names.value(attribute, attribute);
}

QString AdConfig::get_class_display_name(const QString &objectClass) const {
    return class_display_names.value(objectClass, objectClass);
}

QList<QString> AdConfig::get_columns() const {
    return columns;
}

QString AdConfig::get_column_display_name(const Attribute &attribute) const {
    return column_display_names.value(attribute, attribute);
}

QList<QString> AdConfig::get_filter_containers() const {
    return filter_containers;
}

QList<QString> AdConfig::get_possible_superiors(const QList<ObjectClass> &object_classes) const {
    QList<QString> out;

    for (const QString &object_class : object_classes) {
        const AdObject schema = class_schemas[object_class];
        out += schema.get_strings(ATTRIBUTE_POSSIBLE_SUPERIORS);
    }

    out.removeDuplicates();

    return out;
}

QList<QString> AdConfig::get_possible_attributes(const QList<QString> &object_classes) const {
    // Add auxiliary classes of given classes to list
    const QList<QString> all_classes =
    [=]() {
        QList<QString> out;

        out += object_classes;

        for (const auto object_class : object_classes) {
            const AdObject schema = class_schemas[object_class];
            out += schema.get_strings(ATTRIBUTE_AUXILIARY_CLASS);
            out += schema.get_strings(ATTRIBUTE_SYSTEM_AUXILIARY_CLASS);
        }

        out.removeDuplicates();

        return out;
    }();

    // Combine possible attributes of all classes of this object
    QList<QString> attributes;

    for (const auto object_class : all_classes) {
        const AdObject schema = class_schemas[object_class];
        attributes += schema.get_strings(ATTRIBUTE_MAY_CONTAIN);
        attributes += schema.get_strings(ATTRIBUTE_SYSTEM_MAY_CONTAIN);
    }

    attributes.removeDuplicates();

    return attributes;
}

QList<QString> AdConfig::get_find_attributes(const QString &object_class) const {
    return find_attributes.value(object_class, QList<QString>());
}

AttributeType AdConfig::get_attribute_type(const QString &attribute) const {
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
        {
            "2.5.5.10",
            {
                {"4", AttributeType_Octet},
                {"127", AttributeType_ReplicaLink}
            }
        },
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
        },
        {"2.5.5.14", {{"127", AttributeType_DNString}}},
        {"2.5.5.7", {{"127", AttributeType_DNBinary}}},
        {"2.5.5.1", {{"127", AttributeType_DSDN}}},
    };

    const AdObject schema = attribute_schemas[attribute];

    const QString attribute_syntax = schema.get_string(ATTRIBUTE_ATTRIBUTE_SYNTAX);
    const QString om_syntax = schema.get_string(ATTRIBUTE_OM_SYNTAX);

    if (type_map.contains(attribute_syntax) && type_map[attribute_syntax].contains(om_syntax)) {
        return type_map[attribute_syntax][om_syntax];
    } else {
        return AttributeType_StringCase;
    }
}

LargeIntegerSubtype AdConfig::get_large_integer_subtype(const QString &attribute) const {
    // Manually remap large integer types to subtypes
    // TODO: figure out where to get this data
    // externally. So far haven't found anything.
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
        return LargeIntegerSubtype_Datetime;
    } else if (timespans.contains(attribute)) {
        return LargeIntegerSubtype_Timespan;
    } else {
        return LargeIntegerSubtype_Integer;
    }
}

void AdConfig::limit_edit(QLineEdit *edit, const QString &attribute) {
    const int range_upper = get_attribute_range_upper(attribute);
    if (range_upper > 0) {
        edit->setMaxLength(range_upper);
    }
}

bool AdConfig::attribute_is_number(const QString &attribute) const {
    static const QList<AttributeType> number_types = {
        AttributeType_Integer,
        AttributeType_LargeInteger,
        AttributeType_Enumeration,
        AttributeType_Numeric,
    };
    const AttributeType type = get_attribute_type(attribute);

    return number_types.contains(type);
}

bool AdConfig::get_attribute_is_single_valued(const QString &attribute) const {
    return attribute_schemas[attribute].get_bool(ATTRIBUTE_IS_SINGLE_VALUED);
}

bool AdConfig::get_attribute_is_system_only(const QString &attribute) const {
    return attribute_schemas[attribute].get_bool(ATTRIBUTE_SYSTEM_ONLY);
}

int AdConfig::get_attribute_range_upper(const QString &attribute) const {
    return attribute_schemas[attribute].get_int(ATTRIBUTE_RANGE_UPPER);
}

// Display specifier DN is "CN=object-class-Display,CN=..."
// Get "object-class" from that
QString get_display_specifier_class(const QString &display_specifier) {
    const QString rdn = display_specifier.split(",")[0];
    const QString removed_cn = QString(rdn).remove("CN=", Qt::CaseInsensitive);
    QString out = removed_cn;
    out.remove("-Display");

    return out;
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
