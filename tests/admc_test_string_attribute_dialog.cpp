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

#include "admc_test_string_attribute_dialog.h"

#include "attribute_dialogs/string_attribute_dialog.h"
#include "attribute_dialogs/ui_string_attribute_dialog.h"

#include <QLineEdit>

void ADMCTestStringAttributeDialog::init() {
    ADMCTest::init();

    edit = new StringAttributeDialog(parent_widget);
    edit->set_attribute(ATTRIBUTE_NAME);
    edit->open();
    QVERIFY(QTest::qWaitForWindowExposed(edit, 1000));

    line_edit = edit->ui->edit;
}

void ADMCTestStringAttributeDialog::set_value_list_empty() {
    edit->set_value_list({});
    QVERIFY(line_edit->text().isEmpty());
}

void ADMCTestStringAttributeDialog::set_value_list() {
    edit->set_value_list({QByteArray("hello")});
    QCOMPARE(line_edit->text(), "hello");
}

void ADMCTestStringAttributeDialog::get_value_list() {
    const QList<QByteArray> correct_value_list = {QByteArray("hello")};
    edit->set_value_list(correct_value_list);
    const QList<QByteArray> value_list = edit->get_value_list();
    QCOMPARE(correct_value_list, value_list);
}

QTEST_MAIN(ADMCTestStringAttributeDialog)
