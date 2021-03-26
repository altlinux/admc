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

#include "tabs/address_tab.h"
#include "adldap.h"
#include "status.h"
#include "globals.h"
#include "utils.h"
#include "edits/country_edit.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QHash>
#include <QFile>
#include <algorithm>

// TODO: translate country strings to Russian (qt doesn't have it)

#define COUNTRY_CODE_NONE 0

CountryEdit::CountryEdit(QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent)
{
    combo = new QComboBox();

    // Load all country names into combobox
    // NOTE: temp collections to sort items for combo box
    QList<QString> all_countries;
    QHash<QString, int> string_to_code;

    // TODO: cache this
    QFile file(":/admc/countries.csv");
    if (!file.open(QIODevice::ReadOnly)) {
        printf("ERROR: Failed to load countries file!\n");
    } else {
        // Load countries csv into maps
        // Map country code to country string and country abbreviation
        QHash<QString, int> country_string_to_code;
        
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

            all_countries.append(country_string);
            string_to_code[country_string] = code;
        }
    }

    // Put country strings/codes into combo box, sorted by strings
    std::sort(all_countries.begin(), all_countries.end());

    // Special case for "None" country
    // TODO: this seems really easy to break
    const QString none_string = tr("None");
    string_to_code[none_string] = COUNTRY_CODE_NONE;
    all_countries.insert(0, none_string);
    country_strings[COUNTRY_CODE_NONE] = "";
    country_abbreviations[COUNTRY_CODE_NONE] = "";

    for (auto country_string : all_countries) {
        const int code = string_to_code[country_string];

        combo->addItem(country_string, code);
    }

    QObject::connect(
    combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
    [this]() {
        emit edited();
    });
}

void CountryEdit::load_internal(AdInterface &ad, const AdObject &object) {
    const int country_code =
    [object]() {
        if (object.contains(ATTRIBUTE_COUNTRY)) {
            return object.get_int(ATTRIBUTE_COUNTRY);
        } else {
            return COUNTRY_CODE_NONE;
        }
    }();

    const int index = combo->findData(QVariant(country_code));
    if (index != -1) {
        combo->setCurrentIndex(index);
    }
}

void CountryEdit::set_read_only(const bool read_only) {
    combo->setDisabled(read_only);
}

void CountryEdit::add_to_layout(QFormLayout *layout) {
    const QString label_text = adconfig->get_attribute_display_name(ATTRIBUTE_COUNTRY, CLASS_USER) + ":";
    layout->addRow(label_text, combo);
}

bool CountryEdit::apply(AdInterface &ad, const QString &dn) const {
    const int code = combo->currentData().toInt();

    const QString code_string = QString::number(code);
    const QString country_string = country_strings[code];
    const QString abbreviation = country_abbreviations[code];

    bool success = true;
    success = success && ad.attribute_replace_string(dn, ATTRIBUTE_COUNTRY_CODE, code_string);
    success = success && ad.attribute_replace_string(dn, ATTRIBUTE_COUNTRY_ABBREVIATION, abbreviation);
    success = success && ad.attribute_replace_string(dn, ATTRIBUTE_COUNTRY, country_string);

    return success;
}
