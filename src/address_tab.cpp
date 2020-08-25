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

#include "address_tab.h"
#include "ad_interface.h"
#include "status.h"
#include "attribute_edit.h"
#include "attribute_display_strings.h"
#include "utils.h"
#include "details_widget.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QComboBox>
#include <QHash>
#include <QFile>
#include <QLabel>
#include <algorithm>

// TODO: translate country strings to Russian (qt doesn't have it)

#define COUNTRY_CODE_NONE 0

// NOTE: country codes are 3 digits only, so 0-999 = 1000
QString country_strings[1000];
QString country_abbreviations[1000];

AddressTab::AddressTab(DetailsWidget *details_arg)
: DetailsTab(details_arg)
{   
    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);

    const auto edits_layout = new QGridLayout();
    top_layout->addLayout(edits_layout);
    
    const QList<QString> attributes = {
        ATTRIBUTE_STREET,
        ATTRIBUTE_PO_BOX,
        ATTRIBUTE_CITY,
        ATTRIBUTE_STATE,
        ATTRIBUTE_POSTAL_CODE
    };

    QMap<QString, StringEdit *> string_edits;
    make_string_edits(attributes, &string_edits);

    for (auto attribute : attributes) {
        auto edit = string_edits[attribute];
        edits.append(edit);
        edit->add_to_layout(edits_layout);
    }

    country_combo = new QComboBox(this);

    const QString country_display_string = get_attribute_display_string(ATTRIBUTE_COUNTRY);
    append_to_grid_layout_with_label(edits_layout, new QLabel(country_display_string), country_combo);

    // Load all country names into combobox
    // NOTE: temp collections to sort items for combo box
    QList<QString> all_countries;
    QHash<QString, int> string_to_code;

    QFile file(":/countries.csv");
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

    country_combo->blockSignals(true);
    for (auto country_string : all_countries) {
        const int code = string_to_code[country_string];

        country_combo->addItem(country_string, code);
    }
    country_combo->blockSignals(false);

    connect_edits_to_tab(edits, this);
    connect(
        country_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &DetailsTab::on_edit_changed);
}

bool AddressTab::verify() {
    return verify_attribute_edits(edits, this);
}

void AddressTab::apply() {
    apply_attribute_edits(edits, target(), this);

    // Apply country
    const QVariant item_data = country_combo->currentData();
    const int code = item_data.value<int>();

    const bool country_code_changed = (code != original_country_code);
    if (country_code_changed) {
        const QString code_string = QString::number(code);
        const QString country_string = country_strings[code];
        const QString abbreviation = country_abbreviations[code];

        AdInterface::instance()->attribute_replace(target(), ATTRIBUTE_COUNTRY_CODE, code_string);
        AdInterface::instance()->attribute_replace(target(), ATTRIBUTE_COUNTRY_ABBREVIATION, abbreviation);
        AdInterface::instance()->attribute_replace(target(), ATTRIBUTE_COUNTRY, country_string);

        const QString name = AdInterface::instance()->attribute_get(target(), ATTRIBUTE_NAME);
        Status::instance()->message(QString(tr("Changed country of object - %1")).arg(name), StatusType_Success);
    }
}

void AddressTab::reload() {
    load_attribute_edits(edits, target());

    // Load country
    QString current_code_string = AdInterface::instance()->attribute_get(target(), ATTRIBUTE_COUNTRY_CODE);
    int current_code = current_code_string.toInt();
    if (current_code_string == "") {
        current_code = COUNTRY_CODE_NONE;
    }
    original_country_code = current_code;

    const QVariant code_variant(current_code);
    const int index = country_combo->findData(code_variant);
    if (index != -1) {
        country_combo->blockSignals(true);
        country_combo->setCurrentIndex(index);
        country_combo->blockSignals(false);
    }
}

bool AddressTab::accepts_target() const {
    return AdInterface::instance()->is_user(target());
}
