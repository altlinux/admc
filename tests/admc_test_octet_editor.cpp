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

#include <QPlainTextEdit>
#include <QComboBox>

enum FormatIndex {
    FormatIndex_Hex = 0,
    FormatIndex_Bin,
    FormatIndex_Dec,
    FormatIndex_Oct,
};

const QList<FormatIndex> index_list = {
    FormatIndex_Hex,
    FormatIndex_Bin,
    FormatIndex_Dec,
    FormatIndex_Oct,
};

const QHash<FormatIndex, QString> formatted_value_list = {
    {FormatIndex_Hex, "31 32 33 34 35"},
    {FormatIndex_Bin, "00110001 00110010 00110011 00110100 00110101"},
    {FormatIndex_Dec, "049 050 051 052 053"},
    {FormatIndex_Oct, "061 062 063 064 065"},
};

const QList<QByteArray> value_bytes = {QByteArray("12345")};

FormatIndex next_format_index(const FormatIndex index);

void ADMCTestOctetEdit::init() {
    ADMCTest::init();

    edit = new OctetEditor(ATTRIBUTE_DESCRIPTION, parent_widget);
    edit->open();
    QVERIFY(QTest::qWaitForWindowExposed(edit, 1000));

    format_combo = edit->findChild<QComboBox *>();
    QVERIFY(format_combo);

    text_edit = edit->findChild<QPlainTextEdit *>();
    QVERIFY(text_edit);
}

void ADMCTestOctetEdit::display() {
    edit->load(value_bytes);

    for (const FormatIndex &index : index_list) {
        format_combo->setCurrentIndex(index);
        const QString correct_value = formatted_value_list[index];
        const QString value = text_edit->toPlainText();
        QVERIFY(value == correct_value);
    }
}

// Check that edit correctly converts formatted strings back
// to bytes for each format
void ADMCTestOctetEdit::get_new_values() {
    edit->load({});

    for (const FormatIndex &index : index_list) {
        format_combo->setCurrentIndex(index);
        const QString value = formatted_value_list[index];
        text_edit->setPlainText(value);

        const QList<QByteArray> current_value_bytes = edit->get_new_values();
        QVERIFY(current_value_bytes == value_bytes);
    }
}

void ADMCTestOctetEdit::handle_empty_value() {
    edit->load({});
    
    for (const FormatIndex &index : index_list) {
        // Check that empty value correctly loads and is
        // displayed as empty string
        format_combo->setCurrentIndex(index);
        QVERIFY(text_edit->toPlainText().isEmpty());

        // Check that edit correctly switches formats when
        // value is empty
        const FormatIndex next_index = next_format_index(index);
        format_combo->setCurrentIndex(next_index);
        QVERIFY(format_combo->currentIndex() == next_index);
    }
}

// Check that when incorrectly formatted value is entered,
// edit fails to switch to different format
void ADMCTestOctetEdit::handle_incorrect_input() {
    edit->load({});
    
    for (const FormatIndex &index : index_list) {
        text_edit->setPlainText("");

        format_combo->setCurrentIndex(index);
        text_edit->setPlainText("incorrect format");

        const FormatIndex next_index = next_format_index(index);
        format_combo->setCurrentIndex(next_index);
        close_message_box();

        QVERIFY(format_combo->currentIndex() == index);
    }
}

FormatIndex next_format_index(const FormatIndex index) {
    int out = index + 1;

    if (!index_list.contains((FormatIndex) out)) {
        out = index_list[0];
    }

    return (FormatIndex) out;
}

QTEST_MAIN(ADMCTestOctetEdit)
