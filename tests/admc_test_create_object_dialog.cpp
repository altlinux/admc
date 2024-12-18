/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2022 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
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

#include "admc_test_create_object_dialog.h"

#include "adldap.h"
#include "create_dialogs/create_computer_dialog.h"
#include "create_dialogs/create_contact_dialog.h"
#include "create_dialogs/create_group_dialog.h"
#include "create_dialogs/create_ou_dialog.h"
#include "create_dialogs/create_shared_folder_dialog.h"
#include "create_dialogs/create_user_dialog.h"
#include "samba/dom_sid.h"
#include "settings.h"
#include "ui_create_computer_dialog.h"
#include "ui_create_contact_dialog.h"
#include "ui_create_group_dialog.h"
#include "ui_create_ou_dialog.h"
#include "ui_create_shared_folder_dialog.h"
#include "ui_create_user_dialog.h"

void test_lineedit_autofill(QLineEdit *src_edit, QLineEdit *dest_edit);
void test_full_name_autofill(QLineEdit *first_name_edit, QLineEdit *middle_name_edit, QLineEdit *last_name_edit, QLineEdit *full_name_edit);

void ADMCTestCreateObjectDialog::create_user_data() {
    QTest::addColumn<QString>("user_class");
    QTest::addColumn<QString>("password");
    QTest::addColumn<bool>("expected_object_exists");
    QTest::addColumn<bool>("expected_message_box_is_open");

    QTest::newRow("user") << CLASS_USER << TEST_PASSWORD << true << false;
    QTest::newRow("inetOrgPerson") << CLASS_INET_ORG_PERSON << TEST_PASSWORD << true << false;
    QTest::newRow("user empty password") << CLASS_USER << QString() << false << true;
}

void ADMCTestCreateObjectDialog::create_user() {
    QFETCH(QString, user_class);
    QFETCH(QString, password);
    QFETCH(bool, expected_object_exists);
    QFETCH(bool, expected_message_box_is_open);

    const QString name = TEST_USER;
    const QString logon_name = TEST_USER_LOGON;
    const QString parent = test_arena_dn();
    const QString dn = test_object_dn(name, CLASS_USER);

    // Create user
    auto create_dialog = new CreateUserDialog(ad, parent, user_class, parent_widget);
    create_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(create_dialog, 1000));

    create_dialog->ui->name_edit->setText(name);
    create_dialog->ui->sam_name_edit->setText(logon_name);
    create_dialog->ui->password_main_edit->setText(password);
    create_dialog->ui->password_confirm_edit->setText(password);

    create_dialog->accept();

    const bool actual_message_box_is_open = message_box_is_open();
    QCOMPARE(actual_message_box_is_open, expected_message_box_is_open);

    const bool actual_object_exists = object_exists(dn);
    QCOMPARE(actual_object_exists, expected_object_exists);

    if (actual_object_exists) {
        QCOMPARE(create_dialog->get_created_dn(), dn);

        const int actual_uac = [&]() {
            const AdObject object = ad.search_object(dn, {ATTRIBUTE_USER_ACCOUNT_CONTROL});
            const int out = object.get_int(ATTRIBUTE_USER_ACCOUNT_CONTROL);

            return out;
        }();
        const int expected_uac = UAC_NORMAL_ACCOUNT;
        QCOMPARE(actual_uac, expected_uac);
    }
}

void ADMCTestCreateObjectDialog::create_user_autofill() {
    const QString object_class = CLASS_USER;
    const QString name = TEST_OBJECT;
    const QString parent = test_arena_dn();
    const QString dn = test_object_dn(name, object_class);

    auto create_dialog = new CreateUserDialog(ad, parent, object_class, parent_widget);
    create_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(create_dialog, 1000));

    test_lineedit_autofill(create_dialog->ui->upn_prefix_edit, create_dialog->ui->sam_name_edit);

    test_full_name_autofill(create_dialog->ui->first_name_edit, create_dialog->ui->middle_name_edit, create_dialog->ui->last_name_edit, create_dialog->ui->name_edit);
}

