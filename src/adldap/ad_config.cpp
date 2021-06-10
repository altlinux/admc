/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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
#include "ad_config_p.h"

#include "ad_interface.h"
#include "ad_object.h"
#include "ad_utils.h"
#include "ad_filter.h"

#include <QLocale>
#include <QDebug>
#include <QCoreApplication>
#include <algorithm>

#define ATTRIBUTE_ATTRIBUTE_DISPLAY_NAMES "attributeDisplayNames"
#define ATTRIBUTE_EXTRA_COLUMNS         "extraColumns"
#define ATTRIBUTE_FILTER_CONTAINERS     "msDS-FilterContainers"
#define ATTRIBUTE_LDAP_DISPLAY_NAME     "lDAPDisplayName"
#define ATTRIBUTE_POSSIBLE_SUPERIORS    "possSuperiors"
#define ATTRIBUTE_SYSTEM_POSSIBLE_SUPERIORS "systemPossSuperiors"
#define ATTRIBUTE_ATTRIBUTE_SYNTAX      "attributeSyntax"
#define ATTRIBUTE_OM_SYNTAX             "oMSyntax"
#define ATTRIBUTE_CLASS_DISPLAY_NAME    "classDisplayName"
#define ATTRIBUTE_MAY_CONTAIN           "mayContain"
#define ATTRIBUTE_SYSTEM_MAY_CONTAIN    "systemMayContain"
#define ATTRIBUTE_MUST_CONTAIN          "mustContain"
#define ATTRIBUTE_SYSTEM_MUST_CONTAIN   "systemMustContain"
#define ATTRIBUTE_IS_SINGLE_VALUED      "isSingleValued"
#define ATTRIBUTE_SYSTEM_ONLY           "systemOnly"
#define ATTRIBUTE_RANGE_UPPER           "rangeUpper"
#define ATTRIBUTE_AUXILIARY_CLASS       "auxiliaryClass"
#define ATTRIBUTE_SYSTEM_FLAGS          "systemFlags"
#define ATTRIBUTE_LINK_ID               "linkID"
#define ATTRIBUTE_SYSTEM_AUXILIARY_CLASS "systemAuxiliaryClass"

#define CLASS_ATTRIBUTE_SCHEMA          "attributeSchema"
#define CLASS_CLASS_SCHEMA              "classSchema"
#define CLASS_CONTROL_ACCESS_RIGHT      "controlAccessRight"

#define FLAG_ATTR_IS_CONSTRUCTED        0x00000004 

AdConfigPrivate::AdConfigPrivate() {

}

AdConfig::AdConfig() {
    d = new AdConfigPrivate();
}

AdConfig::~AdConfig() {
    delete d;
}

