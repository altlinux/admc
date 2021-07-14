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

#include "admc_test_expiry_edit.h"

#include "edits/expiry_edit.h"
#include "globals.h"

#include <QFormLayout>
#include <QCheckBox>
#include <QDateEdit>

void ADMCTestExpiryEdit::init() {
    ADMCTest::init();

    edit = new ExpiryEdit(&edits, parent_widget);
    add_attribute_edit(edit);

    never_check = parent_widget->findChild<QCheckBox *>("never_check");
    end_of_check = parent_widget->findChild<QCheckBox *>("end_of_check");
    date_edit = parent_widget->findChild<QDateEdit *>("date_edit");

    // Create test user
    const QString name = TEST_USER;
    dn = test_object_dn(name, CLASS_USER);
    const bool create_success = ad.object_add(dn, CLASS_USER);
    QVERIFY(create_success);
}

void ADMCTestExpiryEdit::edited_signal_from_check() {
    bool edited_signal_emitted = false;
    connect(
        edit, &AttributeEdit::edited,
        [&edited_signal_emitted]() {
            edited_signal_emitted = true;
        });

    end_of_check->setChecked(true);
    QVERIFY(edited_signal_emitted);
}

void ADMCTestExpiryEdit::edited_signal_from_date() {
    end_of_check->setChecked(true);
   
    bool edited_signal_emitted = false;
    connect(
        edit, &AttributeEdit::edited,
        [&edited_signal_emitted]() {
            edited_signal_emitted = true;
        });

    const QDate date = date_edit->date();
    const QDate new_date = date.addDays(1);
    date_edit->setDate(new_date);
    QVERIFY(edited_signal_emitted);
}

void ADMCTestExpiryEdit::load() {
    const AdObject object = ad.search_object(dn);
    edit->load(ad, object);

    QVERIFY(never_check->isChecked());
    QVERIFY(!end_of_check->isChecked());
}

void ADMCTestExpiryEdit::apply_unmodified() {
    test_edit_apply_unmodified(edit, dn);
}

void ADMCTestExpiryEdit::apply_date() {
    load();

    end_of_check->setChecked(true);
   
    // NOTE: Skyrim release date
    const QDate new_date = QDate(2011, 11, 11);
    date_edit->setDate(new_date);

    const bool apply_success = edit->apply(ad, dn);
    QVERIFY(apply_success);

    const AdObject updated_object = ad.search_object(dn);
    const QDateTime datetime = updated_object.get_datetime(ATTRIBUTE_ACCOUNT_EXPIRES, g_adconfig);
    const QDate date = datetime.date();
    QVERIFY(date == new_date);
}

void ADMCTestExpiryEdit::apply_never() {
    apply_date();
    
    never_check->setChecked(true);
   
    const bool apply_success = edit->apply(ad, dn);
    QVERIFY(apply_success);

    const AdObject updated_object = ad.search_object(dn);
    const QString expiry_string = updated_object.get_string(ATTRIBUTE_ACCOUNT_EXPIRES);
    QVERIFY(expiry_string == AD_LARGE_INTEGER_DATETIME_NEVER_1 || expiry_string == AD_LARGE_INTEGER_DATETIME_NEVER_2);
}

QTEST_MAIN(ADMCTestExpiryEdit)
