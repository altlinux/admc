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

#include "admc_test_dn_edit.h"

#include "attribute_edits/dn_edit.h"

#include <QLineEdit>

void ADMCTestDNEdit::init() {
    ADMCTest::init();

    line_edit = new QLineEdit(parent_widget);

    edit = new DNEdit(line_edit, parent_widget);

    // Create test user
    const QString name = TEST_USER;
    dn = test_object_dn(name, CLASS_USER);
    const bool create_success = ad.object_add(dn, CLASS_USER);
    QVERIFY(create_success);
}

void ADMCTestDNEdit::load() {
    const AdObject object = ad.search_object(dn);
    edit->load(ad, object);

    const QString actual_value = line_edit->text();
    const QString expected_value = dn_canonical(dn);
    QCOMPARE(actual_value, expected_value);
}

QTEST_MAIN(ADMCTestDNEdit)
