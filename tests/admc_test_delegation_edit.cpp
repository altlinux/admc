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

#include "admc_test_delegation_edit.h"

#include "attribute_edits/delegation_edit.h"
#include "globals.h"

#include <QRadioButton>

void ADMCTestDelegationEdit::initTestCase_data() {
    QTest::addColumn<bool>("use_on_button");
    QTest::addColumn<bool>("is_on");

    QTest::newRow("on") << true << true;
    QTest::newRow("off") << false << false;
}

void ADMCTestDelegationEdit::init() {
    ADMCTest::init();

    auto on_button = new QRadioButton(parent_widget);
    auto off_button = new QRadioButton(parent_widget);

    edit = new DelegationEdit(off_button, on_button, parent_widget);

    QFETCH_GLOBAL(bool, use_on_button);
    button = [&]() {
        if (use_on_button) {
            return on_button;
        } else {
            return off_button;
        }
    }();

    const QString name = TEST_USER;
    dn = test_object_dn(name, CLASS_USER);
    const bool create_success = ad.object_add(dn, CLASS_USER);
    QVERIFY(create_success);
}

void ADMCTestDelegationEdit::emit_edited_signal() {
    bool edited_signal_emitted = false;
    connect(
        edit, &AttributeEdit::edited,
        this,
        [&edited_signal_emitted]() {
            edited_signal_emitted = true;
        });

    button->click();
    QVERIFY(edited_signal_emitted);
}

void ADMCTestDelegationEdit::load() {
    QFETCH_GLOBAL(bool, is_on);

    const bool success = ad.user_set_account_option(dn, AccountOption_TrustedForDelegation, is_on);
    QVERIFY(success);

    const AdObject object = ad.search_object(dn);
    edit->load(ad, object);

    QVERIFY(button->isChecked());
}

void ADMCTestDelegationEdit::apply_unmodified() {
    test_edit_apply_unmodified(edit, dn);
}

void ADMCTestDelegationEdit::apply() {
    QFETCH_GLOBAL(bool, is_on);

    // First set current state to opposite
    const bool success = ad.user_set_account_option(dn, AccountOption_TrustedForDelegation, !is_on);
    QVERIFY(success);

    const AdObject object = ad.search_object(dn);
    edit->load(ad, object);

    button->click();

    const bool apply_success = edit->apply(ad, dn);
    QVERIFY(apply_success);

    const AdObject updated_object = ad.search_object(dn);
    const bool updated_state = updated_object.get_account_option(AccountOption_TrustedForDelegation, g_adconfig);
    QCOMPARE(updated_state, is_on);
}

QTEST_MAIN(ADMCTestDelegationEdit)