void AdConfig::load(AdInterface &ad, const QLocale &locale) {
    d->domain = get_default_domain_from_krb5();
    d->domain_head = domain_to_domain_dn(d->domain);

    d->filter_containers.clear();
    d->columns.clear();
    d->column_display_names.clear();
    d->class_display_names.clear();
    d->find_attributes.clear();
    d->attribute_display_names.clear();
    d->attribute_schemas.clear();
    d->class_schemas.clear();

    const QString locale_dir = [this, locale]() {
        const QString locale_code = [locale]() {
            if (locale.language() == QLocale::Russian) {
                return "419";
            } else {
                        // English
                return "409";
            }
        }();

        return QString("CN=%1,CN=DisplaySpecifiers,%2").arg(locale_code, configuration_dn());
    }();

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
            ATTRIBUTE_LINK_ID,
            ATTRIBUTE_SYSTEM_FLAGS,
        };

        const QHash<QString, AdObject> results = ad.search(schema_dn(), SearchScope_Children, filter, attributes );

        for (const AdObject &object : results.values()) {
            const QString attribute = object.get_string(ATTRIBUTE_LDAP_DISPLAY_NAME);
            d->attribute_schemas[attribute] = object;
        }
    }

    // Class schemas
    {
        const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_CLASS_SCHEMA);

        const QList<QString> attributes = {
            ATTRIBUTE_LDAP_DISPLAY_NAME,
            ATTRIBUTE_POSSIBLE_SUPERIORS,
            ATTRIBUTE_SYSTEM_POSSIBLE_SUPERIORS,
            ATTRIBUTE_MAY_CONTAIN,
            ATTRIBUTE_SYSTEM_MAY_CONTAIN,
            ATTRIBUTE_MUST_CONTAIN,
            ATTRIBUTE_SYSTEM_MUST_CONTAIN,
            ATTRIBUTE_AUXILIARY_CLASS,
            ATTRIBUTE_SYSTEM_AUXILIARY_CLASS,
        };

        const QHash<QString, AdObject> results = ad.search(schema_dn(), SearchScope_Children, filter, attributes);

        for (const AdObject &object : results.values()) {
            const QString object_class = object.get_string(ATTRIBUTE_LDAP_DISPLAY_NAME);
            d->class_schemas[object_class] = object;
        }
    }

    // Class display specifiers
    // NOTE: can't just store objects for these because the values require a decent amount of preprocessing which is best done once here, not everytime value is requested
    {
        const QString filter = QString();

        const QList<QString> search_attributes = {
            ATTRIBUTE_CLASS_DISPLAY_NAME,
            ATTRIBUTE_ATTRIBUTE_DISPLAY_NAMES,
        };

        const QHash<QString, AdObject> results = ad.search(locale_dir, SearchScope_Children, filter, search_attributes);

        for (const AdObject &object : results) {
            const QString dn = object.get_dn();

            // Display specifier DN is "CN=object-class-Display,CN=..."
            // Get "object-class" from that
            const QString object_class = [dn]() {
                const QString rdn = dn.split(",")[0];
                QString out = rdn;
                out.remove("CN=", Qt::CaseInsensitive);
                out.remove("-Display");

                return out;
            }();

            if (object.contains(ATTRIBUTE_CLASS_DISPLAY_NAME)) {
                d->class_display_names[object_class] = object.get_string(ATTRIBUTE_CLASS_DISPLAY_NAME);
            }

            if (object.contains(ATTRIBUTE_ATTRIBUTE_DISPLAY_NAMES)) {
                const QList<QString> display_names = object.get_strings(ATTRIBUTE_ATTRIBUTE_DISPLAY_NAMES);

                for (const auto &display_name_pair : display_names) {
                    const QList<QString> split = display_name_pair.split(",");
                    const QString attribute_name = split[0];
                    const QString display_name = split[1];

                    d->attribute_display_names[object_class][attribute_name] = display_name;
                }

                d->find_attributes[object_class] = [object_class, display_names]() {
                    QList<QString> out;

                    for (const auto &display_name_pair : display_names) {
                        const QList<QString> split = display_name_pair.split(",");
                        const QString attribute = split[0];

                        out.append(attribute);
                    }

                    return out;
                }();
            }
        }
    }

    // Columns
    {
        const QList<QString> columns_values = [&] {
            const QString dn = QString("CN=default-Display,%1").arg(locale_dir);
            const AdObject object = ad.search_object(dn, {ATTRIBUTE_EXTRA_COLUMNS});

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

            d->columns.append(attribute);
            d->column_display_names[attribute] = attribute_display_name;
        }

        // Insert some columns manually
        auto add_custom = [=](const Attribute &attribute, const QString &display_name) {
            d->columns.prepend(attribute);
            d->column_display_names[attribute] = display_name;
        };

        add_custom(ATTRIBUTE_DN, QCoreApplication::translate("AdConfig", "Distinguished name"));
        add_custom(ATTRIBUTE_DESCRIPTION, QCoreApplication::translate("AdConfig", "Description"));
        add_custom(ATTRIBUTE_OBJECT_CLASS, QCoreApplication::translate("AdConfig", "Class"));
        add_custom(ATTRIBUTE_NAME, QCoreApplication::translate("AdConfig", "Name"));
    }

    d->filter_containers = [&] {
        QList<QString> out;

        const QString ui_settings_dn = QString("CN=DS-UI-Default-Settings,%1").arg(locale_dir);
        const AdObject object = ad.search_object(ui_settings_dn, {ATTRIBUTE_FILTER_CONTAINERS});

        // TODO: dns-Zone category is mispelled in
        // ATTRIBUTE_FILTER_CONTAINERS, no idea why, might
        // just be on this domain version
        const QList<QString> categories = [object]() {
            QList<QString> categories_out = object.get_strings(ATTRIBUTE_FILTER_CONTAINERS);
            categories_out.replaceInStrings("dns-Zone", "Dns-Zone");

            return categories_out;
        }();

        // NOTE: ATTRIBUTE_FILTER_CONTAINERS contains object
        // *categories* not classes, so need to get object
        // class from category object
        for (const auto &object_category : categories) {
            const QString category_dn = QString("CN=%1,%2").arg(object_category, schema_dn());
            const AdObject category_object = ad.search_object(category_dn, {ATTRIBUTE_LDAP_DISPLAY_NAME});
            const QString object_class = category_object.get_string(ATTRIBUTE_LDAP_DISPLAY_NAME);

            out.append(object_class);
        }

        // NOTE: domain not included for some reason, so add it manually
        out.append(CLASS_DOMAIN);

        // Make configuration and schema pass filter in dev mode so they are visible and can be fetched
        out.append({CLASS_CONFIGURATION, CLASS_dMD});

        return out;
    }();

    d->right_to_guid_map = [&]() {
        QHash<QString, QString> out;

        const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_CONTROL_ACCESS_RIGHT);

        const QList<QString> attributes = {
            ATTRIBUTE_CN,
            ATTRIBUTE_RIGHTS_GUID,
        };

        const QString search_base = extended_rights_dn();

        const QHash<QString, AdObject> search_results = ad.search(search_base, SearchScope_Children, filter, attributes);

        for (const AdObject &object : search_results.values()) {
            const QString cn = object.get_string(ATTRIBUTE_CN);
            const QString guid = object.get_string(ATTRIBUTE_RIGHTS_GUID);

            out[cn] = guid;
        }

        return out;
    }();
}

