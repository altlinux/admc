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

#include "admc_test_bool_attribute_dialog.h"

#include "attribute_dialogs/bool_attribute_dialog.h"
#include "attribute_dialogs/ui_bool_attribute_dialog.h"

#include <QRadioButton>

void ADMCTestBoolAttributeDialog::initTestCase_data() {
    QTest::addColumn<QString>("button_name");
    QTest::addColumn<QList<QByteArray>>("value");

    QTest::newRow("true") << "true_button" << QList<QByteArray>({"TRUE"});
    QTest::newRow("false") << "false_button" << QList<QByteArray>({"FALSE"});
    QTest::newRow("unset") << "unset_button" << QList<QByteArray>();
}

void ADMCTestBoolAttributeDialog::init() {
    ADMCTest::init();

    edit = new BoolAttributeDialog(parent_widget);
    edit->set_attribute(ATTRIBUTE_DESCRIPTION);
    edit->set_value_list(QList<QByteArray>());
    edit->open();
    QVERIFY(QTest::qWaitForWindowExposed(edit, 1000));

    const QHash<QString, QRadioButton *> button_map = {
        {"true_button", edit->ui->true_button},
        {"false_button", edit->ui->false_button},
        {"unset_button", edit->ui->unset_button},
    };

    QFETCH_GLOBAL(QString, button_name);

    button = button_map[button_name];
}

void ADMCTestBoolAttributeDialog::set_value_list() {
    QFETCH_GLOBAL(QList<QByteArray>, value);

    edit->set_value_list(value);
    QVERIFY(button->isChecked());
}

void ADMCTestBoolAttributeDialog::get_value_list() {
    QFETCH_GLOBAL(QList<QByteArray>, value);

    button->click();
    QCOMPARE(edit->get_value_list(), value);
}

QTEST_MAIN(ADMCTestBoolAttributeDialog)
