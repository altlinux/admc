/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2026 BaseALT Ltd.
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

#ifndef COUNTRY_MANAGER_H
#define COUNTRY_MANAGER_H

#include <QString>
#include <QHash>

enum CountryColumn {
    CountryColumn_Country,
    CountryColumn_CountryRu,
    CountryColumn_Abbreviation,
    CountryColumn_Code,
    CountryColumn_COUNT,
};

class CountryManager {

public:
    /**
     * Get the global instance of the country manager.
     */
    static CountryManager& get_instance() {
        static CountryManager instance;
        return instance;
    }

    CountryManager(CountryManager const&)  = delete;
    void operator=(CountryManager const&)  = delete;

    bool load();

    const QHash<int, QString> &get_name_map() const;
    const QString get_country(const int &code) const;
    const QString get_abbreviation(const int &code) const;
    int get_code(const QString &abbreviation) const;

private:
    QHash<QString, int> string_to_code;
    QHash<QString, int> abbreviation_to_code;
    QHash<int, QString> country_strings;
    QHash<int, QString> country_strings_ru;
    QHash<int, QString> country_abbreviations;

    bool is_country_data_loaded = false;

    CountryManager();
};

QList<QString> country_manager_parse_line(QString line);

#endif  /* ifndef COUNTRY_MANAGER_H */
