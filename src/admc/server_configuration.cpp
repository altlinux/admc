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

#include <QHash>
#include <QLocale>
#include <QDebug>
#include <algorithm>

#define ATTRIBUTE_DISPLAY_NAMES         "attributeDisplayNames"
#define ATTRIBUTE_EXTRA_COLUMNS         "extraColumns"
#define ATTRIBUTE_FILTER_CONTAINERS     "msDS-FilterContainers"
#define ATTRIBUTE_LDAP_DISPLAY_NAME     "lDAPDisplayName"
#define ATTRIBUTE_ADMIN_DISPLAY_NAME    "adminDisplayName"
#define ATTRIBUTE_POSSIBLE_SUPERIORS    "systemPossSuperiors"
#define CLASS_DISPLAY_NAME              "classDisplayName"
#define TREAT_AS_LEAF                   "treatAsLeaf"

QString get_display_specifier_class(const QString &display_specifier);
QList<QString> get_all_display_specifiers();

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

        const QString search_base = AdInterface::instance()->get_search_base();

        return QString("CN=%1,CN=DisplaySpecifiers,CN=Configuration,%2").arg(locale_code, search_base);
    }();

    return locale_dir;
}

QString get_attribute_display_name(const QString &attribute, const QString &objectClass) {
    // { objectClass => { attribute => display_name } }
    static QHash<QString, QHash<QString, QString>> attribute_display_names =
    []() {
        QHash<QString, QHash<QString, QString>> attribute_display_names_out;

        const QList<QString> display_specifiers = get_all_display_specifiers();

        for (const auto display_specifier : display_specifiers) {
            const QList<QString> display_names =
            [display_specifier]() {
                QList<QString> out = AdInterface::instance()->attribute_get_multi(display_specifier, ATTRIBUTE_DISPLAY_NAMES);

                // NOTE: default display specifier contains some extra display names that are used for contents columns
                if (display_specifier.contains("default-Display")) {
                    const QList<QString> extra_display_names = AdInterface::instance()->attribute_get_multi(display_specifier, ATTRIBUTE_EXTRA_COLUMNS);

                    out.append(extra_display_names);
                }

                return out;
            }();

            const QString specifier_class = get_display_specifier_class(display_specifier);

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

        const QList<QString> display_specifiers = get_all_display_specifiers();

        for (const auto display_specifier : display_specifiers) {
            // TODO: duplicated code. Probably load this together with other display specifier info.
            const QString specifier_class = get_display_specifier_class(display_specifier);

            const QString class_display_name = AdInterface::instance()->attribute_get(display_specifier, CLASS_DISPLAY_NAME);

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
            QList<QString> columns_out = AdInterface::instance()->attribute_get_multi(default_display_specifier, ATTRIBUTE_EXTRA_COLUMNS);
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
        const QList<QString> ms_classes = AdInterface::instance()->attribute_get_multi(display_specifier, ATTRIBUTE_FILTER_CONTAINERS);

        // NOTE: ATTRIBUTE_FILTER_CONTAINERS contains classes in non-LDAP format ("Organizational-Unit" vs "organizationalUnit"). Convert to LDAP format by getting ATTRIBUTE_LDAP_DISPLAY_NAME from class' schema.
        const QString search_base = AdInterface::instance()->get_search_base();
        const QString schema_dn = QString("CN=Schema,CN=Configuration,%2").arg(search_base);

        QList<QString> out;
        for (const auto ms_class : ms_classes) {
            const QString class_schema = QString("CN=%1,%2").arg(ms_class, schema_dn);
            const QString ldap_class = AdInterface::instance()->attribute_get(class_schema, ATTRIBUTE_LDAP_DISPLAY_NAME);

            out.append(ldap_class);
        }

        // TODO: why is this not included???
        out.append(CLASS_DOMAIN);

        return out;
    }();

    return classes;
}

QList<QString> get_possible_superiors(const QString &dn) {
    const QString category = AdInterface::instance()->attribute_get(dn, ATTRIBUTE_OBJECT_CATEGORY);
    const QList<QString> possible_superiors = AdInterface::instance()->attribute_get_multi(category, ATTRIBUTE_POSSIBLE_SUPERIORS);

    return possible_superiors;
}

// Display specifier DN is "CN=class-Display,CN=..."
// Get "class" from that
QString get_display_specifier_class(const QString &display_specifier) {
    const QString rdn = display_specifier.split(",")[0];
    const QString removed_cn = QString(rdn).remove("CN=", Qt::CaseInsensitive);
    const QString specifier_class = removed_cn.split('-')[0];

    return specifier_class;
}

QList<QString> get_all_display_specifiers() {
    const QString locale_dir = get_locale_dir();
    const QList<QString> display_specifiers = AdInterface::instance()->list(locale_dir);

    return display_specifiers;
}

QString get_ad_class_name(const QString &ldap_class_name) {
    // NOTE: fill out this map incrementally, would take too long to load all of attributes on startup
    static QHash<QString, QString> ldap_to_ad;

    if (!ldap_to_ad.contains(ldap_class_name)) {
        const QString search_base = AdInterface::instance()->get_search_base();
        const QString schema = QString("CN=Schema,CN=Configuration,%1").arg(search_base);

        const QString filter = filter_EQUALS(ATTRIBUTE_LDAP_DISPLAY_NAME, ldap_class_name);
        const QList<QString> search_results = AdInterface::instance()->search(filter, schema);

        if (!search_results.isEmpty()) {
            const QString class_object = search_results[0];
            const QString ad_class_name = AdInterface::instance()->attribute_get(class_object, ATTRIBUTE_ADMIN_DISPLAY_NAME);

            ldap_to_ad[ldap_class_name] = ad_class_name;
        } else {
            ldap_to_ad[ldap_class_name] = "";
        }
    }

    return ldap_to_ad[ldap_class_name];
}
