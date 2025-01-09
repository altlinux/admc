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

#include "admc_test_string_attribute_dialog.h"

#include "attribute_dialogs/string_attribute_dialog.h"
#include "attribute_dialogs/ui_string_attribute_dialog.h"

#include <QPlainTextEdit>

#define TEST_ATTRIBUTE ATTRIBUTE_CN

void ADMCTestStringAttributeDialog::init_edit(const QList<QByteArray> &value_list) {
    dialog = new StringAttributeDialog(value_list, TEST_ATTRIBUTE, false, parent_widget);
    dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(dialog, 1000));

    text_edit = dialog->ui->edit;
}

void ADMCTestStringAttributeDialog::display_value_data() {
    QTest::addColumn<QList<QByteArray>>("value_list");
    QTest::addColumn<QString>("expected_display_value");

    QTest::newRow("empty") << QList<QByteArray>() << "";
    QTest::newRow("non-empty") << QList<QByteArray>({"hello"}) << "hello";
}

void ADMCTestStringAttributeDialog::display_value() {
    QFETCH(QList<QByteArray>, value_list);
    QFETCH(QString, expected_display_value);

    init_edit(value_list);

    const QString actual_display_value = text_edit->toPlainText();
    QCOMPARE(actual_display_value, expected_display_value);
}

void ADMCTestStringAttributeDialog::get_value_list_data() {
    QTest::addColumn<QList<QByteArray>>("value_list");

    QTest::newRow("empty") << QList<QByteArray>();
    QTest::newRow("non-empty") << QList<QByteArray>({"hello"});
}

void ADMCTestStringAttributeDialog::get_value_list() {
    QFETCH(QList<QByteArray>, value_list);

    init_edit(value_list);

    const QList<QByteArray> actual_value_list = dialog->get_value_list();
    const QList<QByteArray> expected_value_list = value_list;
    QCOMPARE(actual_value_list, expected_value_list);
}

void ADMCTestStringAttributeDialog::limit_length() {
    const auto value_list = QList<QByteArray>({"hello"});

    init_edit(value_list);

    const int range_upper = ad.adconfig()->get_attribute_range_upper(TEST_ATTRIBUTE);

    QVERIFY(range_upper > 0);

    const QString new_value = QString(range_upper + 10, 'a');
    text_edit->setPlainText(new_value);

    const QString current_value = text_edit->toPlainText();
    const int current_value_length = current_value.length();

    QCOMPARE(current_value_length, range_upper);
}

QTEST_MAIN(ADMCTestStringAttributeDialog)
