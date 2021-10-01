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

#include "admc_test_datetime_editor.h"

#include "editors/datetime_editor.h"

#include <QDateTimeEdit>

void ADMCTestDateTimeEditor::init() {
    ADMCTest::init();

    edit = new DateTimeEditor(ATTRIBUTE_WHEN_CHANGED, parent_widget);
    edit->open();
    QVERIFY(QTest::qWaitForWindowExposed(edit, 1000));

    datetime_edit = edit->findChild<QDateTimeEdit *>();
    QVERIFY(datetime_edit);
}

void ADMCTestDateTimeEditor::load() {
    edit->load({QByteArray("20210706131457.0Z")});
    const QDateTime correct_datetime = QDateTime(QDate(2021, 7, 6), QTime(13, 14, 57));
    QCOMPARE(datetime_edit->dateTime(), correct_datetime);
}

// NOTE: datetime edit always returns empty list, see
// comment in it's source
void ADMCTestDateTimeEditor::get_new_values() {
    edit->load({QByteArray("20210706131457.0Z")});
    const QList<QByteArray> value_list = edit->get_new_values();
    QVERIFY(value_list.isEmpty());
}

QTEST_MAIN(ADMCTestDateTimeEditor)
