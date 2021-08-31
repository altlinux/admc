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
#include "move_object_dialog.h"
#include "rename_object_dialog.h"
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

void ADMCTestObjectMenu::object_menu_move() {
    const QString parent = test_arena_dn();

    const QString user_name = TEST_USER;
    const QString user_dn = test_object_dn(user_name, CLASS_USER);

    const QString move_target_name = "move-target-ou";
    const QString move_target_dn = test_object_dn(move_target_name, CLASS_OU);

    const QString user_dn_after_move = dn_from_name_and_parent(user_name, move_target_dn, CLASS_USER);

    // Create test user
    const bool create_user_success = ad.object_add(user_dn, CLASS_USER);
    QVERIFY2(create_user_success, "Failed to create user");
    QVERIFY2(object_exists(user_dn), "Created user doesn't exist");

    // Create move target
    const bool create_move_target_success = ad.object_add(move_target_dn, CLASS_OU);
    QVERIFY2(create_move_target_success, "Failed to create move target");
    QVERIFY2(object_exists(move_target_dn), "Created move target doesn't exist");

    // Open move dialog
    auto move_dialog = new MoveObjectDialog({user_dn}, parent_widget);
    move_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(move_dialog, 1000));

    QTreeView *move_dialog_view = move_dialog->findChild<QTreeView *>();
    QVERIFY2((move_dialog_view != nullptr), "Failed to cast move_dialog_view");

    // Select move target in the view
    navigate_until_object(move_dialog_view, move_target_dn, ContainerRole_DN);

    move_dialog->accept();

    QVERIFY2(object_exists(user_dn_after_move), "Moved object doesn't exist");
    QVERIFY2(object_exists(user_dn_after_move), "Moved object doesn't exist");

    QVERIFY(true);
}

void ADMCTestObjectMenu::object_menu_able_account_data() {
    QTest::addColumn<bool>("final_state");
    QTest::addColumn<QDate>("date");
    QTest::addColumn<QString>("value");

    QTest::newRow("enable") << true << QDate(2011, 11, 11) << "129655295400000000";
    QTest::newRow("disable") << false << QDate() << AD_LARGE_INTEGER_DATETIME_NEVER_2;
    ;
}

void ADMCTestObjectMenu::object_menu_able_account() {
    QFETCH(bool, final_state);

    const QString dn = test_object_dn(TEST_USER, CLASS_USER);

    ad.object_add(dn, CLASS_USER);

    // Setup initial disabled state
    ad.user_set_account_option(dn, AccountOption_Disabled, !final_state);

    // Modify state using object menu
    object_operation_set_disabled({dn}, final_state, parent_widget);

    // Check that final disabled state has changed
    const AdObject update_object = ad.search_object(dn);
    const bool current_state = update_object.get_account_option(AccountOption_Disabled, ad.adconfig());
    QCOMPARE(current_state, final_state);
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
    auto rename_dialog = new RenameObjectDialog(old_dn, parent_widget);
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