void ADMCTestCreateObjectDialog::create_ou() {
    const QString name = TEST_OU;
    const QString parent = test_arena_dn();
    const QString dn = test_object_dn(name, CLASS_OU);

    // Create ou
    auto create_dialog = new CreateOUDialog(parent, parent_widget);
    create_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(create_dialog, 1000));

    create_dialog->ui->name_edit->setText(name);

    create_dialog->accept();

    QVERIFY2(object_exists(dn), "Created OU doesn't exist");
    QCOMPARE(create_dialog->get_created_dn(), dn);
}

void ADMCTestCreateObjectDialog::create_computer() {
    const QString object_class = CLASS_COMPUTER;
    const QString name = TEST_COMPUTER;
    const QString parent = test_arena_dn();
    const QString dn = test_object_dn(name, object_class);

    // Open create dialog
    auto create_dialog = new CreateComputerDialog(parent, parent_widget);
    create_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(create_dialog, 1000));

    create_dialog->ui->name_edit->setText(name);
    create_dialog->ui->sam_name_edit->setText(name);

    create_dialog->accept();

    const QString actual_dn = create_dialog->get_created_dn();

    QVERIFY2(object_exists(dn), "Created computer doesn't exist");
    QCOMPARE(actual_dn, dn);

    const AdObject object = ad.search_object(actual_dn);

    const int actual_sam_type = object.get_int(ATTRIBUTE_SAM_ACCOUNT_TYPE);
    const int expected_sam_type = SAM_MACHINE_ACCOUNT;
    QCOMPARE(actual_sam_type, expected_sam_type);

    const int actual_primary_group_rid = object.get_int(ATTRIBUTE_PRIMARY_GROUP_ID);
    const int expected_primary_group_rid = DOMAIN_RID_DOMAIN_MEMBERS;
    QCOMPARE(actual_primary_group_rid, expected_primary_group_rid);

    const int actual_uac = object.get_int(ATTRIBUTE_USER_ACCOUNT_CONTROL);
    const int expected_uac = (UAC_PASSWD_NOTREQD | UAC_WORKSTATION_TRUST_ACCOUNT);
    QCOMPARE(actual_uac, expected_uac);

    const QString actual_sam_name = object.get_string(ATTRIBUTE_SAM_ACCOUNT_NAME);
    const QString expected_sam_name = QString("%1$").arg(name);
    QCOMPARE(actual_sam_name, expected_sam_name);
}

void ADMCTestCreateObjectDialog::create_computer_autofill() {
    const QString object_class = CLASS_COMPUTER;
    const QString name = TEST_COMPUTER;
    const QString parent = test_arena_dn();

    auto create_dialog = new CreateComputerDialog(parent, parent_widget);
    create_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(create_dialog, 1000));

    create_dialog->ui->name_edit->setText(name);

    const QString actual_sam_name = create_dialog->ui->sam_name_edit->text();
    const QString expected_sam_name = name.toUpper();
    QCOMPARE(actual_sam_name, expected_sam_name);
}

void ADMCTestCreateObjectDialog::create_group() {
    const QString object_class = CLASS_GROUP;
    const QString name = TEST_GROUP;
    const QString parent = test_arena_dn();
    const QString dn = test_object_dn(name, object_class);

    // Open create dialog
    auto create_dialog = new CreateGroupDialog(parent, parent_widget);
    create_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(create_dialog, 1000));

    create_dialog->ui->name_edit->setText(name);
    create_dialog->ui->sam_name_edit->setText(name);

    create_dialog->accept();

    QVERIFY2(object_exists(dn), "Created group doesn't exist");
    QCOMPARE(create_dialog->get_created_dn(), dn);
}

