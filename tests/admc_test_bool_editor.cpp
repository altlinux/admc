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

#include "admc_test_bool_editor.h"

#include "editors/bool_editor.h"

#include <QRadioButton>

const QList<QByteArray> empty_value = {};
const QList<QByteArray> true_value = {"TRUE"};
const QList<QByteArray> false_value = {"FALSE"};

void ADMCTestBoolEdit::init() {
    ADMCTest::init();

    edit = new BoolEditor(ATTRIBUTE_DESCRIPTION, parent_widget);
    edit->open();
    QVERIFY(QTest::qWaitForWindowExposed(edit, 1000));

    true_button = edit->findChild<QRadioButton *>("true_button");
    QVERIFY(true_button);

    false_button = edit->findChild<QRadioButton *>("false_button");
    QVERIFY(false_button);

    unset_button = edit->findChild<QRadioButton *>("unset_button");
    QVERIFY(unset_button);
}

void ADMCTestBoolEdit::load() {
    edit->load(empty_value);
    QVERIFY(unset_button->isChecked());

    edit->load(true_value);
    QVERIFY(true_button->isChecked());

    edit->load(false_value);
    QVERIFY(false_button->isChecked());
}

void ADMCTestBoolEdit::get_new_values() {
    edit->load(empty_value);

    true_button->click();
    const QList<QByteArray> true_value_from_get = edit->get_new_values();
    QVERIFY(true_value_from_get == true_value);

    false_button->click();
    const QList<QByteArray> false_value_from_get = edit->get_new_values();
    QVERIFY(false_value_from_get == false_value);

    unset_button->click();
    const QList<QByteArray> empty_value_from_get = edit->get_new_values();
    QVERIFY(empty_value_from_get == empty_value);
}

QTEST_MAIN(ADMCTestBoolEdit)
