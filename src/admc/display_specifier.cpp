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

#include "display_specifier.h"
#include "ad_interface.h"
#include "settings.h"

#include <QHash>
#include <QLocale>
#include <QDebug>
#include <algorithm>

#define ATTRIBUTE_DISPLAY_NAMES     "attributeDisplayNames"
#define ATTRIBUTE_EXTRA_COLUMNS     "extraColumns"
#define ATTRIBUTE_FILTER_CONTAINERS "msDS-FilterContainers"
#define ATTRIBUTE_LDAP_DISPLAY_NAME "lDAPDisplayName"
#define CLASS_DISPLAY_NAME          "classDisplayName"
#define TREAT_AS_LEAF               "treatAsLeaf"

// NOTE: it is assumed that a language change requires a restart
// so display specifiers are loaded once and are permanent

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

QString get_attribute_display_string(const QString &attribute, const QString &objectClass) {
    // { objectClass => { attribute => display_name } }
    static QHash<QString, QHash<QString, QString>> attribute_display_names =
    []() {
        QHash<QString, QHash<QString, QString>> attribute_display_names_out;

        const QString locale_dir = get_locale_dir();

        const QList<QString> display_specifiers = AdInterface::instance()->list(locale_dir);

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

            // Display specifier DN is "CN=class-Display,CN=..."
            // Get "class" from that
            const QString specifier_class =
            [display_specifier]() {
                const QString rdn = display_specifier.split(",")[0];
                const QString removed_cn = QString(rdn).remove("CN=", Qt::CaseInsensitive);
                const QString class_out = removed_cn.split('-')[0];

                return class_out;
            }();

            for (const auto display_name : display_names) {
                const QList<QString> display_name_split = display_name.split(",");
                const QString attribute_name = display_name_split[0];
                const QString attribute_display_name = display_name_split[1];

                attribute_display_names_out[specifier_class][attribute_name] = attribute_display_name;
            }
        }

        return attribute_display_names_out;
    }();

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

QList<QString> get_extra_contents_columns() {
    const QList<QString> columns_values =
    []() {
        const QString locale_dir = get_locale_dir();
        const QString default_display_specifier = QString("CN=default-Display,%1").arg(locale_dir);
        QList<QString> columns_out = AdInterface::instance()->attribute_get_multi(default_display_specifier, ATTRIBUTE_EXTRA_COLUMNS);
        std::reverse(columns_out.begin(), columns_out.end());

        return columns_out;
    }();

    QList<QString> columns;

    for (const QString &value : columns_values) {
        const QList<QString> column_split = value.split(',');
        const QString attribute = column_split[0];

        columns.append(attribute);
    }

    return columns;
}

QList<QString> get_containers_filter_classes() {
    const QString locale_dir = get_locale_dir();
    const QString display_specifier = QString("CN=DS-UI-Default-Settings,%1").arg(locale_dir);
    const QList<QString> ms_classes = AdInterface::instance()->attribute_get_multi(display_specifier, ATTRIBUTE_FILTER_CONTAINERS);

    // NOTE: ATTRIBUTE_FILTER_CONTAINERS contains classes in non-LDAP format ("Organizational-Unit" vs "organizationalUnit"). Convert to LDAP format by getting ATTRIBUTE_LDAP_DISPLAY_NAME from class' schema.
    const QList<QString> classes =
    [ms_classes]() {
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
