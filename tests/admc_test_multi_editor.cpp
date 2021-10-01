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

#include "admc_test_multi_editor.h"

#include "editors/multi_editor.h"
#include "editors/string_editor.h"

#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>

const QList<QByteArray> test_value = {QByteArray("hello")};

void ADMCTestMultiEditor::init() {
    ADMCTest::init();

    edit = new MultiEditor(ATTRIBUTE_DESCRIPTION, parent_widget);
    edit->open();
    QVERIFY(QTest::qWaitForWindowExposed(edit, 1000));

    list_widget = edit->findChild<QListWidget *>();
    QVERIFY(list_widget);

    add_button = edit->findChild<QPushButton *>("add_button");
    QVERIFY(add_button);
    
    remove_button = edit->findChild<QPushButton *>("remove_button");
    QVERIFY(remove_button);
}

void ADMCTestMultiEditor::load_empty() {
    edit->load({});
    QCOMPARE(list_widget->count(), 0);
}

void ADMCTestMultiEditor::load() {
    edit->load(test_value);
    QListWidgetItem *item = list_widget->item(0);
    QVERIFY(item);
    QCOMPARE(item->text(), "hello");
}

void ADMCTestMultiEditor::get_new_values() {
    edit->load(test_value);
    const QList<QByteArray> current = edit->get_new_values();
    QCOMPARE(current, test_value);
}

void ADMCTestMultiEditor::add() {
    edit->load(test_value);

    add_button->click();

    auto string_editor = edit->findChild<StringEditor *>();
    QVERIFY(string_editor);

    auto line_edit = string_editor->findChild<QLineEdit *>();
    QVERIFY(line_edit);

    const QString new_value = "new value";

    line_edit->setText(new_value);

    string_editor->accept();

    QCOMPARE(list_widget->count(), 2);

    auto added_item = list_widget->item(1);
    QVERIFY(added_item);
    QCOMPARE(added_item->text(), new_value);
}

void ADMCTestMultiEditor::remove() {
    edit->load({
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

QTEST_MAIN(ADMCTestMultiEditor)
