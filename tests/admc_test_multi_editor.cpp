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

#include "admc_test_list_attribute_dialog.h"

#include "attribute_dialogs/list_attribute_dialog.h"
#include "attribute_dialogs/string_attribute_dialog.h"
#include "attribute_dialogs/ui_list_attribute_dialog.h"
#include "attribute_dialogs/ui_string_attribute_dialog.h"

#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

const QList<QByteArray> test_value = {QByteArray("hello")};

void ADMCTestListAttributeDialog::init() {
    ADMCTest::init();

    edit = new ListAttributeDialog(parent_widget);
    edit->set_attribute(ATTRIBUTE_DESCRIPTION);
    edit->open();
    QVERIFY(QTest::qWaitForWindowExposed(edit, 1000));

    list_widget = edit->ui->list_widget;
    add_button = edit->ui->add_button;
    remove_button = edit->ui->remove_button;
}

void ADMCTestListAttributeDialog::set_value_list_empty() {
    edit->set_value_list({});
    QCOMPARE(list_widget->count(), 0);
}

void ADMCTestListAttributeDialog::set_value_list() {
    edit->set_value_list(test_value);
    QListWidgetItem *item = list_widget->item(0);
    QVERIFY(item);
    QCOMPARE(item->text(), "hello");
}

void ADMCTestListAttributeDialog::get_value_list() {
    edit->set_value_list(test_value);
    const QList<QByteArray> current = edit->get_value_list();
    QCOMPARE(current, test_value);
}

void ADMCTestListAttributeDialog::add() {
    edit->set_value_list(test_value);

    add_button->click();

    auto string_attribute_dialog = edit->findChild<StringAttributeDialog *>();
    QVERIFY(string_attribute_dialog);

    QLineEdit *line_edit = string_attribute_dialog->ui->edit;

    const QString new_value = "new value";

    line_edit->setText(new_value);

    string_attribute_dialog->accept();

    QCOMPARE(list_widget->count(), 2);

    auto added_item = list_widget->item(1);
    QVERIFY(added_item);
    QCOMPARE(added_item->text(), new_value);
}

void ADMCTestListAttributeDialog::remove() {
    edit->set_value_list({
        QByteArray("first"),
        QByteArray("second"),
        QByteArray("third"),
    });

    list_widget->setCurrentRow(1);
    remove_button->click();

    QCOMPARE(list_widget->count(), 2);
    QCOMPARE(list_widget->item(0)->text(), "first");
    QCOMPARE(list_widget->item(1)->text(), "third");
}

QTEST_MAIN(ADMCTestListAttributeDialog)
