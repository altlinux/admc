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

#include "admc_test_list_attribute_dialog.h"

#include "attribute_dialogs/list_attribute_dialog.h"
#include "attribute_dialogs/string_attribute_dialog.h"
#include "attribute_dialogs/ui_list_attribute_dialog.h"
#include "attribute_dialogs/ui_string_attribute_dialog.h"

#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>

void ADMCTestListAttributeDialog::initTestCase_data() {
    QTest::addColumn<QList<QByteArray>>("value_list");
    QTest::addColumn<QString>("display_value");

    const QList<QByteArray> test_value = {
        QByteArray("first"),
        QByteArray("second"),
        QByteArray("third"),
    };

    QTest::newRow("list") << test_value << "first";
}

void ADMCTestListAttributeDialog::init() {
    ADMCTest::init();

    QFETCH_GLOBAL(QList<QByteArray>, value_list);

    dialog = new ListAttributeDialog(value_list, ATTRIBUTE_DESCRIPTION, false, parent_widget);
    dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(dialog, 1000));

    list_widget = dialog->ui->list_widget;
    add_button = dialog->ui->add_button;
    remove_button = dialog->ui->remove_button;
}

void ADMCTestListAttributeDialog::display_value() {
    QFETCH_GLOBAL(QString, display_value);

    QListWidgetItem *item = list_widget->item(0);
    QVERIFY(item);
    QCOMPARE(item->text(), display_value);
}

void ADMCTestListAttributeDialog::get_value_list() {
    QFETCH_GLOBAL(QList<QByteArray>, value_list);

    const QList<QByteArray> actual_value_list = dialog->get_value_list();
    QCOMPARE(actual_value_list, value_list);
}

void ADMCTestListAttributeDialog::add() {
    add_button->click();

    auto string_attribute_dialog = dialog->findChild<StringAttributeDialog *>();
    QVERIFY(string_attribute_dialog);

    QPlainTextEdit *text_edit = string_attribute_dialog->ui->edit;

    const QString new_value = "new value";

    text_edit->setPlainText(new_value);

    string_attribute_dialog->accept();

    QFETCH_GLOBAL(QList<QByteArray>, value_list);
    const int expected_new_size = value_list.size() + 1;
    const int actual_new_size = list_widget->count();

    QCOMPARE(actual_new_size, expected_new_size);

    auto added_item = list_widget->item(list_widget->count() - 1);
    QVERIFY(added_item);
    QCOMPARE(added_item->text(), new_value);
}

void ADMCTestListAttributeDialog::remove() {
    list_widget->setCurrentRow(1);
    remove_button->click();

    QFETCH_GLOBAL(QList<QByteArray>, value_list);
    const int expected_new_size = value_list.size() - 1;
    const int actual_new_size = list_widget->count();

    QCOMPARE(actual_new_size, expected_new_size);
    QCOMPARE(list_widget->item(0)->text(), value_list[0]);
    QCOMPARE(list_widget->item(1)->text(), value_list[2]);
}

QTEST_MAIN(ADMCTestListAttributeDialog)
