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

#include "admc_test_gpoptions_edit.h"

#include "attribute_edits/gpoptions_edit.h"

#include <QCheckBox>
#include <QFormLayout>

#define TEST_ATTRIBUTE ATTRIBUTE_FIRST_NAME

void ADMCTestGpoptionsEdit::init() {
    ADMCTest::init();

    check = new QCheckBox(parent_widget);

    edit = new GpoptionsEdit(check, parent_widget);

    const QString name = TEST_OU;
    dn = test_object_dn(name, CLASS_OU);
    const bool create_success = ad.object_add(dn, CLASS_OU);
    QVERIFY(create_success);
}

void ADMCTestGpoptionsEdit::test_emit_edited_signal() {
    bool edited_signal_emitted = false;
    connect(
        edit, &AttributeEdit::edited,
        [&edited_signal_emitted]() {
            edited_signal_emitted = true;
        });

    check->setChecked(true);
    QVERIFY(edited_signal_emitted);
}

void ADMCTestGpoptionsEdit::load() {
    const AdObject object = ad.search_object(dn);
    edit->load(ad, object);

    const QString edit_value = check->text();
    ;
    QCOMPARE(check->isChecked(), false);
}

void ADMCTestGpoptionsEdit::apply() {
    check->setChecked(true);

    const bool apply_success = edit->apply(ad, dn);
    QVERIFY(apply_success);

    const AdObject object = ad.search_object(dn);
    const QString current_value = object.get_string(ATTRIBUTE_GPOPTIONS);

    QCOMPARE(current_value, GPOPTIONS_BLOCK_INHERITANCE);
}

QTEST_MAIN(ADMCTestGpoptionsEdit)
