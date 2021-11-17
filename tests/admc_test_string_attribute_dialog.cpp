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

void ADMCTestStringAttributeDialog::initTestCase_data() {
    QTest::addColumn<QList<QByteArray>>("value_list");
    QTest::addColumn<QString>("display_value");

    QTest::newRow("empty") << QList<QByteArray>() << "";
    QTest::newRow("non-empty") << QList<QByteArray>({"hello"}) << "hello";
}

void ADMCTestStringAttributeDialog::init() {
    ADMCTest::init();

    QFETCH_GLOBAL(QList<QByteArray>, value_list);

    dialog = new StringAttributeDialog(value_list, ATTRIBUTE_NAME, false, parent_widget);
    dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(dialog, 1000));

    line_edit = dialog->ui->edit;
}

void ADMCTestStringAttributeDialog::display_value() {
    QFETCH_GLOBAL(QString, display_value);

    const QString actual_display_value = line_edit->text();
    QCOMPARE(actual_display_value, display_value);
}

void ADMCTestStringAttributeDialog::get_value_list() {
    QFETCH_GLOBAL(QList<QByteArray>, value_list);

    const QList<QByteArray> value_list_from_dialog = dialog->get_value_list();
    QCOMPARE(value_list_from_dialog, value_list);
}

QTEST_MAIN(ADMCTestStringAttributeDialog)
