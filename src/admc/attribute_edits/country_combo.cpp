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

#include "attribute_edits/country_combo.h"

#include "adldap.h"
#include "globals.h"
#include "settings.h"
#include "status.h"
#include "utils.h"

#include <QComboBox>
#include <QDebug>
#include <QFile>
#include <QHash>
#include <algorithm>

#define COUNTRY_CODE_NONE 0

bool loaded_country_data = false;
QHash<QString, int> string_to_code;
QHash<int, QString> country_strings;
QHash<int, QString> country_abbreviations;
QHash<QString, int> abbreviation_to_code;

void country_combo_load_data() {
    if (loaded_country_data) {
        qDebug() << "ERROR: Attempted to load country data more than once";

        return;
    }

    QFile file(":/admc/countries.csv");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "ERROR: Failed to load countries file!\n";

        return;
    } else {
        // Load countries csv into maps. Map country code to
        // country string and country abbreviation

        // Skip header
        file.readLine();

        while (!file.atEnd()) {
            const QByteArray line_array = file.readLine();
            const QString line(line_array);
            const QList<QString> line_split = line.split(',');

            if (line_split.size() != 3) {
                continue;
            }

            const QString country_string = line_split[0];
            const QString abbreviation = line_split[1];
            const QString code_string = line_split[2];
            const int code = code_string.toInt();

            country_strings[code] = country_string;
            country_abbreviations[code] = abbreviation;
            abbreviation_to_code[abbreviation] = code;

            string_to_code[country_string] = code;
        }

        file.close();
    }

    loaded_country_data = true;
}

void country_combo_init(QComboBox *combo) {
    // Generate order of countries that will be used to
    // fill the combo. Country of the current locale is
    // moved up to the start of the list.
    const QList<QString> country_list = []() {
        const QString current_country = [&]() {
            const QLocale current_locale = settings_get_variant(SETTING_locale).toLocale();
            const QString locale_name = current_locale.name();
            const QList<QString> locale_name_split = locale_name.split("_");

            if (locale_name_split.size() == 2) {
                const QString abbreviation = locale_name_split[1];
                const int code = abbreviation_to_code[abbreviation];
                const QString country_name = country_strings[code];

                return country_name;
            } else {
                return QString();
            }
        }();

        QList<QString> out = country_strings.values();
        std::sort(out.begin(), out.end());
        out.removeAll(current_country);
        out.prepend(current_country);

        return out;
    }();

    // Add "None" at the start
    combo->addItem(QCoreApplication::translate("country_widget", "None"), COUNTRY_CODE_NONE);

    for (const QString &country : country_list) {
        const int code = string_to_code[country];

        combo->addItem(country, code);
    }
}

void country_combo_load(QComboBox *combo, const AdObject &object) {
    const int country_code = [object]() {
        if (object.contains(ATTRIBUTE_COUNTRY_CODE)) {
            return object.get_int(ATTRIBUTE_COUNTRY_CODE);
        } else {
            return COUNTRY_CODE_NONE;
        }
    }();

    const int index = combo->findData(QVariant(country_code));
    if (index != -1) {
        combo->setCurrentIndex(index);
    }
}

bool country_combo_apply(const QComboBox *combo, AdInterface &ad, const QString &dn) {
    const int code = combo->currentData().toInt();

    // NOTE: this handles the COUNTRY_CODE_NONE case by
    // using empty strings for it's values
    const QString code_string = QString::number(code);
    const QString country_string = country_strings.value(code, QString());
    const QString abbreviation = country_abbreviations.value(code, QString());

    bool success = true;
    success = success && ad.attribute_replace_string(dn, ATTRIBUTE_COUNTRY_CODE, code_string);
    success = success && ad.attribute_replace_string(dn, ATTRIBUTE_COUNTRY_ABBREVIATION, abbreviation);
    success = success && ad.attribute_replace_string(dn, ATTRIBUTE_COUNTRY, country_string);

    return success;
}
