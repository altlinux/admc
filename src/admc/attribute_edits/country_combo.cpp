/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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
QHash<int, QString> country_strings_ru;
QHash<int, QString> country_abbreviations;
QHash<QString, int> abbreviation_to_code;

enum CountryColumn {
    CountryColumn_Country,
    CountryColumn_CountryRu,
    CountryColumn_Abbreviation,
    CountryColumn_Code,
    CountryColumn_COUNT,
};

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
            const QString line = QString(line_array);

            // Split line by comma's, taking into
            // account that some comma's are inside
            // quoted parts and ignoring those.
            //
            // NOTE: there's definitely a better way to
            // do this
            const QList<QString> line_split = [&]() -> QList<QString> {
                if (line.contains('\"')) {
                    QList<QString> split_by_quotes = line.split('\"');
                    split_by_quotes.removeAll("");

                    if (split_by_quotes.size() == 2) {
                        QList<QString> split_rest = split_by_quotes[1].split(',');
                        split_rest.removeAll("");

                        QList<QString> out;
                        out.append(split_by_quotes[0]);
                        out.append(split_rest);

                        return out;
                    } else {
                        return QList<QString>();
                    }
                } else {
                    return line.split(',');
                }
            }();

            if (line_split.size() != CountryColumn_COUNT) {
                qDebug() << "country.csv contains malformed line: " << line;

                continue;
            }

            const QString country_string = line_split[CountryColumn_Country];
            const QString country_string_ru = line_split[CountryColumn_CountryRu];
            const QString abbreviation = line_split[CountryColumn_Abbreviation];
            const QString code_string = line_split[CountryColumn_Code];
            const int code = code_string.toInt();

            country_strings[code] = country_string;
            country_strings_ru[code] = country_string_ru;
            country_abbreviations[code] = abbreviation;
            abbreviation_to_code[abbreviation] = code;

            string_to_code[country_string] = code;
        }

        file.close();
    }

    loaded_country_data = true;
}

void country_combo_init(QComboBox *combo) {
    const QHash<int, QString> name_map = [&]() {
        const bool locale_is_ru = [&]() {
            const QLocale locale = settings_get_variant(SETTING_locale).toLocale();
            const bool out = (locale.language() == QLocale::Russian);

            return out;
        }();

        if (locale_is_ru) {
            return country_strings_ru;
        } else {
            return country_strings;
        }
    }();

    // Generate order of countries that will be used to
    // fill the combo.
    //
    // NOTE: modify order of countries in the combo to
    // put a particular country at the top of the list.
    // If this program ever happens to be used outside
    // of that particular country, there is a feature
    // flag "SETTING_feature_current_locale_first".
    const QList<QString> country_list = [&]() {
        const QString country_russia = [&]() {
            const QLocale top_locale = [&]() {
                const bool current_locale_first = settings_get_variant(SETTING_feature_current_locale_first).toBool();

                if (current_locale_first) {
                    const QLocale current_locale = settings_get_variant(SETTING_locale).toLocale();

                    return current_locale;
                } else {
                    const QLocale russia_locale = QLocale(QLocale::Russian, QLocale::Russia);

                    return russia_locale;
                }
            }();
            const QString locale_name = top_locale.name();
            const QList<QString> locale_name_split = locale_name.split("_");

            if (locale_name_split.size() == 2) {
                const QString abbreviation = locale_name_split[1];
                const int code = abbreviation_to_code[abbreviation];
                const QString country_name = name_map[code];

                return country_name;
            } else {
                return QString();
            }
        }();

        QList<QString> out = name_map.values();
        std::sort(out.begin(), out.end());
        out.removeAll(country_russia);
        out.prepend(country_russia);

        return out;
    }();

    // Add "None" at the start
    combo->addItem(QCoreApplication::translate("country_widget", "None"), COUNTRY_CODE_NONE);

    for (const QString &country : country_list) {
        const int code = name_map.key(country);

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