QString AdConfig::domain() const {
    return d->domain;
}

QString AdConfig::domain_head() const {
    return d->domain_head;
}

QString AdConfig::configuration_dn() const {
    return QString("CN=Configuration,%1").arg(domain_head());
}

QString AdConfig::schema_dn() const {
    return QString("CN=Schema,%1").arg(configuration_dn());
}

QString AdConfig::partitions_dn() const {
    return QString("CN=Partitions,CN=Configuration,%1").arg(domain_head());
}

QString AdConfig::extended_rights_dn() const {
    return QString("CN=Extended-Rights,%1").arg(configuration_dn());
}

QString AdConfig::get_attribute_display_name(const Attribute &attribute, const ObjectClass &objectClass) const {
    if (d->attribute_display_names.contains(objectClass) && d->attribute_display_names[objectClass].contains(attribute)) {
        const QString display_name = d->attribute_display_names[objectClass][attribute];

        return display_name;
    }

    // NOTE: display specifier doesn't cover all attributes for all classes, so need to hardcode some of them here
    static const QHash<Attribute, QString> fallback_display_names = {
        {ATTRIBUTE_NAME, QCoreApplication::translate("AdConfig", "Name")},
        {ATTRIBUTE_DN, QCoreApplication::translate("AdConfig", "Distinguished name")},
        {ATTRIBUTE_OBJECT_CLASS, QCoreApplication::translate("AdConfig", "Object class")},
        {ATTRIBUTE_WHEN_CREATED, QCoreApplication::translate("AdConfig", "Created")},
        {ATTRIBUTE_WHEN_CHANGED, QCoreApplication::translate("AdConfig", "Changed")},
        {ATTRIBUTE_USN_CREATED, QCoreApplication::translate("AdConfig", "USN created")},
        {ATTRIBUTE_USN_CHANGED, QCoreApplication::translate("AdConfig", "USN changed")},
        {ATTRIBUTE_ACCOUNT_EXPIRES, QCoreApplication::translate("AdConfig", "Account expires")},
        {ATTRIBUTE_OBJECT_CATEGORY, QCoreApplication::translate("AdConfig", "Type")},
        {ATTRIBUTE_PROFILE_PATH, QCoreApplication::translate("AdConfig", "Profile path")},
        {ATTRIBUTE_SCRIPT_PATH, QCoreApplication::translate("AdConfig", "Logon script")},
        {ATTRIBUTE_SAMACCOUNT_NAME, QCoreApplication::translate("AdConfig", "Logon name (pre-Windows 2000)")},
        {ATTRIBUTE_MAIL, QCoreApplication::translate("AdConfig", "E-mail")},
    };

    return fallback_display_names.value(attribute, attribute);
}

