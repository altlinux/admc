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

#include "admc_test_datetime_edit.h"

#include "attribute_edits/datetime_edit.h"
#include "globals.h"

#include <QDateTimeEdit>

// NOTE: no apply_unmodified() test because all datetime
// attributes are read-only

void ADMCTestDateTimeEdit::init() {
    ADMCTest::init();

    qedit = new QDateTimeEdit(parent_widget);

    edit = new DateTimeEdit(qedit, ATTRIBUTE_WHEN_CHANGED, parent_widget);

    const QString name = TEST_USER;
    dn = test_object_dn(name, CLASS_USER);
    const bool create_success = ad.object_add(dn, CLASS_USER);
    QVERIFY(create_success);
}

void ADMCTestDateTimeEdit::load() {
    const AdObject object = ad.search_object(dn);
    edit->load(ad, object);

    const QDateTime correct_datetime = object.get_datetime(ATTRIBUTE_WHEN_CHANGED, g_adconfig);
    const QDateTime datetime = qedit->dateTime();

    QCOMPARE(datetime, correct_datetime);
}

QTEST_MAIN(ADMCTestDateTimeEdit)
