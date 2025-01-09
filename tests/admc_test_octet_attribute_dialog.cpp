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

#include "admc_test_octet_attribute_dialog.h"

#include "attribute_dialogs/octet_attribute_dialog.h"
#include "attribute_dialogs/ui_octet_attribute_dialog.h"

#include <QComboBox>
#include <QPlainTextEdit>

void ADMCTestOctetAttributeDialog::initTestCase_data() {
    QTest::addColumn<QList<QByteArray>>("value_list");
    QTest::addColumn<int>("index");
    QTest::addColumn<int>("other_index");
    QTest::addColumn<QString>("display_value");

    const QList<QByteArray> test_value_list = {QByteArray("12345")};

    QTest::newRow("hex") << test_value_list << 0 << 1 << "31 32 33 34 35";
    QTest::newRow("bin") << test_value_list << 1 << 2 << "00110001 00110010 00110011 00110100 00110101";
    QTest::newRow("dec") << test_value_list << 2 << 3 << "049 050 051 052 053";
    QTest::newRow("oct") << test_value_list << 3 << 0 << "061 062 063 064 065";
    QTest::newRow("empty") << QList<QByteArray>() << 0 << 1 << "";
    ;
}

void ADMCTestOctetAttributeDialog::init() {
    ADMCTest::init();

    QFETCH_GLOBAL(QList<QByteArray>, value_list);

    dialog = new OctetAttributeDialog(value_list, ATTRIBUTE_DESCRIPTION, false, parent_widget);
    dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(dialog, 1000));

    format_combo = dialog->ui->format_combo;
    text_edit = dialog->ui->edit;
}

void ADMCTestOctetAttributeDialog::display_value() {
    QFETCH_GLOBAL(QString, display_value);
    QFETCH_GLOBAL(int, index);

    format_combo->setCurrentIndex(index);

    const QString actual_display_value = text_edit->toPlainText();
    QCOMPARE(actual_display_value, display_value);
}

// Check that edit correctly converts formatted strings back
// to bytes for each format
void ADMCTestOctetAttributeDialog::get_value_list() {
    QFETCH_GLOBAL(QList<QByteArray>, value_list);

    const QList<QByteArray> actual_value_list = dialog->get_value_list();
    QCOMPARE(actual_value_list, value_list);
}

// Check that when incorrectly formatted value is entered,
// edit fails to switch to different format
void ADMCTestOctetAttributeDialog::handle_incorrect_input() {
    QFETCH_GLOBAL(int, index);
    QFETCH_GLOBAL(int, other_index);

    format_combo->setCurrentIndex(index);
    text_edit->setPlainText("incorrect format");

    format_combo->setCurrentIndex(other_index);
    close_message_box();

    QCOMPARE(format_combo->currentIndex(), index);
}

QTEST_MAIN(ADMCTestOctetAttributeDialog)
