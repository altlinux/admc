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

#include "admc_test_unlock_edit.h"

#include "attribute_edits/unlock_edit.h"

#include <QCheckBox>
#include <QFormLayout>

// NOTE: this doesn't really "lock" accounts. Accounts can
// only be locked by the server and lockout time only
// displays the state. Since unlock edit modifies lockout
// time, test it like this.
#define LOCKOUT_LOCKED_VALUE "1"

void ADMCTestUnlockEdit::init() {
    ADMCTest::init();

    checkbox = new QCheckBox(parent_widget);

    unlock_edit = new UnlockEdit(checkbox, parent_widget);

    // Create test user
    const QString name = TEST_USER;
    dn = test_object_dn(name, CLASS_USER);
    const bool create_success = ad.object_add(dn, CLASS_USER);
    QVERIFY(create_success);
}

// edited() signal should be emitted when checkbox is toggled
void ADMCTestUnlockEdit::test_emit_edited_signal() {
    bool edited_signal_emitted = false;
    connect(
        unlock_edit, &AttributeEdit::edited,
        this,
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

// Edit should start out as unchecked after loading user
void ADMCTestUnlockEdit::unchecked_after_load() {
    load_locked_user_into_edit();

    const bool apply_success = unlock_edit->apply(ad, dn);
    QVERIFY(apply_success);

    const bool checkbox_is_unchecked = (!checkbox->isChecked());
    QVERIFY2(checkbox_is_unchecked, "Checkbox wasn't unchecked after load() call");
}

void ADMCTestUnlockEdit::apply_unmodified() {
    test_edit_apply_unmodified(unlock_edit, dn);
}

// Edit should do nothing if checkbox is unchecked (locked
// user stays locked)
void ADMCTestUnlockEdit::test_apply_unchecked() {
    load_locked_user_into_edit();

    checkbox->setChecked(false);

    const bool apply_success = unlock_edit->apply(ad, dn);
    QVERIFY(apply_success);
    QVERIFY2(!user_is_unlocked(), "Edit unlocked user when the checkbox was unchecked. Edit should've done nothing.");
}

// Edit should unlock locked user if checkbox is checked
void ADMCTestUnlockEdit::test_apply_checked() {
    load_locked_user_into_edit();

    checkbox->setChecked(true);

    const bool apply_success = unlock_edit->apply(ad, dn);
    QVERIFY(apply_success);
    QVERIFY2(user_is_unlocked(), "Edit failed to unlock user.");
}

// Edit should uncheck the checkbox after applying
void ADMCTestUnlockEdit::uncheck_after_apply() {
    checkbox->setChecked(true);

    const bool apply_success = unlock_edit->apply(ad, dn);
    QVERIFY(apply_success);

    const bool checkbox_is_unchecked = (!checkbox->isChecked());
    QVERIFY2(checkbox_is_unchecked, "Checkbox wasn't unchecked after apply() call");
}

bool ADMCTestUnlockEdit::user_is_unlocked() {
    const AdObject object = ad.search_object(dn);
    const QString lockout_time = object.get_string(ATTRIBUTE_LOCKOUT_TIME);
    const bool is_unlocked = (lockout_time == LOCKOUT_UNLOCKED_VALUE);

    return is_unlocked;
}

void ADMCTestUnlockEdit::load_locked_user_into_edit() {
    // Lock test user
    ad.attribute_replace_string(dn, ATTRIBUTE_LOCKOUT_TIME, LOCKOUT_LOCKED_VALUE);

    // Load user into edit
    const AdObject object = ad.search_object(dn);
    unlock_edit->load(ad, object);
}

QTEST_MAIN(ADMCTestUnlockEdit)