QString AdConfig::get_class_display_name(const QString &objectClass) const {
    return d->class_display_names.value(objectClass, objectClass);
}

QList<QString> AdConfig::get_columns() const {
    return d->columns;
}

QString AdConfig::get_column_display_name(const Attribute &attribute) const {
    return d->column_display_names.value(attribute, attribute);
}

int AdConfig::get_column_index(const QString &attribute) const {
    if (!d->columns.contains(attribute)) {
        qWarning() << "ADCONFIG columns missing attribute:" << attribute;
    }

    return d->columns.indexOf(attribute);
}

QList<QString> AdConfig::get_filter_containers() const {
    return d->filter_containers;
}

QList<QString> AdConfig::get_possible_superiors(const QList<ObjectClass> &object_classes) const {
    QList<QString> out;

    for (const QString &object_class : object_classes) {
        const AdObject schema = d->class_schemas[object_class];
        out += schema.get_strings(ATTRIBUTE_POSSIBLE_SUPERIORS);
        out += schema.get_strings(ATTRIBUTE_SYSTEM_POSSIBLE_SUPERIORS);
    }

    out.removeDuplicates();

    return out;
}

QList<QString> AdConfig::get_optional_attributes(const QList<QString> &object_classes) const {
    const QList<QString> all_classes = d->add_auxiliary_classes(object_classes);

    QList<QString> attributes;

    for (const auto &object_class : all_classes) {
        const AdObject schema = d->class_schemas[object_class];
        attributes += schema.get_strings(ATTRIBUTE_MAY_CONTAIN);
        attributes += schema.get_strings(ATTRIBUTE_SYSTEM_MAY_CONTAIN);
    }

    attributes.removeDuplicates();

    return attributes;
}

QList<QString> AdConfig::get_mandatory_attributes(const QList<QString> &object_classes) const {
    const QList<QString> all_classes = d->add_auxiliary_classes(object_classes);

    QList<QString> attributes;

    for (const auto &object_class : all_classes) {
        const AdObject schema = d->class_schemas[object_class];
        attributes += schema.get_strings(ATTRIBUTE_MUST_CONTAIN);
        attributes += schema.get_strings(ATTRIBUTE_SYSTEM_MUST_CONTAIN);
    }

    attributes.removeDuplicates();

    return attributes;
}

QList<QString> AdConfig::get_find_attributes(const QString &object_class) const {
    return d->find_attributes.value(object_class, QList<QString>());
}

