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

#include "admc_test_protect_deletion_edit.h"

#include "attribute_edits/protect_deletion_edit.h"

#include <QCheckBox>

void ADMCTestProtectDeletionEdit::init() {
    ADMCTest::init();

    checkbox = new QCheckBox(parent_widget);

    edit = new ProtectDeletionEdit(checkbox, parent_widget);

    dn = test_object_dn(TEST_OU, CLASS_OU);
    const bool create_success = ad.object_add(dn, CLASS_OU);
    QVERIFY(create_success);
}

void ADMCTestProtectDeletionEdit::edited_signal_data() {
    QTest::addColumn<bool>("start_state");

    QTest::newRow("when checked") << false;
    QTest::newRow("when unchecked") << true;
}

void ADMCTestProtectDeletionEdit::edited_signal() {
    QFETCH(bool, start_state);
    const bool end_state = !start_state;

    checkbox->setChecked(start_state);

    bool edited_signal_emitted = false;
    connect(
        edit, &AttributeEdit::edited,
        this,
        [&edited_signal_emitted]() {
            edited_signal_emitted = true;
        });

    checkbox->setChecked(end_state);
    QVERIFY(edited_signal_emitted);
}

void ADMCTestProtectDeletionEdit::apply_data() {
    QTest::addColumn<bool>("is_checked");
    QTest::addColumn<bool>("expected_delete_success");

    QTest::newRow("protected") << true << false;
    QTest::newRow("not protected") << false << true;
}

void ADMCTestProtectDeletionEdit::apply() {
    QFETCH(bool, is_checked);
    QFETCH(bool, expected_delete_success);

    const AdObject object = ad.search_object(dn);
    edit->load(ad, object);

    checkbox->setChecked(is_checked);
    const bool apply_success = edit->apply(ad, dn);
    QVERIFY(apply_success);

    qInfo() << "Error relating to \"Insufficient access\" is part of the test";
    const bool actual_delete_success = ad.object_delete(dn);
    QCOMPARE(actual_delete_success, expected_delete_success);

    // Finally, disable protection so test suite can
    // delete this object to clean up
    if (!actual_delete_success) {
        checkbox->setChecked(false);
        const bool apply_2_success = edit->apply(ad, dn);
        QVERIFY(apply_2_success);
    }
}

QTEST_MAIN(ADMCTestProtectDeletionEdit)
