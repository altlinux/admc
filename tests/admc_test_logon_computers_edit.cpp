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

#include "admc_test_logon_computers_edit.h"

#include "edits/logon_computers_edit.h"
#include "edits/logon_computers_edit_p.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

void ADMCTestLogonComputersEdit::init() {
    ADMCTest::init();

    auto button = new QPushButton(parent_widget);

    edit = new LogonComputersEdit(button, &edits, parent_widget);
    add_attribute_edit(edit);

    dialog = parent_widget->findChild<LogonComputersDialog *>();
    QVERIFY(dialog);

    open_dialog_button = parent_widget->findChild<QPushButton *>("logon_computers_edit_button");

    list = dialog->findChild<QListWidget *>("list");
    QVERIFY(list);

    value_edit = dialog->findChild<QLineEdit *>("edit");
    QVERIFY(value_edit);

    add_button = dialog->findChild<QPushButton *>("add_button");
    QVERIFY(add_button);

    remove_button = dialog->findChild<QPushButton *>("remove_button");
    QVERIFY(remove_button);

    const QString name = TEST_USER;
    dn = test_object_dn(name, CLASS_USER);
    const bool create_success = ad.object_add(dn, CLASS_USER);
    QVERIFY(create_success);

    const QString test_value = "test,value";
    ad.attribute_replace_string(dn, ATTRIBUTE_USER_WORKSTATIONS, test_value);

    const AdObject object = ad.search_object(dn);
    edit->load(ad, object);
}

void ADMCTestLogonComputersEdit::load() {
    open_dialog_button->click();
    QVERIFY(QTest::qWaitForWindowExposed(dialog, 1000));

    QCOMPARE(list->count(), 2);
    test_list_item(0, "test");
    test_list_item(1, "value");
}

void ADMCTestLogonComputersEdit::emit_edited_signal() {
    bool edited_signal_emitted = false;
    connect(
        edit, &AttributeEdit::edited,
        [&edited_signal_emitted]() {
            edited_signal_emitted = true;
        });

    open_dialog_button->click();
    QVERIFY(QTest::qWaitForWindowExposed(dialog, 1000));

    dialog->accept();

    QVERIFY(edited_signal_emitted);
}

void ADMCTestLogonComputersEdit::add() {
    open_dialog_button->click();
    QVERIFY(QTest::qWaitForWindowExposed(dialog, 1000));

    value_edit->setText("new");

    add_button->click();

    QCOMPARE(list->count(), 3);
    test_list_item(0, "test");
    test_list_item(1, "value");
    test_list_item(2, "new");
}

void ADMCTestLogonComputersEdit::remove() {
    open_dialog_button->click();
    QVERIFY(QTest::qWaitForWindowExposed(dialog, 1000));

    list->setCurrentRow(0);

    remove_button->click();

    QCOMPARE(list->count(), 1);
    test_list_item(0, "value");
}

void ADMCTestLogonComputersEdit::test_list_item(const int row, const QString &text) {
    auto item = list->item(row);
    QVERIFY(item);
    QCOMPARE(item->text(), text);
}

void ADMCTestLogonComputersEdit::apply_unmodified() {
    test_edit_apply_unmodified(edit, dn);
}

void ADMCTestLogonComputersEdit::apply() {
    open_dialog_button->click();
    QVERIFY(QTest::qWaitForWindowExposed(dialog, 1000));

    value_edit->setText("new");

    add_button->click();

    dialog->accept();

    edit->apply(ad, dn);

    const AdObject updated_object = ad.search_object(dn);
    const QString updated_value = updated_object.get_string(ATTRIBUTE_USER_WORKSTATIONS);
    QCOMPARE(updated_value, "test,value,new");
}

QTEST_MAIN(ADMCTestLogonComputersEdit)
