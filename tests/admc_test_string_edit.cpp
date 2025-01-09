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

#include "admc_test_string_edit.h"

#include "attribute_edits/string_edit.h"

#include <QFormLayout>
#include <QLineEdit>

#define TEST_ATTRIBUTE ATTRIBUTE_FIRST_NAME

void ADMCTestStringEdit::init() {
    ADMCTest::init();

    line_edit = new QLineEdit(parent_widget);

    edit = new StringEdit(line_edit, TEST_ATTRIBUTE, parent_widget);

    // Create test user
    const QString name = TEST_USER;
    dn = test_object_dn(name, CLASS_USER);
    const bool create_success = ad.object_add(dn, CLASS_USER);
    QVERIFY(create_success);
}

// edited() signal should be emitted when lineedit is edited
void ADMCTestStringEdit::test_emit_edited_signal() {
    bool edited_signal_emitted = false;
    connect(
        edit, &AttributeEdit::edited,
        this,
        [&edited_signal_emitted]() {
            edited_signal_emitted = true;
        });

    line_edit->setText("test");
    QVERIFY(edited_signal_emitted);
}

// Edit should contain current attribute value after load()
// call
void ADMCTestStringEdit::load() {
    const QString test_value = "test value";
    // Set attribute value
    ad.attribute_replace_string(dn, TEST_ATTRIBUTE, test_value);

    // Load user into edit
    const AdObject object = ad.search_object(dn);
    edit->load(ad, object);

    const QString edit_value = line_edit->text();
    ;
    QCOMPARE(edit_value, test_value);
}

void ADMCTestStringEdit::apply_unmodified() {
    test_edit_apply_unmodified(edit, dn);
}

// Edit should do change attribute to value
void ADMCTestStringEdit::apply_modified() {
    const QString new_value = "new value";
    line_edit->setText(new_value);

    const bool apply_success = edit->apply(ad, dn);
    QVERIFY(apply_success);

    const AdObject object = ad.search_object(dn);
    const QString current_value = object.get_string(TEST_ATTRIBUTE);

    QCOMPARE(current_value, new_value);
}

// Apply should trim leading and trailing spaces
void ADMCTestStringEdit::apply_trim() {
    const QString new_value = " new value ";
    line_edit->setText(new_value);

    const bool apply_success = edit->apply(ad, dn);
    QVERIFY(apply_success);

    const AdObject object = ad.search_object(dn);
    const QString current_value = object.get_string(TEST_ATTRIBUTE);

    QCOMPARE(current_value, new_value.trimmed());
}

QTEST_MAIN(ADMCTestStringEdit)
