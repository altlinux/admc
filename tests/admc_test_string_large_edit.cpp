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

#include "admc_test_string_large_edit.h"

#include "attribute_edits/string_large_edit.h"

#include <QFormLayout>
#include <QPlainTextEdit>

#define TEST_ATTRIBUTE ATTRIBUTE_FIRST_NAME

void ADMCTestStringLargeEdit::init() {
    ADMCTest::init();

    text_edit = new QPlainTextEdit(parent_widget);

    edit = new StringLargeEdit(text_edit, TEST_ATTRIBUTE, parent_widget);

    // Create test user
    const QString name = TEST_USER;
    dn = test_object_dn(name, CLASS_USER);
    const bool create_success = ad.object_add(dn, CLASS_USER);
    QVERIFY(create_success);
}

// edited() signal should be emitted when lineedit is edited
void ADMCTestStringLargeEdit::test_emit_edited_signal() {
    bool edited_signal_emitted = false;
    connect(
        edit, &AttributeEdit::edited,
        this,
        [&edited_signal_emitted]() {
            edited_signal_emitted = true;
        });

    text_edit->setPlainText("test");
    QVERIFY(edited_signal_emitted);
}

// Edit should contain current attribute value after load()
// call
void ADMCTestStringLargeEdit::load() {
    const QString test_value = "test value";
    // Set attribute value
    ad.attribute_replace_string(dn, TEST_ATTRIBUTE, test_value);

    // Load user into edit
    const AdObject object = ad.search_object(dn);
    edit->load(ad, object);

    const QString edit_value = text_edit->toPlainText();
    ;
    QCOMPARE(edit_value, test_value);
}

void ADMCTestStringLargeEdit::apply_unmodified() {
    test_edit_apply_unmodified(edit, dn);
}

// Edit should do change attribute to value
void ADMCTestStringLargeEdit::apply() {
    const QString new_value = "new value";
    text_edit->setPlainText(new_value);

    const bool apply_success = edit->apply(ad, dn);
    QVERIFY(apply_success);

    const AdObject object = ad.search_object(dn);
    const QString current_value = object.get_string(TEST_ATTRIBUTE);

    QCOMPARE(current_value, new_value);
}

QTEST_MAIN(ADMCTestStringLargeEdit)
