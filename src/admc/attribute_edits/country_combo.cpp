/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
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

#include "attribute_edits/country_combo.h"

#include "core/managers/country_manager.h"

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

void country_combo_init(QComboBox *combo) {
    const QHash<int, QString> name_map =
        CountryManager::get_instance().get_name_map();

    // Generate order of countries that will be used to
    // fill the combo.
    //
    // NOTE: modify order of countries in the combo to
    // put a particular country at the top of the list.
    // If this program ever happens to be used outside
    // of that particular country, there is a feature
    // flag "SETTING_feature_current_locale_first".
    QString country_russia;
    const bool current_locale_first =
        settings_get_variant(SETTING_feature_current_locale_first).toBool();
    QLocale top_locale;
    if (current_locale_first) {
        top_locale = settings_get_variant(SETTING_locale).toLocale();
    } else {
        top_locale = QLocale(QLocale::Russian, QLocale::Russia);
    }

    const QString locale_name = top_locale.name();
    const QList<QString> locale_name_split = locale_name.split("_");

    if (locale_name_split.size() == 2) {
        const QString abbreviation = locale_name_split[1];
        const int code = CountryManager::get_instance().get_code(abbreviation);
        const QString country_name = name_map[code];

        country_russia = country_name;
    }

    QList<QString> country_list = name_map.values();
    std::sort(country_list.begin(), country_list.end());
    country_list.removeAll(country_russia);
    country_list.prepend(country_russia);

    // Add "None" at the start
    combo->addItem(QCoreApplication::translate("country_widget", "None"), COUNTRY_CODE_NONE);

    for (const QString &country : country_list) {
        const int code = name_map.key(country);

        combo->addItem(country, code);
    }
}

void country_combo_load(QComboBox *combo, const AdObject &object) {
    int country_code;
    if (object.contains(ATTRIBUTE_COUNTRY_CODE)) {
        country_code = object.get_int(ATTRIBUTE_COUNTRY_CODE);
    } else {
        country_code = COUNTRY_CODE_NONE;
    }

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
    const QString country_string =
        CountryManager::get_instance().get_country(code);
    const QString abbreviation =
        CountryManager::get_instance().get_abbreviation(code);

    bool success = true;
    success = success && ad.attribute_replace_string(dn, ATTRIBUTE_COUNTRY_CODE, code_string);
    success = success && ad.attribute_replace_string(dn, ATTRIBUTE_COUNTRY_ABBREVIATION, abbreviation);
    success = success && ad.attribute_replace_string(dn, ATTRIBUTE_COUNTRY, country_string);

    return success;
}
