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

#include "admc_test_expiry_edit.h"

#include "attribute_edits/expiry_edit.h"
#include "attribute_edits/expiry_widget.h"
#include "attribute_edits/ui_expiry_widget.h"
#include "globals.h"

#include <QDateEdit>
#include <QFormLayout>
#include <QRadioButton>

void ADMCTestExpiryEdit::initTestCase_data() {
    QTest::addColumn<QString>("button_name");
    QTest::addColumn<QDate>("date");
    QTest::addColumn<QString>("value");

    QTest::newRow("end of") << "end_of_check" << QDate(2011, 11, 11) << "129655295400000000";
    QTest::newRow("never") << "never_check" << QDate() << AD_LARGE_INTEGER_DATETIME_NEVER_2;
    ;
}

void ADMCTestExpiryEdit::init() {
    ADMCTest::init();

    auto widget = new ExpiryWidget(parent_widget);

    edit = new ExpiryEdit(widget, parent_widget);

    date_edit = widget->ui->date_edit;

    QFETCH_GLOBAL(QString, button_name);
    const QHash<QString, QRadioButton *> button_map = {
        {"end_of_check", widget->ui->end_of_check},
        {"never_check", widget->ui->never_check},
    };
    button = button_map[button_name];

    // Create test user
    const QString name = TEST_USER;
    dn = test_object_dn(name, CLASS_USER);
    const bool create_success = ad.object_add(dn, CLASS_USER);
    QVERIFY(create_success);
}

void ADMCTestExpiryEdit::edited_signal() {
    bool edited_signal_emitted = false;
    connect(
        edit, &AttributeEdit::edited,
        this,
        [&edited_signal_emitted]() {
            edited_signal_emitted = true;
        });

    button->setChecked(true);
    QVERIFY(edited_signal_emitted);
}

void ADMCTestExpiryEdit::load() {
    QFETCH_GLOBAL(QDate, date);
    QFETCH_GLOBAL(QString, value);

    ad.attribute_replace_string(dn, ATTRIBUTE_ACCOUNT_EXPIRES, value);
    const AdObject object = ad.search_object(dn);
    edit->load(ad, object);

    QVERIFY(button->isChecked());
    if (date_edit->isEnabled()) {
        QCOMPARE(date_edit->date(), date);
    }
}

void ADMCTestExpiryEdit::apply_unmodified() {
    test_edit_apply_unmodified(edit, dn);
}

void ADMCTestExpiryEdit::apply() {
    QFETCH_GLOBAL(QDate, date);
    QFETCH_GLOBAL(QString, value);

    // Replace value into something different before testing
    ad.attribute_replace_string(dn, ATTRIBUTE_ACCOUNT_EXPIRES, "129655295400000001");

    button->setChecked(true);
    date_edit->setDate(date);

    const bool apply_success = edit->apply(ad, dn);
    QVERIFY(apply_success);

    const AdObject updated_object = ad.search_object(dn);
    const QString expiry_string = updated_object.get_string(ATTRIBUTE_ACCOUNT_EXPIRES);
    QCOMPARE(expiry_string, value);
}

QTEST_MAIN(ADMCTestExpiryEdit)
