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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QHash>
#include <QFile>
#include <algorithm>

// TODO: translate country strings to Russian (qt doesn't have it)

// NOTE: country codes are 3 digits only, so 0-999 = 1000
QString country_strings[1000];
QString country_abbreviations[1000];

AddressTab::AddressTab(DetailsWidget *details_arg)
: DetailsTab(details_arg)
{   
    title = tr("Address");

    // TODO: don't know why, but if i just have hbox as top layout, the widgets are misaligned
    const auto top_layout = new QVBoxLayout(this);

    const auto attributes_layout = new QHBoxLayout();
    top_layout->insertLayout(-1, attributes_layout);

    const auto label_layout = new QVBoxLayout();
    const auto edit_layout = new QVBoxLayout();
    attributes_layout->insertLayout(-1, label_layout);
    attributes_layout->insertLayout(-1, edit_layout);

    auto make_line_edit =
    [this, label_layout, edit_layout](const QString &attribute, const QString &label_text) {
        add_attribute_edit(attribute, label_text, label_layout, edit_layout, OldAttributeEditType_Editable);
    };

    make_line_edit(ATTRIBUTE_STREET, tr("Street:"));
    make_line_edit(ATTRIBUTE_PO_BOX, tr("P.O. Box:"));
    make_line_edit(ATTRIBUTE_CITY, tr("City"));
    make_line_edit(ATTRIBUTE_STATE, tr("State/province:"));
    make_line_edit(ATTRIBUTE_POSTAL_CODE, tr("Postal code:"));

    auto country_label = new QLabel("Country:");
    label_layout->addWidget(country_label);

    country_combo = new QComboBox(this);
    edit_layout->addWidget(country_combo);

    connect(country_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &AddressTab::on_country_combo);

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

    country_combo->blockSignals(true);
    for (auto country_string : all_countries) {
        const int code = string_to_code[country_string];

        country_combo->addItem(country_string, code);
    }
    country_combo->blockSignals(false);
}

void AddressTab::apply() {

}

void AddressTab::reload_internal() {
    // Load country
    const QString current_code_string = AdInterface::instance()->attribute_get(target(), ATTRIBUTE_COUNTRY_CODE);
    const int current_code = current_code_string.toInt();
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

void AddressTab::on_country_combo(int index) {
    const QVariant item_data = country_combo->itemData(index);
    const int code = item_data.value<int>();

    const QString code_string = QString::number(code);
    const QString country_string = country_strings[code];
    const QString abbreviation = country_abbreviations[code];

    AdInterface::instance()->attribute_replace(target(), ATTRIBUTE_COUNTRY_CODE, code_string);
    AdInterface::instance()->attribute_replace(target(), ATTRIBUTE_COUNTRY_ABBREVIATION, abbreviation);
    AdInterface::instance()->attribute_replace(target(), ATTRIBUTE_COUNTRY, country_string);

    const QString name = AdInterface::instance()->attribute_get(target(), ATTRIBUTE_NAME);
    Status::instance()->message(QString(tr("Changed country of object - %1")).arg(name), StatusType_Success);
}
