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
#include "console_types/console_object.h"
#include "create_object_dialog.h"
#include "find_object_dialog.h"
#include "select_object_advanced_dialog.h"
#include "find_widget.h"
#include "move_object_dialog.h"
#include "password_dialog.h"
#include "rename_object_dialog.h"
#include "select_container_dialog.h"
#include "select_object_dialog.h"
#include "filter_widget/filter_widget_simple_tab.h"
#include "filter_widget/filter_widget_advanced_tab.h"
#include "utils.h"

#include <QComboBox>
#include <QDebug>
#include <QModelIndex>
#include <QPushButton>
#include <QTest>
#include <QTreeView>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QLineEdit>

// Test that when adding an object from find dialog to
// select dialog, the correct object is added.
void ADMCTestObjectMenu::select_dialog_correct_object_added() {
    const QString parent = test_arena_dn();

    // Create 5 different users
    const QList<QString> names = {
        "test-user-1",
        "test-user-2",
        "test-user-3",
        "test-user-4",
        "test-user-5",
    };
    for (const QString &name : names) {
        const QString dn = test_object_dn(name, CLASS_USER);

        const bool create_success = ad.object_add(dn, CLASS_USER);
        QVERIFY2(create_success, "Failed to create test user");
    }

    const QString select_dn = test_object_dn("test-user-3", CLASS_USER);

    auto select_dialog = new SelectObjectDialog({CLASS_USER}, SelectObjectDialogMultiSelection_Yes, parent_widget);
    select_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(select_dialog, 1000));

    select_in_select_dialog(select_dialog, select_dn);

    // Switch to view containing selected object
    auto select_dialog_view = select_dialog->findChild<QTreeView *>();
    QVERIFY(select_dialog_view != nullptr);

    // Verify that user added from find dialog is in
    // selected list
    auto select_dialog_model = select_dialog_view->model();

    QVERIFY(select_dialog_model->rowCount() == 1);
    const QModelIndex index = select_dialog_model->index(0, 0);
    const QString index_dn = index.data(ObjectRole_DN).toString();

    QVERIFY(index_dn == select_dn);
}

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

    QTreeView *move_dialog_view = qobject_cast<QTreeView *>(QApplication::focusWidget());
    QVERIFY2((move_dialog_view != nullptr), "Failed to cast move_dialog_view");

    // Select move target in the view
    navigate_until_object(move_dialog_view, move_target_dn, ContainerRole_DN);

    move_dialog->accept();

    QVERIFY2(object_exists(user_dn_after_move), "Moved object doesn't exist");
    QVERIFY2(object_exists(user_dn_after_move), "Moved object doesn't exist");

    QVERIFY(true);
}

void ADMCTestObjectMenu::object_menu_reset_password() {
    const QString user_name = TEST_USER;
    const QString user_dn = test_object_dn(user_name, CLASS_USER);
    const QString password = "pass123!";

    // Create test user
    const bool create_user_success = ad.object_add(user_dn, CLASS_USER);
    QVERIFY2(create_user_success, "Failed to create user");
    QVERIFY2(object_exists(user_dn), "Created user doesn't exist");

    const QByteArray pwdLastSet_value_before = [=]() {
        const AdObject object = ad.search_object(user_dn);

        return object.get_value(ATTRIBUTE_PWD_LAST_SET);
    }();

    // Open password dialog
    const auto password_dialog = new PasswordDialog({user_dn}, parent_widget);
    password_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(password_dialog, 1000));

    // Enter password
    auto password_main_edit = password_dialog->findChild<QLineEdit *>("password_main_edit");
    QVERIFY(password_main_edit);
    password_main_edit->setText(password);

    auto password_confirm_edit = password_dialog->findChild<QLineEdit *>("password_confirm_edit");
    QVERIFY(password_confirm_edit);
    password_confirm_edit->setText(password);

    password_dialog->accept();

    const QByteArray pwdLastSet_value_after = [=]() {
        const AdObject object = ad.search_object(user_dn);

        return object.get_value(ATTRIBUTE_PWD_LAST_SET);
    }();

    const bool password_changed = (pwdLastSet_value_before != pwdLastSet_value_after);
    QVERIFY2(password_changed, "Failed to change password");
}

void ADMCTestObjectMenu::object_menu_disable_enable_account() {
    auto test_disable_enable = [=](const bool initial_disabled_state) {
        const QString dn = [=]() {
            const QString name = [=]() {
                if (initial_disabled_state) {
                    return QString("disabled-%1").arg(TEST_USER);
                } else {
                    return QString("enabled-%1").arg(TEST_USER);
                }
            }();

            return test_object_dn(name, CLASS_USER);
        }();

        const bool create_success = ad.object_add(dn, CLASS_USER);
        QVERIFY2(create_success, qPrintable(QString("Failed to create user - %1").arg(dn)));
        QVERIFY2(object_exists(dn), qPrintable(QString("Created user doesn't exist - %1").arg(dn)));

        // Setup initial disabled state
        const bool set_disabled_success = ad.user_set_account_option(dn, AccountOption_Disabled, initial_disabled_state);
        QVERIFY2(set_disabled_success, qPrintable(QString("Failed to set disabled account option for user - %1").arg(dn)));

        // Modify state using object menu
        object_operation_set_disabled({dn}, !initial_disabled_state, parent_widget);

        // Check that final disabled state has changed
        const AdObject object = ad.search_object(dn);
        const bool final_disabled_state = object.get_account_option(AccountOption_Disabled, ad.adconfig());
        const bool disabled_state_changed = (final_disabled_state != initial_disabled_state);

        const QString error_text = [=]() {
            if (initial_disabled_state) {
                return QString("Failed to enable user - %1").arg(dn);
            } else {
                return QString("Failed to disable user - %1").arg(dn);
            }
        }();
        QVERIFY2(disabled_state_changed, qPrintable(error_text));
    };

    test_disable_enable(true);
    test_disable_enable(false);
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

void ADMCTestObjectMenu::object_menu_add_to_group() {
    const QString parent = test_arena_dn();

    const QString user_name = TEST_USER;
    const QString user_dn = test_object_dn(user_name, CLASS_USER);

    const QString group_name = TEST_GROUP;
    const QString group_dn = test_object_dn(group_name, CLASS_GROUP);

    // Create test user
    const bool create_user_success = ad.object_add(user_dn, CLASS_USER);
    QVERIFY2(create_user_success, "Failed to create user");
    QVERIFY2(object_exists(user_dn), "Created user doesn't exist");

    // Create test group
    const bool create_group_success = ad.object_add(group_dn, CLASS_GROUP);
    QVERIFY2(create_group_success, "Failed to create group");
    QVERIFY2(object_exists(group_dn), "Created group doesn't exist");

    // Open add to group dialog
    object_operation_add_to_group({user_dn}, parent_widget);
    auto select_dialog = parent_widget->findChild<SelectObjectDialog *>();
    QVERIFY2((select_dialog != nullptr), "Failed to find select_dialog");
    QVERIFY(QTest::qWaitForWindowExposed(select_dialog, 1000));
    
    select_in_select_dialog(select_dialog, group_dn);
    select_dialog->accept();
    
    const AdObject group = ad.search_object(group_dn);
    const QList<QString> group_members = group.get_strings(ATTRIBUTE_MEMBER);
    const bool user_is_member_of_group = group_members.contains(user_dn);
    QVERIFY2(user_is_member_of_group, "User did not become member of group");

    QVERIFY(true);
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
