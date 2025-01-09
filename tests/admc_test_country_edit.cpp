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

#include "admc_test_country_edit.h"

#include "attribute_edits/country_combo.h"
#include "attribute_edits/country_edit.h"

#include <QComboBox>
#include <QFormLayout>

#define TEST_ATTRIBUTE ATTRIBUTE_FIRST_NAME

void ADMCTestCountryEdit::init() {
    ADMCTest::init();

    country_combo_load_data();

    combo = new QComboBox(parent_widget);

    edit = new CountryEdit(combo, parent_widget);

    // Create test user
    const QString name = TEST_USER;
    dn = test_object_dn(name, CLASS_USER);
    const bool create_success = ad.object_add(dn, CLASS_USER);
    QVERIFY(create_success);
}

// Combo count should be some decently large number. If csv
// failed to load, combo would have just the "None" item
void ADMCTestCountryEdit::loaded_csv_file() {
    QVERIFY(combo->count() > 200);
}

// edited() signal should be emitted when lineedit is edited
void ADMCTestCountryEdit::emit_edited_signal() {
    bool edited_signal_emitted = false;
    connect(
        edit, &AttributeEdit::edited,
        this,
        [&edited_signal_emitted]() {
            edited_signal_emitted = true;
        });

    combo->setCurrentIndex(combo->currentIndex() + 1);
    QVERIFY(edited_signal_emitted);
}

// Edit should contain current attribute value after load()
// call
void ADMCTestCountryEdit::load() {
    // Set attribute value
    const int test_value = 4;
    ad.attribute_replace_int(dn, ATTRIBUTE_COUNTRY_CODE, test_value);

    // Load user into edit
    const AdObject object = ad.search_object(dn);
    edit->load(ad, object);

    const int edit_value = combo->currentData().toInt();
    QCOMPARE(edit_value, test_value);
}

void ADMCTestCountryEdit::apply_unmodified() {
    test_edit_apply_unmodified(edit, dn);
}

// Edit should do change attribute to value.
void ADMCTestCountryEdit::apply_modified() {
    const int new_value = 8;
    const int new_value_index = combo->findData(QVariant(new_value));
    combo->setCurrentIndex(new_value_index);

    const AdObject object_before = ad.search_object(dn);
    const QString country_abbreviation_before = object_before.get_string(ATTRIBUTE_COUNTRY_ABBREVIATION);
    const QString country_before = object_before.get_string(ATTRIBUTE_COUNTRY);

    const bool apply_success = edit->apply(ad, dn);
    QVERIFY(apply_success);

    const AdObject object_after = ad.search_object(dn);
    const int current_value = object_after.get_int(ATTRIBUTE_COUNTRY_CODE);
    const QString country_abbreviation_after = object_after.get_string(ATTRIBUTE_COUNTRY_ABBREVIATION);
    const QString country_after = object_after.get_string(ATTRIBUTE_COUNTRY);
    QCOMPARE(current_value, new_value);

    // NOTE: figuring out what abbreviation and country
    // strings are actually supposed to be requires parsing
    // the countries csv file, so just check that these
    // strings changed.
    QVERIFY(country_abbreviation_after != country_abbreviation_before);
    QVERIFY(country_after != country_before);
}

QTEST_MAIN(ADMCTestCountryEdit)
