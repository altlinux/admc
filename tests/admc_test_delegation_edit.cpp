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

#include "admc_test_delegation_edit.h"

#include "edits/delegation_edit.h"
#include "globals.h"

#include <QRadioButton>

void ADMCTestDelegationEdit::init() {
    ADMCTest::init();

    edit = new DelegationEdit(&edits, parent_widget);
    add_attribute_edit(edit);

    off_button = parent_widget->findChild<QRadioButton *>("off_button");
    QVERIFY(off_button);

    on_button = parent_widget->findChild<QRadioButton *>("on_button");
    QVERIFY(on_button);

    const QString name = TEST_USER;
    dn = test_object_dn(name, CLASS_USER);
    const bool create_success = ad.object_add(dn, CLASS_USER);
    QVERIFY(create_success);
}

void ADMCTestDelegationEdit::emit_edited_signal() {
    bool edited_signal_emitted = false;
    connect(
        edit, &AttributeEdit::edited,
        [&edited_signal_emitted]() {
            edited_signal_emitted = true;
        });

    on_button->click();
    QVERIFY(edited_signal_emitted);

    edited_signal_emitted = false;
    off_button->click();
    QVERIFY(edited_signal_emitted);
}

void ADMCTestDelegationEdit::load() {
    auto do_test = [&](const bool is_on) {
        const bool success = ad.user_set_account_option(dn, AccountOption_TrustedForDelegation, is_on);
        QVERIFY(success);

        const AdObject object = ad.search_object(dn);
        edit->load(ad, object);

        QRadioButton *button_that_should_be_checked = [&]() {
            if (is_on) {
                return on_button;
            } else {
                return off_button;
            }
        }();

        QVERIFY(button_that_should_be_checked->isChecked());
    };

    do_test(true);
    do_test(false);
}

void ADMCTestDelegationEdit::apply() {
    auto do_test = [&](const bool is_on) {
        // First set current state to opposite
        const bool success = ad.user_set_account_option(dn, AccountOption_TrustedForDelegation, !is_on);
        QVERIFY(success);

        const AdObject object = ad.search_object(dn);
        edit->load(ad, object);

        QRadioButton *button_to_click = [&]() {
            if (is_on) {
                return on_button;
            } else {
                return off_button;
            }
        }();

        button_to_click->click();

        const bool apply_success = edit->apply(ad, dn);
        QVERIFY(apply_success);

        const AdObject updated_object = ad.search_object(dn);
        const bool updated_state = updated_object.get_account_option(AccountOption_TrustedForDelegation, g_adconfig);
        QVERIFY(updated_state == is_on);
    };

    do_test(true);
    do_test(false);
}

QTEST_MAIN(ADMCTestDelegationEdit)
