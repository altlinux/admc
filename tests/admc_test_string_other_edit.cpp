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

#include "admc_test_string_other_edit.h"

#include "attribute_dialogs/list_attribute_dialog.h"
#include "attribute_dialogs/string_attribute_dialog.h"
#include "attribute_dialogs/ui_list_attribute_dialog.h"
#include "attribute_edits/string_other_edit.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>

#define TEST_ATTRIBUTE_MAIN ATTRIBUTE_WWW_HOMEPAGE
#define TEST_ATTRIBUTE_OTHER ATTRIBUTE_WWW_HOMEPAGE_OTHER

const QString main_value = "main_value";
const QList<QByteArray> other_value_list = {
    "first",
    "second",
    "third",
};

#define new_value "new_value"

void ADMCTestStringOtherEdit::init() {
    ADMCTest::init();

    line_edit = new QLineEdit(parent_widget);
    other_button = new QPushButton(parent_widget);

    edit = new StringOtherEdit(line_edit, other_button, TEST_ATTRIBUTE_MAIN, ATTRIBUTE_WWW_HOMEPAGE_OTHER, parent_widget);

    // Create test user
    const QString name = TEST_USER;
    dn = test_object_dn(name, CLASS_USER);
    const bool create_success = ad.object_add(dn, CLASS_USER);
    QVERIFY(create_success);

    // Set values
    ad.attribute_replace_string(dn, TEST_ATTRIBUTE_MAIN, main_value);
    ad.attribute_replace_values(dn, TEST_ATTRIBUTE_OTHER, other_value_list);

    // Load user into edit
    const AdObject object = ad.search_object(dn);
    edit->load(ad, object);
}

// edited() signal should be emitted when lineedit is edited
// and when other values are added
void ADMCTestStringOtherEdit::test_emit_edited_signal() {
    bool edited_signal_emitted = false;
    connect(
        edit, &AttributeEdit::edited,
        this,
        [&edited_signal_emitted]() {
            edited_signal_emitted = true;
        });

    line_edit->setText("test");
    QVERIFY(edited_signal_emitted);

    edited_signal_emitted = false;

    add_new_other_value();

    QVERIFY(edited_signal_emitted);
}

// Edit should contain current attribute value after load()
// call
void ADMCTestStringOtherEdit::load() {
    QCOMPARE(line_edit->text(), main_value);

    other_button->click();

    auto list_attribute_dialog = parent_widget->findChild<ListAttributeDialog *>();
    QVERIFY(list_attribute_dialog);

    auto list_widget = list_attribute_dialog->findChild<QListWidget *>();
    QVERIFY(list_widget);

    QCOMPARE(line_edit->text(), main_value);
    for (int i = 0; i < 3; i++) {
        QCOMPARE(list_widget->item(i)->text(), other_value_list[i]);
    }
}

void ADMCTestStringOtherEdit::apply_unmodified() {
    test_edit_apply_unmodified(edit, dn);
}

void ADMCTestStringOtherEdit::apply_modified_main_value() {
    line_edit->setText(new_value);

    const bool apply_success = edit->apply(ad, dn);
    QVERIFY(apply_success);

    const AdObject object = ad.search_object(dn);
    const QString current_value_main = object.get_string(TEST_ATTRIBUTE_MAIN);

    QCOMPARE(current_value_main, new_value);

    const QList<QByteArray> current_value_other = object.get_values(TEST_ATTRIBUTE_OTHER);

    QCOMPARE(current_value_other, other_value_list);
}

void ADMCTestStringOtherEdit::apply_modified_other_value() {
    add_new_other_value();

    const bool apply_success = edit->apply(ad, dn);
    QVERIFY(apply_success);

    const AdObject object = ad.search_object(dn);
    const QString current_value_main = object.get_string(TEST_ATTRIBUTE_MAIN);

    QCOMPARE(current_value_main, main_value);

    const QList<QByteArray> current_value_other = object.get_values(TEST_ATTRIBUTE_OTHER);
    QList<QByteArray> correct_value_other = other_value_list;
    correct_value_other.append(new_value);

    QCOMPARE(current_value_other, correct_value_other);
}

void ADMCTestStringOtherEdit::add_new_other_value() {
    other_button->click();

    auto list_attribute_dialog = parent_widget->findChild<ListAttributeDialog *>();
    QVERIFY(list_attribute_dialog);

    QPushButton *add_button = list_attribute_dialog->ui->add_button;
    add_button->click();

    auto string_attribute_dialog = list_attribute_dialog->findChild<StringAttributeDialog *>();
    QVERIFY(string_attribute_dialog);

    auto string_attribute_dialog_line_edit = string_attribute_dialog->findChild<QPlainTextEdit *>();
    QVERIFY(string_attribute_dialog_line_edit);

    string_attribute_dialog_line_edit->setPlainText(new_value);

    string_attribute_dialog->accept();

    list_attribute_dialog->accept();
}

QTEST_MAIN(ADMCTestStringOtherEdit)
