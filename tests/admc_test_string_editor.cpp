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

#include "admc_test_string_editor.h"

#include "editors/string_editor.h"

#include <QLineEdit>

void ADMCTestStringEditor::init() {
    ADMCTest::init();

    edit = new StringEditor(ATTRIBUTE_NAME, parent_widget);
    edit->open();
    QVERIFY(QTest::qWaitForWindowExposed(edit, 1000));

    line_edit = edit->findChild<QLineEdit *>();
    QVERIFY(line_edit);
}

void ADMCTestStringEditor::load_empty() {
    edit->load({});
    QVERIFY(line_edit->text().isEmpty());
}

void ADMCTestStringEditor::load() {
    edit->load({QByteArray("hello")});
    QVERIFY(line_edit->text() == "hello");
}

void ADMCTestStringEditor::get_new_values() {
    const QList<QByteArray> correct_value_list = {QByteArray("hello")};
    edit->load(correct_value_list);
    const QList<QByteArray> value_list = edit->get_new_values();
    QVERIFY(correct_value_list == value_list);
}

QTEST_MAIN(ADMCTestStringEditor)
