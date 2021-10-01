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

#include "admc_test_object_menu.h"

#include "adldap.h"
#include "console_impls/object_impl.h"
#include "create_object_dialog.h"
#include "filter_widget/filter_widget_advanced_tab.h"
#include "filter_widget/filter_widget_simple_tab.h"
#include "find_object_dialog.h"
#include "find_widget.h"
#include "rename_user_dialog.h"
#include "select_container_dialog.h"
#include "select_object_advanced_dialog.h"
#include "select_object_dialog.h"
#include "utils.h"

#include <QComboBox>
#include <QDebug>
#include <QLineEdit>
#include <QModelIndex>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QTest>
#include <QTreeView>

void ADMCTestObjectMenu::object_menu_new_user() {
    const QString name = TEST_USER;
    const QString logon_name = TEST_USER_LOGON;
    const QString password = TEST_PASSWORD;
    const QString parent = test_arena_dn();
    const QString dn = test_object_dn(name, CLASS_USER);

    // Create user
    auto create_dialog = new CreateObjectDialog({parent}, CLASS_USER, parent_widget);
    create_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(create_dialog, 1000));

    // Fill out edits
    auto name_edit = create_dialog->findChild<QLineEdit *>("name_edit");
    QVERIFY(name_edit != nullptr);
    name_edit->setText(name);

    auto sama_edit = create_dialog->findChild<QLineEdit *>("sama_edit");
    QVERIFY(sama_edit != nullptr);
    sama_edit->setText(logon_name);

    auto password_main_edit = create_dialog->findChild<QLineEdit *>("password_main_edit");
    QVERIFY(password_main_edit != nullptr);
    password_main_edit->setText(password);

    auto password_confirm_edit = create_dialog->findChild<QLineEdit *>("password_confirm_edit");
    QVERIFY(password_confirm_edit != nullptr);
    password_confirm_edit->setText(password);

    create_dialog->accept();

    QVERIFY2(object_exists(dn), "Created user doesn't exist");

    QVERIFY(true);
}

void ADMCTestObjectMenu::object_menu_new_ou() {
    const QString name = TEST_OU;
    const QString parent = test_arena_dn();
    const QString dn = test_object_dn(name, CLASS_OU);

    // Create ou
    auto create_dialog = new CreateObjectDialog({parent}, CLASS_OU, parent_widget);
    create_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(create_dialog, 1000));

    // Enter name
    auto name_edit = create_dialog->findChild<QLineEdit *>("name_edit");
    QVERIFY(name_edit != nullptr);
    name_edit->setText(name);

    create_dialog->accept();

    QVERIFY2(object_exists(dn), "Created OU doesn't exist");

    QVERIFY(true);
}

void ADMCTestObjectMenu::object_menu_new_computer() {
    const QString object_class = CLASS_COMPUTER;
    const QString name = TEST_COMPUTER;
    const QString parent = test_arena_dn();
    const QString dn = test_object_dn(name, object_class);

    // Open create dialog
    auto create_dialog = new CreateObjectDialog({parent}, CLASS_COMPUTER, parent_widget);
    create_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(create_dialog, 1000));

    // Enter name
    auto name_edit = create_dialog->findChild<QLineEdit *>("name_edit");
    QVERIFY(name_edit != nullptr);
    name_edit->setText(name);

    // Enter logon name
    auto sama_edit = create_dialog->findChild<QLineEdit *>("sama_edit");
    QVERIFY(sama_edit != nullptr);
    sama_edit->setText(name);

    create_dialog->accept();

    QVERIFY2(object_exists(dn), "Created computer doesn't exist");

    QVERIFY(true);
}

void ADMCTestObjectMenu::object_menu_new_group() {
    const QString object_class = CLASS_GROUP;
    const QString name = TEST_GROUP;
    const QString parent = test_arena_dn();
    const QString dn = test_object_dn(name, object_class);

    // Open create dialog
    auto create_dialog = new CreateObjectDialog({parent}, CLASS_GROUP, parent_widget);
    create_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(create_dialog, 1000));

    // Enter name
    auto name_edit = create_dialog->findChild<QLineEdit *>("name_edit");
    QVERIFY(name_edit != nullptr);
    name_edit->setText(name);

    // Enter logon name
    auto sama_edit = create_dialog->findChild<QLineEdit *>("sama_edit");
    QVERIFY(sama_edit != nullptr);
    sama_edit->setText(name);

    create_dialog->accept();

    QVERIFY2(object_exists(dn), "Created group doesn't exist");

    QVERIFY(true);
}