void ADMCTestCreateObjectDialog::create_group_autofill() {
    // NOTE: use shorter name because of sam name limits
    const QString name = TEST_OBJECT;
    const QString parent = test_arena_dn();

    auto create_dialog = new CreateGroupDialog(parent, parent_widget);
    create_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(create_dialog, 1000));

    create_dialog->ui->name_edit->setText(name);

    const QString actual_sam_name = create_dialog->ui->sam_name_edit->text();
    const QString expected_sam_name = name;
    QCOMPARE(actual_sam_name, expected_sam_name);
}

void ADMCTestCreateObjectDialog::create_shared_folder() {
    const QString object_class = CLASS_SHARED_FOLDER;
    const QString name = TEST_OBJECT;
    const QString parent = test_arena_dn();
    const QString dn = test_object_dn(name, object_class);

    // Open create dialog
    auto create_dialog = new CreateSharedFolderDialog(parent, parent_widget);
    create_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(create_dialog, 1000));

    create_dialog->ui->name_edit->setText(name);
    create_dialog->ui->path_edit->setText("path");

    create_dialog->accept();

    QVERIFY2(object_exists(dn), "Created shared folder doesn't exist");
    QCOMPARE(create_dialog->get_created_dn(), dn);
}

void ADMCTestCreateObjectDialog::create_contact() {
    const QString object_class = CLASS_CONTACT;
    const QString name = TEST_OBJECT;
    const QString parent = test_arena_dn();
    const QString dn = test_object_dn(name, object_class);

    // Open create dialog
    auto create_dialog = new CreateContactDialog(parent, parent_widget);
    create_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(create_dialog, 1000));

    create_dialog->ui->first_name_edit->setText("first");
    create_dialog->ui->last_name_edit->setText("last");
    create_dialog->ui->full_name_edit->setText(name);
    create_dialog->ui->display_name_edit->setText("display");

    create_dialog->accept();

    QVERIFY2(object_exists(dn), "Created contact doesn't exist");
    QCOMPARE(create_dialog->get_created_dn(), dn);
}

void ADMCTestCreateObjectDialog::create_contact_autofill() {
    const QString object_class = CLASS_CONTACT;
    const QString name = TEST_OBJECT;
    const QString parent = test_arena_dn();
    const QString dn = test_object_dn(name, object_class);

    auto create_dialog = new CreateContactDialog(parent, parent_widget);
    create_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(create_dialog, 1000));

    test_full_name_autofill(create_dialog->ui->first_name_edit, create_dialog->ui->middle_name_edit, create_dialog->ui->last_name_edit, create_dialog->ui->full_name_edit);
}

void ADMCTestCreateObjectDialog::trim() {
    const QString name = TEST_OU;
    const QString name_untrimmed = QString(" %1 ").arg(name);
    const QString parent = test_arena_dn();
    const QString dn = test_object_dn(name, CLASS_OU);

    auto create_dialog = new CreateOUDialog(parent, parent_widget);
    create_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(create_dialog, 1000));

    create_dialog->ui->name_edit->setText(name_untrimmed);

    create_dialog->accept();

    QVERIFY(object_exists(dn));
    QCOMPARE(create_dialog->get_created_dn(), dn);
}

void test_full_name_autofill(QLineEdit *first_name_edit, QLineEdit *middle_name_edit, QLineEdit *last_name_edit, QLineEdit *full_name_edit) {
    const QString first_name = "first";
    const QString last_name = "last";
    const QString middle_name = "middle";

    first_name_edit->setText(first_name);
    last_name_edit->setText(last_name);
    middle_name_edit->setText(middle_name);

    const QString actual_full_name = full_name_edit->text();
    const QString expected_full_name = [&]() {
        const bool last_name_first = settings_get_variant(SETTING_last_name_before_first_name).toBool();

        if (last_name_first) {
            return QString("%1 %2 %3").arg(last_name, first_name, middle_name);
        } else {
            return QString("%1 %2 %3").arg(first_name, middle_name, last_name);
        }
    }();
    QCOMPARE(actual_full_name, expected_full_name);
}

QTEST_MAIN(ADMCTestCreateObjectDialog)
