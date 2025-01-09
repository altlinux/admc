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

#include "admc_test_number_attribute_dialog.h"

#include "attribute_dialogs/number_attribute_dialog.h"
#include "attribute_dialogs/ui_number_attribute_dialog.h"

#include <QLineEdit>

void ADMCTestNumberAttributeDialog::init_edit(const QList<QByteArray> &value_list) {
    dialog = new NumberAttributeDialog(value_list, ATTRIBUTE_NAME, false, parent_widget);
    dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(dialog, 1000));

    line_edit = dialog->ui->edit;
}

void ADMCTestNumberAttributeDialog::display_value_data() {
    QTest::addColumn<QList<QByteArray>>("value_list");
    QTest::addColumn<QString>("display_value");

    QTest::newRow("empty") << QList<QByteArray>() << "";
    QTest::newRow("non-empty") << QList<QByteArray>({"123"}) << "123";
}

void ADMCTestNumberAttributeDialog::display_value() {
    QFETCH(QList<QByteArray>, value_list);
    QFETCH(QString, display_value);

    init_edit(value_list);

    const QString actual_display_value = line_edit->text();
    QCOMPARE(actual_display_value, display_value);
}

void ADMCTestNumberAttributeDialog::get_value_list_data() {
    QTest::addColumn<QList<QByteArray>>("value_list");

    QTest::newRow("empty") << QList<QByteArray>();
    QTest::newRow("non-empty") << QList<QByteArray>({"123"});
}

void ADMCTestNumberAttributeDialog::get_value_list() {
    QFETCH(QList<QByteArray>, value_list);

    init_edit(value_list);

    const QList<QByteArray> value_list_from_dialog = dialog->get_value_list();
    QCOMPARE(value_list_from_dialog, value_list);
}

void ADMCTestNumberAttributeDialog::validate_data() {
    QTest::addColumn<QList<QByteArray>>("value_list");
    QTest::addColumn<Qt::Key>("pressed_key");
    QTest::addColumn<QList<QByteArray>>("expected_value_list");
    QTest::addColumn<QString>("expected_display_value");

    QTest::newRow("press '1'") << QList<QByteArray>({"123"}) << Qt::Key_1 << QList<QByteArray>({"1231"}) << "1231";
    QTest::newRow("press 'a'") << QList<QByteArray>({"123"}) << Qt::Key_A << QList<QByteArray>({"123"}) << "123";
}

void ADMCTestNumberAttributeDialog::validate() {
    QFETCH(QList<QByteArray>, value_list);
    QFETCH(Qt::Key, pressed_key);
    QFETCH(QList<QByteArray>, expected_value_list);
    QFETCH(QString, expected_display_value);

    init_edit(value_list);

    QTest::keyPress(line_edit, pressed_key);
    QTest::keyPress(line_edit, Qt::Key_Enter);

    const QList<QByteArray> actual_value_list = dialog->get_value_list();
    QCOMPARE(actual_value_list, expected_value_list);

    const QString actual_display_value = line_edit->text();
    QCOMPARE(actual_display_value, expected_display_value);
}

QTEST_MAIN(ADMCTestNumberAttributeDialog)
