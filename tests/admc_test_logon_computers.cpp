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

#include "admc_test_logon_computers.h"

#include "edits/logon_computers_edit.h"
#include "edits/logon_computers_edit_p.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

void ADMCTestLogonComputers::init() {
    ADMCTest::init();

    QList<AttributeEdit *> edits;
    edit = new LogonComputersEdit(&edits, parent_widget);
    auto layout = new QFormLayout();
    parent_widget->setLayout(layout);
    edit->add_to_layout(layout);

    parent_widget->show();
    QVERIFY(QTest::qWaitForWindowExposed(parent_widget, 1000));

    dialog = parent_widget->findChild<LogonComputersDialog *>();
    QVERIFY(dialog != nullptr);

    open_dialog_button = parent_widget->findChild<QPushButton *>("logon_computers_edit_button");

    list = dialog->findChild<QListWidget *>("list");
    QVERIFY(list != nullptr);

    value_edit = dialog->findChild<QLineEdit *>("edit");
    QVERIFY(value_edit != nullptr);

    add_button = dialog->findChild<QPushButton *>("add_button");
    QVERIFY(add_button != nullptr);

    remove_button = dialog->findChild<QPushButton *>("remove_button");
    QVERIFY(remove_button != nullptr);

    const QString name = TEST_USER;
    dn = test_object_dn(name, CLASS_USER);
    const bool create_success = ad.object_add(dn, CLASS_USER);
    QVERIFY(create_success);

    const QString test_value = "test,value";
    ad.attribute_replace_string(dn, ATTRIBUTE_USER_WORKSTATIONS, test_value);

    const AdObject object = ad.search_object(dn);
    edit->load(ad, object);
}

void ADMCTestLogonComputers::load() {
    open_dialog_button->click();
    QVERIFY(QTest::qWaitForWindowExposed(dialog, 1000));

    QVERIFY(list->count() == 2);
    test_list_item(0, "test");
    test_list_item(1, "value");
}

void ADMCTestLogonComputers::emit_edited_signal() {
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

void ADMCTestLogonComputers::add() {
    open_dialog_button->click();
    QVERIFY(QTest::qWaitForWindowExposed(dialog, 1000));

    value_edit->setText("new");

    add_button->click();

    QVERIFY(list->count() == 3);
    test_list_item(0, "test");
    test_list_item(1, "value");
    test_list_item(2, "new");
}

void ADMCTestLogonComputers::remove() {
    open_dialog_button->click();
    QVERIFY(QTest::qWaitForWindowExposed(dialog, 1000));

    list->setCurrentRow(0);

    remove_button->click();

    QVERIFY(list->count() == 1);
    test_list_item(0, "value");
}

void ADMCTestLogonComputers::test_list_item(const int row, const QString &text) {
    auto item = list->item(row);
    QVERIFY(item != nullptr);
    QVERIFY(item->text() == text);
}

void ADMCTestLogonComputers::apply() {
    open_dialog_button->click();
    QVERIFY(QTest::qWaitForWindowExposed(dialog, 1000));

    value_edit->setText("new");

    add_button->click();

    dialog->accept();

    edit->apply(ad, dn);

    const AdObject updated_object = ad.search_object(dn);
    const QString updated_value = updated_object.get_string(ATTRIBUTE_USER_WORKSTATIONS);
    QVERIFY(updated_value == "test,value,new");
}

QTEST_MAIN(ADMCTestLogonComputers)
