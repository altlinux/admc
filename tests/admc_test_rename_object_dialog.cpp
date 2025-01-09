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

#include "admc_test_rename_object_dialog.h"

#include "adldap.h"
#include "rename_dialogs/rename_user_dialog.h"
#include "ui_rename_user_dialog.h"

void ADMCTestRenameObjectDialog::rename() {
    const QString old_name = TEST_USER;
    const QString new_name = old_name + "2";

    const QString old_dn = test_object_dn(old_name, CLASS_USER);
    const QString new_dn = dn_rename(old_dn, new_name);

    // Create test object
    const bool create_success = ad.object_add(old_dn, CLASS_USER);
    QVERIFY(create_success);
    QVERIFY(object_exists(old_dn));

    // Open rename dialog
    auto rename_object_dialog = new RenameUserDialog(ad, old_dn, parent_widget);
    rename_object_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(rename_object_dialog, 1000));

    rename_object_dialog->ui->name_edit->setText(new_name);

    rename_object_dialog->accept();

    QVERIFY(object_exists(new_dn));
    QCOMPARE(rename_object_dialog->get_new_dn(), new_dn);
}

void ADMCTestRenameObjectDialog::rename_user_autofill() {
    const QString old_name = TEST_USER;
    const QString old_dn = test_object_dn(old_name, CLASS_USER);

    const bool create_success = ad.object_add(old_dn, CLASS_USER);
    QVERIFY(create_success);
    QVERIFY(object_exists(old_dn));

    auto rename_object_dialog = new RenameUserDialog(ad, old_dn, parent_widget);
    rename_object_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(rename_object_dialog, 1000));

    test_lineedit_autofill(rename_object_dialog->ui->upn_prefix_edit, rename_object_dialog->ui->sam_name_edit);
}

void ADMCTestRenameObjectDialog::trim() {
    const QString old_name = TEST_USER;
    const QString new_name = QString("%1 new").arg(old_name);
    const QString new_name_untrimmed = QString(" %1 ").arg(new_name);

    const QString old_dn = test_object_dn(old_name, CLASS_USER);
    const QString new_dn = test_object_dn(new_name, CLASS_USER);

    // Create test object
    const bool create_success = ad.object_add(old_dn, CLASS_USER);
    QVERIFY(create_success);
    QVERIFY(object_exists(old_dn));

    // Open rename dialog
    auto rename_object_dialog = new RenameUserDialog(ad, old_dn, parent_widget);
    rename_object_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(rename_object_dialog, 1000));

    rename_object_dialog->ui->name_edit->setText(new_name_untrimmed);

    rename_object_dialog->accept();

    QVERIFY(object_exists(new_dn));
    QCOMPARE(rename_object_dialog->get_new_dn(), new_dn);
}

QTEST_MAIN(ADMCTestRenameObjectDialog)
