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

#include "admc_test_datetime_attribute_dialog.h"

#include "attribute_dialogs/datetime_attribute_dialog.h"
#include "attribute_dialogs/ui_datetime_attribute_dialog.h"

#include <QDateTimeEdit>

void ADMCTestDatetimeAttributeDialog::initTestCase_data() {
    QTest::addColumn<QList<QByteArray>>("value_list");
    QTest::addColumn<QDateTime>("display_value");

    QTest::newRow("non-empty") << QList<QByteArray>({"20210706131457.0Z"}) << QDateTime(QDate(2021, 7, 6), QTime(13, 14, 57));
}

void ADMCTestDatetimeAttributeDialog::init() {
    ADMCTest::init();

    QFETCH_GLOBAL(QList<QByteArray>, value_list);

    dialog = new DatetimeAttributeDialog(value_list, ATTRIBUTE_WHEN_CHANGED, false, parent_widget);
    dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(dialog, 1000));

    datetime_edit = dialog->ui->edit;
}

void ADMCTestDatetimeAttributeDialog::display_value() {
    QFETCH_GLOBAL(QDateTime, display_value);

    const QDateTime actual_display_value = datetime_edit->dateTime();
    QCOMPARE(actual_display_value, display_value);
}

void ADMCTestDatetimeAttributeDialog::get_value_list() {
    QFETCH_GLOBAL(QList<QByteArray>, value_list);

    const QList<QByteArray> actualy_value_list = dialog->get_value_list();
    QCOMPARE(actualy_value_list, value_list);
}

QTEST_MAIN(ADMCTestDatetimeAttributeDialog)
