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

#include "admc_test_bool_attribute_dialog.h"

#include "attribute_dialogs/bool_attribute_dialog.h"
#include "attribute_dialogs/ui_bool_attribute_dialog.h"

#include <QRadioButton>

void ADMCTestBoolAttributeDialog::initTestCase_data() {
    QTest::addColumn<QString>("button_name");
    QTest::addColumn<QList<QByteArray>>("value_list");

    QTest::newRow("true") << "true_button" << QList<QByteArray>({"TRUE"});
    QTest::newRow("false") << "false_button" << QList<QByteArray>({"FALSE"});
    QTest::newRow("unset") << "unset_button" << QList<QByteArray>();
}

void ADMCTestBoolAttributeDialog::init() {
    ADMCTest::init();

    QFETCH_GLOBAL(QList<QByteArray>, value_list);

    dialog = new BoolAttributeDialog(value_list, ATTRIBUTE_DESCRIPTION, false, parent_widget);
    dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(dialog, 1000));
}

void ADMCTestBoolAttributeDialog::display_value() {
    const QHash<QString, QRadioButton *> button_map = {
        {"true_button", dialog->ui->true_button},
        {"false_button", dialog->ui->false_button},
        {"unset_button", dialog->ui->unset_button},
    };

    QFETCH_GLOBAL(QString, button_name);

    button = button_map[button_name];

    QVERIFY(button->isChecked());
}

void ADMCTestBoolAttributeDialog::get_value_list() {
    QFETCH_GLOBAL(QList<QByteArray>, value_list);

    const QList<QByteArray> actual_value_list = dialog->get_value_list();
    QCOMPARE(actual_value_list, value_list);
}

QTEST_MAIN(ADMCTestBoolAttributeDialog)
