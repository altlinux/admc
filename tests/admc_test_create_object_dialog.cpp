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

#include "admc_test_create_object_dialog.h"

#include "adldap.h"
#include "samba/dom_sid.h"
#include "create_computer_dialog.h"
#include "create_group_dialog.h"
#include "create_ou_dialog.h"
#include "create_user_dialog.h"
#include "ui_create_computer_dialog.h"
#include "ui_create_group_dialog.h"
#include "ui_create_ou_dialog.h"
#include "ui_create_user_dialog.h"

void ADMCTestCreateObjectDialog::create_user() {
    const QString name = TEST_USER;
    const QString logon_name = TEST_USER_LOGON;
    const QString password = TEST_PASSWORD;
    const QString parent = test_arena_dn();
    const QString dn = test_object_dn(name, CLASS_USER);

    // Create user
    auto create_dialog = new CreateUserDialog(ad, parent, parent_widget);
    create_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(create_dialog, 1000));

    create_dialog->ui->name_edit->setText(name);
    create_dialog->ui->sam_name_edit->setText(logon_name);
    create_dialog->ui->password_main_edit->setText(password);
    create_dialog->ui->password_confirm_edit->setText(password);

    create_dialog->accept();

    QVERIFY2(object_exists(dn), "Created user doesn't exist");
    QCOMPARE(create_dialog->get_created_dn(), dn);

    const int actual_uac = [&]() {
        const AdObject object = ad.search_object(dn, {ATTRIBUTE_USER_ACCOUNT_CONTROL});
        const int out = object.get_int(ATTRIBUTE_USER_ACCOUNT_CONTROL);

        return out;
    }();
    const int expected_uac = UAC_NORMAL_ACCOUNT;
    QCOMPARE(actual_uac, expected_uac);
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

QTEST_MAIN(ADMCTestCreateObjectDialog)
