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
    auto create_dialog = new CreateUserDialog(parent_widget);
    create_dialog->set_parent_dn(parent);
    create_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(create_dialog, 1000));

    create_dialog->ui->name_edit->setText(name);
    create_dialog->ui->sam_name_edit->setText(logon_name);
    create_dialog->ui->password_main_edit->setText(password);
    create_dialog->ui->password_confirm_edit->setText(password);

    create_dialog->accept();

    QVERIFY2(object_exists(dn), "Created user doesn't exist");
    QCOMPARE(create_dialog->get_created_name(), name);
    QCOMPARE(create_dialog->get_created_dn(), dn);
}

void ADMCTestCreateObjectDialog::create_ou() {
    const QString name = TEST_OU;
    const QString parent = test_arena_dn();
    const QString dn = test_object_dn(name, CLASS_OU);

    // Create ou
    auto create_dialog = new CreateOUDialog(parent_widget);
    create_dialog->set_parent_dn(parent);
    create_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(create_dialog, 1000));

    create_dialog->ui->name_edit->setText(name);

    create_dialog->accept();

    QVERIFY2(object_exists(dn), "Created OU doesn't exist");
    QCOMPARE(create_dialog->get_created_name(), name);
    QCOMPARE(create_dialog->get_created_dn(), dn);
}

void ADMCTestCreateObjectDialog::create_computer() {
    const QString object_class = CLASS_COMPUTER;
    const QString name = TEST_COMPUTER;
    const QString parent = test_arena_dn();
    const QString dn = test_object_dn(name, object_class);

    // Open create dialog
    auto create_dialog = new CreateComputerDialog(parent_widget);
    create_dialog->set_parent_dn(parent);
    create_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(create_dialog, 1000));

    create_dialog->ui->name_edit->setText(name);
    create_dialog->ui->sam_name_edit->setText(name);

    create_dialog->accept();

    QVERIFY2(object_exists(dn), "Created computer doesn't exist");
    QCOMPARE(create_dialog->get_created_name(), name);
    QCOMPARE(create_dialog->get_created_dn(), dn);
}

void ADMCTestCreateObjectDialog::create_group() {
    const QString object_class = CLASS_GROUP;
    const QString name = TEST_GROUP;
    const QString parent = test_arena_dn();
    const QString dn = test_object_dn(name, object_class);

    // Open create dialog
    auto create_dialog = new CreateGroupDialog(parent_widget);
    create_dialog->set_parent_dn(parent);
    create_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(create_dialog, 1000));

    create_dialog->ui->name_edit->setText(name);
    create_dialog->ui->sam_name_edit->setText(name);

    create_dialog->accept();

    QVERIFY2(object_exists(dn), "Created group doesn't exist");
    QCOMPARE(create_dialog->get_created_name(), name);
    QCOMPARE(create_dialog->get_created_dn(), dn);
}

QTEST_MAIN(ADMCTestCreateObjectDialog)