AttributeType AdConfig::get_attribute_type(const QString &attribute) const {
    // NOTE: replica of: https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-adts/7cda533e-d7a4-4aec-a517-91d02ff4a1aa
    // syntax -> om syntax list -> type
    static QHash<QString, QHash<QString, AttributeType>> type_map = {
        {"2.5.5.8", {{"1", AttributeType_Boolean}}},
        {
            "2.5.5.9",
            {
                {"10", AttributeType_Enumeration},
                {"2", AttributeType_Integer},
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
                {"127", AttributeType_ReplicaLink},
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
                {"24", AttributeType_GeneralizedTime},
            }
        },
        {"2.5.5.14", {{"127", AttributeType_DNString}}},
        {"2.5.5.7", {{"127", AttributeType_DNBinary}}},
        {"2.5.5.1", {{"127", AttributeType_DSDN}}},
    };

    const AdObject schema = d->attribute_schemas[attribute];

    const QString attribute_syntax = schema.get_string(ATTRIBUTE_ATTRIBUTE_SYNTAX);
    const QString om_syntax = schema.get_string(ATTRIBUTE_OM_SYNTAX);

    if (type_map.contains(attribute_syntax) && type_map[attribute_syntax].contains(om_syntax)) {
        return type_map[attribute_syntax][om_syntax];
    } else {
        return AttributeType_StringCase;
    }
}

LargeIntegerSubtype AdConfig::get_attribute_large_integer_subtype(const QString &attribute) const {
    // Manually remap large integer types to subtypes
    static const QList<QString> datetimes = {
        ATTRIBUTE_ACCOUNT_EXPIRES,
        ATTRIBUTE_LAST_LOGON,
        ATTRIBUTE_LAST_LOGON_TIMESTAMP,
        ATTRIBUTE_PWD_LAST_SET,
        ATTRIBUTE_LOCKOUT_TIME,
        ATTRIBUTE_BAD_PWD_TIME,
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

bool AdConfig::get_attribute_is_number(const QString &attribute) const {
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
    return d->attribute_schemas[attribute].get_bool(ATTRIBUTE_IS_SINGLE_VALUED);
}

bool AdConfig::get_attribute_is_system_only(const QString &attribute) const {
    return d->attribute_schemas[attribute].get_bool(ATTRIBUTE_SYSTEM_ONLY);
}

int AdConfig::get_attribute_range_upper(const QString &attribute) const {
    return d->attribute_schemas[attribute].get_int(ATTRIBUTE_RANGE_UPPER);
}

bool AdConfig::get_attribute_is_backlink(const QString &attribute) const {
    if (d->attribute_schemas[attribute].contains(ATTRIBUTE_LINK_ID)) {
        const int link_id = d->attribute_schemas[attribute].get_int(ATTRIBUTE_LINK_ID);
        const bool link_id_is_odd = (link_id % 2 != 0);

        return link_id_is_odd;
    } else {
        return false;
    }
}

bool AdConfig::get_attribute_is_constructed(const QString &attribute) const {
    const int system_flags = d->attribute_schemas[attribute].get_int(ATTRIBUTE_SYSTEM_FLAGS);
    return bit_is_set(system_flags, FLAG_ATTR_IS_CONSTRUCTED);   
}

QString AdConfig::get_right_guid(const QString &right_cn) const {
    const QString out = d->right_to_guid_map.value(right_cn, QString());
    return out;
}

// (noncontainer classes) = (all classes) - (container classes)
QList<QString> AdConfig::get_noncontainer_classes() {
    QList<QString> out = filter_classes;

    const QList<QString> container_classes = get_filter_containers();
    for (const QString &container_class : container_classes) {
        out.removeAll(container_class);
    }

    return out;
}

QList<QString> AdConfigPrivate::add_auxiliary_classes(const QList<QString> &object_classes) const {
    QList<QString> out;

    out += object_classes;

    for (const auto &object_class : object_classes) {
        const AdObject schema = class_schemas[object_class];
        out += schema.get_strings(ATTRIBUTE_AUXILIARY_CLASS);
        out += schema.get_strings(ATTRIBUTE_SYSTEM_AUXILIARY_CLASS);
    }

    out.removeDuplicates();

    return out;
}
