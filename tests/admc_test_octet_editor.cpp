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

#include "admc_test_octet_editor.h"

#include "editors/octet_editor.h"
#include "editors/ui_octet_editor.h"

#include <QPlainTextEdit>
#include <QComboBox>

const QList<QByteArray> value_bytes = {QByteArray("12345")};

void ADMCTestOctetEditor::initTestCase_data() {
    QTest::addColumn<int>("index");
    QTest::addColumn<int>("other_index");
    QTest::addColumn<QString>("value");

    QTest::newRow("hex") << 0 << 1 << "31 32 33 34 35";
    QTest::newRow("bin") << 1 << 2 << "00110001 00110010 00110011 00110100 00110101";
    QTest::newRow("dec") << 2 << 3 << "049 050 051 052 053";
    QTest::newRow("oct") << 3 << 0 << "061 062 063 064 065";
    ;
}

void ADMCTestOctetEditor::init() {
    ADMCTest::init();

    edit = new OctetEditor(ATTRIBUTE_DESCRIPTION, parent_widget);
    edit->open();
    QVERIFY(QTest::qWaitForWindowExposed(edit, 1000));

    format_combo = edit->ui->format_combo;
    text_edit = edit->ui->edit;
}

void ADMCTestOctetEditor::display() {
    QFETCH_GLOBAL(int, index);
    QFETCH_GLOBAL(QString, value);

    edit->load(value_bytes);

    format_combo->setCurrentIndex(index);
    QCOMPARE(text_edit->toPlainText(), value);
}

// Check that edit correctly converts formatted strings back
// to bytes for each format
void ADMCTestOctetEditor::get_new_values() {
    QFETCH_GLOBAL(int, index);
    QFETCH_GLOBAL(QString, value);

    edit->load({});

    format_combo->setCurrentIndex(index);
    text_edit->setPlainText(value);

    QCOMPARE(edit->get_new_values(), value_bytes);
}

void ADMCTestOctetEditor::handle_empty_value() {
    QFETCH_GLOBAL(int, index);
    QFETCH_GLOBAL(int, other_index);
    QFETCH_GLOBAL(QString, value);

    edit->load({});
    
    // Check that empty value correctly loads and is
    // displayed as empty string
    format_combo->setCurrentIndex(index);
    QCOMPARE(text_edit->toPlainText(), "");

    // Check that edit correctly switches formats when
    // value is empty
    format_combo->setCurrentIndex(other_index);
    QCOMPARE(format_combo->currentIndex(), other_index);
}

// Check that when incorrectly formatted value is entered,
// edit fails to switch to different format
void ADMCTestOctetEditor::handle_incorrect_input() {
    QFETCH_GLOBAL(int, index);
    QFETCH_GLOBAL(int, other_index);
    QFETCH_GLOBAL(QString, value);

    edit->load({});
    
    text_edit->setPlainText("");

    format_combo->setCurrentIndex(index);
    text_edit->setPlainText("incorrect format");

    format_combo->setCurrentIndex(other_index);
    close_message_box();

    QCOMPARE(format_combo->currentIndex(), index);
}

QTEST_MAIN(ADMCTestOctetEditor)