void ADMCTestObjectMenu::object_menu_find_simple() {
    const QString parent = test_arena_dn();

    const QString user_name = TEST_USER;
    const QString user_dn = test_object_dn(user_name, CLASS_USER);

    // Create test user
    const bool create_user_success = ad.object_add(user_dn, CLASS_USER);
    QVERIFY2(create_user_success, "Failed to create user");
    QVERIFY2(object_exists(user_dn), "Created user doesn't exist");

    auto find_dialog = new FindObjectDialog(filter_classes, parent, parent_widget);
    find_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(find_dialog, 1000));

    // Enter name in search field
    auto simple_tab = find_dialog->findChild<FilterWidgetSimpleTab *>();
    QVERIFY(simple_tab != nullptr);
    auto name_edit = simple_tab->findChild<QLineEdit *>("name_edit");
    QVERIFY(name_edit != nullptr);
    name_edit->setText(user_name);

    // Press find button
    auto find_button = find_dialog->findChild<QPushButton *>("find_button");
    QVERIFY(find_button != nullptr);
    find_button->click();

    // Confirm that results are not empty
    auto find_results = find_dialog->findChild<QTreeView *>();

    wait_for_find_results_to_load(find_results);

    QVERIFY2(find_results->model()->rowCount(), "No results found");
}

void ADMCTestObjectMenu::object_menu_find_advanced() {
    const QString parent = test_arena_dn();

    const QString user_name = TEST_USER;
    const QString user_dn = test_object_dn(user_name, CLASS_USER);

    // Create test user
    const bool create_user_success = ad.object_add(user_dn, CLASS_USER);
    QVERIFY2(create_user_success, "Failed to create user");
    QVERIFY2(object_exists(user_dn), "Created user doesn't exist");

    auto find_dialog = new FindObjectDialog(filter_classes, parent, parent_widget);
    find_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(find_dialog, 1000));

    auto tab_widget = find_dialog->findChild<QTabWidget *>();
    QVERIFY(tab_widget != nullptr);
    auto advanced_tab = find_dialog->findChild<FilterWidgetAdvancedTab *>();
    QVERIFY(advanced_tab != nullptr);
    tab_widget->setCurrentWidget(advanced_tab);

    auto filter_edit = advanced_tab->findChild<QPlainTextEdit *>();
    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_DN, user_dn);
    filter_edit->setPlainText(filter);

    auto find_button = find_dialog->findChild<QPushButton *>("find_button");
    QVERIFY(find_button != nullptr);
    find_button->click();

    auto find_results = find_dialog->findChild<QTreeView *>();

    wait_for_find_results_to_load(find_results);

    QVERIFY2(find_results->model()->rowCount(), "No results found");
}

void ADMCTestObjectMenu::object_menu_rename() {
    const QString old_name = TEST_USER;
    const QString new_name = old_name + "2";

    const QString old_dn = test_object_dn(old_name, CLASS_USER);
    const QString new_dn = dn_rename(old_dn, new_name);

    // Create test object
    const bool create_success = ad.object_add(old_dn, CLASS_USER);
    QVERIFY(create_success);
    QVERIFY(object_exists(old_dn));

    // Open rename dialog
    auto rename_dialog = new RenameUserDialog(parent_widget);
    rename_dialog->set_target(old_dn);
    rename_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(rename_dialog, 1000));

    // Enter new name
    auto name_edit = rename_dialog->findChild<QLineEdit *>("name_edit");
    QVERIFY(name_edit != nullptr);
    name_edit->setText(new_name);

    rename_dialog->accept();

    QVERIFY(object_exists(new_dn));
}

QTEST_MAIN(ADMCTestObjectMenu)
