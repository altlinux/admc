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

#define ATTRIBUTE_DISPLAY_NAMES "attributeDisplayNames"
#define ATTRIBUTE_EXTRA_COLUMNS "extraColumns"
#define CLASS_DISPLAY_NAME      "classDisplayName"
#define TREAT_AS_LEAF           "treatAsLeaf"

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

    return attribute;
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
