/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
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

#include "admc_test_upn_edit.h"

#include "edits/upn_edit.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>

#define TEST_SUFFIX "test.com"

void ADMCTestUpnEdit::init() {
    ADMCTest::init();

    // Embed unlock edit in parent widget
    QList<AttributeEdit *> edits;
    upn_edit = new UpnEdit(&edits, ad, parent_widget);
    auto layout = new QFormLayout();
    parent_widget->setLayout(layout);
    upn_edit->add_to_layout(layout);

    parent_widget->show();
    QVERIFY(QTest::qWaitForWindowExposed(parent_widget, 1000));

    prefix_edit = parent_widget->findChild<QLineEdit *>();
    QVERIFY(prefix_edit != nullptr);

    suffix_edit = parent_widget->findChild<QComboBox *>();
    QVERIFY(suffix_edit != nullptr);

    // Create test user
    const QString name = TEST_USER;
    dn = test_object_dn(name, CLASS_USER);
    const bool create_success = ad.object_add(dn, CLASS_USER);
    QVERIFY(create_success);

    const QString test_upn = QString("%1@%2").arg(name, TEST_SUFFIX);
    ad.attribute_replace_string(dn, ATTRIBUTE_USER_PRINCIPAL_NAME, test_upn);

    const AdObject object = ad.search_object(dn);
    upn_edit->load(ad, object);
}

// Edit should load prefix and suffix into widgets correctly
void ADMCTestUpnEdit::test_load() {
    const QString prefix = prefix_edit->text();
    QVERIFY(prefix == TEST_USER);

    const QString suffix = suffix_edit->currentText();
    QVERIFY(suffix == TEST_SUFFIX);
}

// edited() signal should be emitted when prefix or suffix
// widgets are edited
void ADMCTestUpnEdit::test_emit_edited() {
    bool edited_signal_emitted = false;
    connect(
        upn_edit, &AttributeEdit::edited,
        [&edited_signal_emitted]() {
            edited_signal_emitted = true;
        });

    prefix_edit->setText("test");
    QVERIFY(edited_signal_emitted);

    edited_signal_emitted = false;

    const int suffix_count = suffix_edit->count();
    QVERIFY((suffix_count > 1));
    const int changed_index =
    [this]() {
        if (suffix_edit->currentIndex() == 0) {
            return 1;
        } else {
            return 0;
        }
    }();
    suffix_edit->setCurrentIndex(changed_index);
    QVERIFY(edited_signal_emitted);
}

// Edit should apply changes to suffix
void ADMCTestUpnEdit::test_apply_suffix() {
    change_suffix_in_edit();

    const bool apply_success = upn_edit->apply(ad, dn);
    QVERIFY(apply_success);

    QVERIFY2(edit_state_equals_to_server_state(), "Failed to change upn suffix");
}

// Edit should apply changes to prefix
void ADMCTestUpnEdit::test_apply_prefix() {
    prefix_edit->setText("test-new-prefix");

    const bool apply_success = upn_edit->apply(ad, dn);
    QVERIFY(apply_success);

    QVERIFY2(edit_state_equals_to_server_state(), "Failed to change upn prefix");
}

// Edit should apply changes to prefix
void ADMCTestUpnEdit::test_apply_prefix_and_suffix() {
    change_suffix_in_edit();
    prefix_edit->setText("test-new-prefix2");

    const bool apply_success = upn_edit->apply(ad, dn);
    QVERIFY(apply_success);

    QVERIFY2(edit_state_equals_to_server_state(), "Failed to change upn prefix and suffix");
}

// Edit should reset to server state after load() call
void ADMCTestUpnEdit::test_reset() {
    change_suffix_in_edit();
    prefix_edit->setText("test-new-prefix3");

    const AdObject object = ad.search_object(dn);
    upn_edit->load(ad, object);

    QVERIFY2(edit_state_equals_to_server_state(), "Failed to reset");
}

QString ADMCTestUpnEdit::get_current_upn() {
    const QString prefix = prefix_edit->text();
    const QString suffix = suffix_edit->currentText();
    const QString upn = QString("%1@%2").arg(prefix, suffix);

    return upn;
}

bool ADMCTestUpnEdit::edit_state_equals_to_server_state() {
    const AdObject object = ad.search_object(dn);
    const QString server_upn = object.get_string(ATTRIBUTE_USER_PRINCIPAL_NAME);
    const QString edit_upn = get_current_upn();

    return (edit_upn == server_upn);
}

// Change to next suffix, not equal to current one
void ADMCTestUpnEdit::change_suffix_in_edit() {
    const int new_suffix_index =
    [this]() {
        const QString current_suffix = suffix_edit->currentText();
        
        for (int i = 0; i < suffix_edit->count(); i++) {
            const QString suffix = suffix_edit->itemText(i);

            if (suffix != current_suffix) {
                return i;
            }
        }
        return -1;
    }();
    QVERIFY2((new_suffix_index != -1), "Failed to find different suffix");

    suffix_edit->setCurrentIndex(new_suffix_index);
}

// verify() must return false if there's a user with the
// same upn
void ADMCTestUpnEdit::test_verify() {
    // Create user with conflicting upn
    const QString conflict_name = "conflicting-upn-test-user";
    const QString conflict_dn = test_object_dn(conflict_name, CLASS_USER);
    const bool create_success = ad.object_add(conflict_dn, CLASS_USER);
    QVERIFY(create_success);
    const QString conflicting_upn = get_current_upn();
    ad.attribute_replace_string(conflict_dn, ATTRIBUTE_USER_PRINCIPAL_NAME, conflicting_upn);

    // Verify should fail
    close_message_box_later();
    const bool verify_success = upn_edit->verify(ad, dn);

    QVERIFY2(!verify_success, "verify() didn't notice upn conflict");
}

QTEST_MAIN(ADMCTestUpnEdit)
