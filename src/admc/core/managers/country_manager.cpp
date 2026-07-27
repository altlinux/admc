/*
 * ADMC - AD Management Center
 *
 * Country manager is responsible for loading and handling the country data.
 *
 * Copyright (C) 2020-2026 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
 * Copyright (C) 2026 Artyom V. Poptsov
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

#include <QByteArray>
#include <QFile>
#include <QList>
#include <QLocale>
#include <QString>

#include "core/settings.h"
#include "country_manager.h"

static const QString COUNTRIES_FILE = "countries.csv";

CountryManager::CountryManager() {
    // Do nothing.
}

/**
 * This is an one-pass CSV line parser that reads a line from left to right and
 * parses it into separate fields, with proper quoted fields handling.
 *
 *       [*]          double-quote?
 *        |    ,-------------------------,
 *        |    |                         |
 *        V    V      double-quote?      |
 * [STATE_READ_FIELD]------------->[STATE_READ_QUOTED_FIELD]
 *  |          A   |                |                     A
 *  |          |   |                |                     |
 *  `----------'   |                `---------------------'
 *                 | end-of-line?
 *                 V
 *                [*]
 *
 * @param line A line to parse.
 * @return A list of parsed fields.
 */
QList<QString> country_manager_parse_line(QString line) {
    static const char SEPARATOR = ',';
    enum {
        STATE_READ_FIELD,
        STATE_READ_QUOTED_FIELD
    };
    QList<QString> result;
    QString field;
    int state = STATE_READ_FIELD;
    qsizetype end = line.length();
    for (qsizetype index = 0; index < end; index++) {
        QChar ch = line[index];
        switch (state) {
        case STATE_READ_FIELD:
            if (ch == '\"') {
                state = STATE_READ_QUOTED_FIELD;
            } else if (ch == SEPARATOR) {
                result.append(field);
                field = "";
            } else if ((ch == '\r') || (ch == '\n')) {
                if (! field.isEmpty()) {
                    result.append(field);
                    field = "";
                }
                index++;
            } else {
                field += ch;
            }
            break;
        case STATE_READ_QUOTED_FIELD:
            if (ch == '\"') {
                state = STATE_READ_FIELD;
            } else {
                field += ch;
            }
            break;
        }
    }
    if (! field.isEmpty()) {
        result.append(field);
    }
    return result;
}

/**
 * Load countries from a CSV file into hash maps.  Map country code to country
 * string and country abbreviation.
 */
bool CountryManager::load() {
    if (is_country_data_loaded) {
        qDebug() << "ERROR: Attempted to load country data more than once";
        return false;
    }

    QFile file(":/admc/" + COUNTRIES_FILE);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "ERROR: Failed to load countries file!\n";
        return false;
    }

    // Skip header
    file.readLine();

    while (!file.atEnd()) {
        const QByteArray line_array = file.readLine();
        const QString line = QString(line_array);
        QList<QString> line_split = country_manager_parse_line(line);

        if (line_split.size() != CountryColumn_COUNT) {
            qDebug() << COUNTRIES_FILE << "contains malformed line: " << line;

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

    is_country_data_loaded = true;

    return true;
}

const QHash<int, QString> &CountryManager::get_name_map() const {
    const QLocale locale = settings_get_variant(SETTING_locale).toLocale();
    const bool locale_is_ru = (locale.language() == QLocale::Russian);
    if (locale_is_ru) {
        return country_strings_ru;
    } else {
        return country_strings;
    }
}

/**
 * Get country string by its code.
 *
 * @param code Country code.
 * @return A country string.
 */
const QString CountryManager::get_country(const int &code) const {
    return country_strings.value(code, QString());
}

/**
 * Get country abbreviation string by its code.
 *
 * @param code Country code.
 * @return A country abbreviation string.
 */
const QString CountryManager::get_abbreviation(const int &code) const {
    return country_abbreviations.value(code, QString());
}

/**
 * Get country code by its abbreviation.
 *
 * @param abbreviation The country abbreviation.
 * @return A country code.
 */
int CountryManager::get_code(const QString &abbreviation) const {
    return abbreviation_to_code[abbreviation];
}
