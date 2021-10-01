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

void ADMCTestDelegationEdit::initTestCase_data() {
    QTest::addColumn<QString>("button_name");
    QTest::addColumn<bool>("is_on");

    QTest::newRow("on") << "on_button" << true;
    QTest::newRow("off") << "off_button" << false;
}

void ADMCTestDelegationEdit::init() {
    ADMCTest::init();

    edit = new DelegationEdit(&edits, parent_widget);
    add_attribute_edit(edit);

    QFETCH_GLOBAL(QString, button_name);
    button = parent_widget->findChild<QRadioButton *>(button_name);
    QVERIFY(button);

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
