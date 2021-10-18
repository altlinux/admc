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
#include "editors/ui_bool_editor.h"

#include <QRadioButton>

void ADMCTestBoolEditor::initTestCase_data() {
    QTest::addColumn<QString>("button_name");
    QTest::addColumn<QList<QByteArray>>("value");

    QTest::newRow("true") << "true_button" << QList<QByteArray>({"TRUE"});
    QTest::newRow("false") << "false_button" << QList<QByteArray>({"FALSE"});
    QTest::newRow("unset") << "unset_button" << QList<QByteArray>();
}

void ADMCTestBoolEditor::init() {
    ADMCTest::init();

    edit = new BoolEditor(ATTRIBUTE_DESCRIPTION, parent_widget);
    edit->load(QList<QByteArray>());
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

void ADMCTestBoolEditor::load() {
    QFETCH_GLOBAL(QList<QByteArray>, value);

    edit->load(value);
    QVERIFY(button->isChecked());
}

void ADMCTestBoolEditor::get_new_values() {
    QFETCH_GLOBAL(QList<QByteArray>, value);

    button->click();
    QCOMPARE(edit->get_new_values(), value);
}

QTEST_MAIN(ADMCTestBoolEditor)
