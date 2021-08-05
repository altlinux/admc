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

#include "admc_test_password_edit.h"

#include "edits/password_edit.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QDateEdit>

// NOTE: no apply_unmodified() test because password edit
// always changes object when applying

void ADMCTestPasswordEdit::init() {
    ADMCTest::init();

    edit = new PasswordEdit(&edits, parent_widget);
    add_attribute_edit(edit);

    main_edit = parent_widget->findChild<QLineEdit *>("password_main_edit");
    confirm_edit = parent_widget->findChild<QLineEdit *>("password_confirm_edit");

    // Create test user
    const QString name = TEST_USER;
    dn = test_object_dn(name, CLASS_USER);
    const bool create_success = ad.object_add(dn, CLASS_USER);
    QVERIFY(create_success);
}

void ADMCTestPasswordEdit::edited_signal() {
    bool edited_signal_emitted = false;
    connect(
        edit, &AttributeEdit::edited,
        [&edited_signal_emitted]() {
            edited_signal_emitted = true;
        });

    main_edit->setText("test");
    QVERIFY(edited_signal_emitted);
}

void ADMCTestPasswordEdit::load() {
    const AdObject object = ad.search_object(dn);
    edit->load(ad, object);

    QVERIFY(main_edit->text().isEmpty());
    QVERIFY(confirm_edit->text().isEmpty());
}

void ADMCTestPasswordEdit::verify() {
    main_edit->setText("test");
    confirm_edit->setText("no-match");

    const bool verify_success = edit->verify(ad, dn);
    close_message_box();
    QCOMPARE(verify_success, false);
}

void ADMCTestPasswordEdit::apply() {
    load();

    main_edit->setText("pass123!");
    confirm_edit->setText("pass123!");

    const QByteArray pwdLastSet_value_before = [=]() {
        const AdObject object = ad.search_object(dn);

        return object.get_value(ATTRIBUTE_PWD_LAST_SET);
    }();

    const bool apply_success = edit->apply(ad, dn);
    QVERIFY(apply_success);

    const QByteArray pwdLastSet_value_after = [=]() {
        const AdObject object = ad.search_object(dn);

        return object.get_value(ATTRIBUTE_PWD_LAST_SET);
    }();
    QVERIFY(pwdLastSet_value_after != pwdLastSet_value_before);
}

QTEST_MAIN(ADMCTestPasswordEdit)
