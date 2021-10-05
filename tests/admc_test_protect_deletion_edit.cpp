/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

#include "edits/protect_deletion_edit.h"

#include <QCheckBox>
#include <QFormLayout>

// NOTE: this doesn't really "lock" accounts. Accounts can
// only be locked by the server and lockout time only
// displays the state. Since unlock edit modifies lockout
// time, test it like this.
#define LOCKOUT_LOCKED_VALUE "1"

void ADMCTestProtectDeletionEdit::init() {
    ADMCTest::init();

    checkbox = new QCheckBox(parent_widget);

    edit = new ProtectDeletionEdit(checkbox, &edits, parent_widget);
    add_attribute_edit(edit);

    dn = test_object_dn(TEST_OU, CLASS_OU);
    const bool create_success = ad.object_add(dn, CLASS_OU);
    QVERIFY(create_success);
}

// edited() signal should be emitted when checkbox is toggled
void ADMCTestProtectDeletionEdit::emit_edited_signal() {
    bool edited_signal_emitted = false;
    connect(
        edit, &AttributeEdit::edited,
        [&edited_signal_emitted]() {
            edited_signal_emitted = true;
        });

    // Check checkbox
    checkbox->setChecked(true);
    QVERIFY(edited_signal_emitted);

    // Unheck checkbox
    edited_signal_emitted = false;
    checkbox->setChecked(false);
    QVERIFY(edited_signal_emitted);
}

// Edit should unlock locked user if checkbox is checked
void ADMCTestProtectDeletionEdit::apply() {
    const AdObject object = ad.search_object(dn);
    edit->load(ad, object);

    // Enable protection against deletion
    checkbox->setChecked(true);
    const bool apply_success = edit->apply(ad, dn);
    QVERIFY(apply_success);

    // Try to delete, should fail
    qInfo() << "Error relating to \"Insufficient access\" is part of the test";
    const bool delete_success = ad.object_delete(dn);
    QCOMPARE(delete_success, false);

    // Disable protection against deletion
    checkbox->setChecked(false);
    const bool apply_2_success = edit->apply(ad, dn);
    QVERIFY(apply_2_success);

    const bool delete_2_success = ad.object_delete(dn);
    QVERIFY(delete_2_success);
}

QTEST_MAIN(ADMCTestProtectDeletionEdit)
