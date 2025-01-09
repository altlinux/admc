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

#include "admc_test_sam_name_edit.h"

#include "attribute_edits/sam_name_edit.h"

#include <QLineEdit>

#define TEST_ATTRIBUTE ATTRIBUTE_FIRST_NAME

void ADMCTestSamNameEdit::init() {
    ADMCTest::init();

    line_edit = new QLineEdit(parent_widget);
    auto domain_edit = new QLineEdit(parent_widget);

    edit = new SamNameEdit(line_edit, domain_edit, parent_widget);
}

void ADMCTestSamNameEdit::verify_data() {
    QTest::addColumn<QString>("value");
    QTest::addColumn<bool>("correct_result");

    const QString bad_chars_string = SAM_NAME_BAD_CHARS;

    for (int i = 0; i < bad_chars_string.length(); i++) {
        const QChar bad_char = bad_chars_string.at(i);

        const QString bad_char_string = QString(bad_char);
        const QByteArray bad_char_bytes = bad_char_string.toUtf8();

        const QString value = QString("test%1value").arg(bad_char);

        QTest::newRow(bad_char_bytes.constData()) << value << false;
    }

    QTest::newRow("ends with dot") << "testvalue." << false;
    QTest::newRow("contains dot") << "test.value" << true;
}

void ADMCTestSamNameEdit::verify() {
    QFETCH(QString, value);
    QFETCH(bool, correct_result);

    line_edit->setText(value);

    const bool actual_result = edit->verify(ad, QString());

    QCOMPARE(actual_result, correct_result);
}

void ADMCTestSamNameEdit::trim() {
    const QString dn = test_object_dn(TEST_USER, CLASS_USER);
    const bool create_success = ad.object_add(dn, CLASS_USER);
    QVERIFY(create_success);
    QVERIFY(object_exists(dn));

    const AdObject object = ad.search_object(dn);

    edit->load(ad, object);

    line_edit->setText(" trim-test ");

    const bool apply_success = edit->apply(ad, dn);
    QVERIFY(apply_success);

    const QString actual_sam_name = [&]() {
        const AdObject object_after_apply = ad.search_object(dn);
        const QString out = object_after_apply.get_string(ATTRIBUTE_SAM_ACCOUNT_NAME);

        return out;
    }();
    const QString expected_sam_name = "trim-test";
    QCOMPARE(actual_sam_name, expected_sam_name);
}

QTEST_MAIN(ADMCTestSamNameEdit)
