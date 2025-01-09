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

#include "admc_test_account_option_edit.h"

#include "attribute_edits/account_option_edit.h"
#include "globals.h"

#include <QCheckBox>

Q_DECLARE_METATYPE(AccountOption)

void ADMCTestAccountOptionEdit::init() {
    ADMCTest::init();

    const QList<AccountOption> option_list = [&]() {
        QList<AccountOption> out;

        for (int i = 0; i < AccountOption_COUNT; i++) {
            const AccountOption option = (AccountOption) i;
            out.append(option);
        }

        return out;
    }();

    for (const AccountOption &option : option_list) {
        auto check = new QCheckBox(parent_widget);
        auto edit = new AccountOptionEdit(check, option, parent_widget);

        check_map[option] = check;
        edit_map[option] = edit;
    }

    // Create test user
    const QString name = TEST_USER;
    dn = test_object_dn(name, CLASS_USER);
    const bool create_success = ad.object_add(dn, CLASS_USER);
    QVERIFY(create_success);
}

void ADMCTestAccountOptionEdit::test_emit_edited_signal() {
    const AccountOption option = AccountOption_SmartcardRequired;
    AccountOptionEdit *edit = edit_map[option];
    QCheckBox *check = check_map[option];

    bool edited_signal_emitted = false;
    connect(
        edit, &AttributeEdit::edited,
        this,
        [&edited_signal_emitted]() {
            edited_signal_emitted = true;
        });

    check->setChecked(true);
    QVERIFY(edited_signal_emitted);
}

void ADMCTestAccountOptionEdit::load_data() {
    QTest::addColumn<AccountOption>("option");
    QTest::addColumn<bool>("value");

    QTest::newRow("disabled") << AccountOption_Disabled << true;
    QTest::newRow("cant change pass") << AccountOption_CantChangePassword << false;
    QTest::newRow("pass expired") << AccountOption_PasswordExpired << true;
    QTest::newRow("deskey") << AccountOption_UseDesKey << false;
    QTest::newRow("smartcard") << AccountOption_SmartcardRequired << false;
    QTest::newRow("cant delegate") << AccountOption_CantDelegate << false;
    QTest::newRow("req preauth") << AccountOption_DontRequirePreauth << false;
    QTest::newRow("trusted") << AccountOption_TrustedForDelegation << false;
}

void ADMCTestAccountOptionEdit::load() {
    QFETCH(AccountOption, option);
    QFETCH(bool, value);

    AccountOptionEdit *edit = edit_map[option];
    QCheckBox *check = check_map[option];

    const AdObject object = ad.search_object(dn);
    edit->load(ad, object);

    const bool edit_value = check->isChecked();
    QCOMPARE(edit_value, value);
}

void ADMCTestAccountOptionEdit::apply_data() {
    QTest::addColumn<AccountOption>("option");

    QTest::newRow("disabled") << AccountOption_Disabled;
    QTest::newRow("cant change pass") << AccountOption_CantChangePassword;
    QTest::newRow("pass expired") << AccountOption_PasswordExpired;
    QTest::newRow("deskey") << AccountOption_UseDesKey;
    QTest::newRow("smartcard") << AccountOption_SmartcardRequired;
    QTest::newRow("cant delegate") << AccountOption_CantDelegate;
    QTest::newRow("req preauth") << AccountOption_DontRequirePreauth;
    QTest::newRow("trusted") << AccountOption_TrustedForDelegation;
}

void ADMCTestAccountOptionEdit::apply() {
    QFETCH(AccountOption, option);

    AccountOptionEdit *edit = edit_map[option];
    QCheckBox *check = check_map[option];

    const AdObject object = ad.search_object(dn);
    edit->load(ad, object);

    const bool old_value = check->isChecked();
    const bool new_value = !old_value;
    check->setChecked(new_value);

    edit->apply(ad, dn);

    QCOMPARE(check->isChecked(), new_value);

    const AdObject updated_object = ad.search_object(dn);
    const bool value_in_object = updated_object.get_account_option(option, g_adconfig);
    QCOMPARE(value_in_object, new_value);
}

void ADMCTestAccountOptionEdit::conflicts_data() {
    QTest::addColumn<AccountOption>("subject");
    QTest::addColumn<AccountOption>("blocker");

    QTest::newRow("pass-expired blocks dont-expire-pass") << AccountOption_PasswordExpired << AccountOption_DontExpirePassword;
    QTest::newRow("dont-expire-pass blocks pass-expired") << AccountOption_DontExpirePassword << AccountOption_PasswordExpired;
    QTest::newRow("pass-expired blocks cant-change-pass") << AccountOption_PasswordExpired << AccountOption_CantChangePassword;
    QTest::newRow("cant-change-pass blocks pass-expired") << AccountOption_CantChangePassword << AccountOption_PasswordExpired;
}

void ADMCTestAccountOptionEdit::conflicts() {
    QFETCH(AccountOption, subject);
    QFETCH(AccountOption, blocker);

    account_option_setup_conflicts(check_map);

    QCheckBox *subject_check = check_map[subject];
    QCheckBox *blocker_check = check_map[blocker];

    // Setup starting state
    subject_check->setChecked(false);
    blocker_check->setChecked(false);

    // Check blocker
    blocker_check->setChecked(true);

    // Attempt to check subject, should get blocked
    subject_check->click();
    close_message_box();
    QCOMPARE(subject_check->isChecked(), false);

    // Attempt to check again, conflicts should be blocked
    // every time
    for (int i = 0; i < 3; i++) {
        subject_check->click();
        close_message_box();
        QCOMPARE(subject_check->isChecked(), false);
    }
}

QTEST_MAIN(ADMCTestAccountOptionEdit)
