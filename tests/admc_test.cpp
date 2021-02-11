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

#include "admc_test.h"

#include "ad_interface.h"
#include "ad_defines.h"
#include "ad_object.h"
#include "ad_utils.h"
#include "filter.h"
#include "create_dialog.h"
#include "utils.h"
#include "status.h"
#include "object_menu.h"
#include "object_model.h"
#include "select_container_dialog.h"
#include "select_dialog.h"
#include "find_select_dialog.h"
#include "settings.h"
#include "rename_dialog.h"
#include "password_dialog.h"
#include "find_dialog.h"

#include <QTest>
#include <QDebug>
#include <QModelIndex>
#include <QTreeView>
#include <QPushButton>
#include <QComboBox>

#define TEST_USER "test-user"
#define TEST_USER_LOGON "test-user-logon"
#define TEST_PASSWORD "pass123!"
#define TEST_OU "test-ou"
#define TEST_GROUP "test-group"
#define TEST_COMPUTER "test-computer"

#define PRINT_FOCUS_WIDGET_BEFORE_TAB false
#define PRINT_FOCUS_WIDGET_AFTER_TAB false

void ADMCTest::initTestCase() {
    const bool connected = AD()->connect();
    QVERIFY2(connected, "Failed to connect to AD server");

    // NOTE: temporary band-aid until messages are routed correctly throgh AdInterface instance. This makes status error messages be printed to console. I think it's useful to understand why a test failed. When messages are collected in an AdInterface instances, can just print them here ourselves and avoid touching Status.
    STATUS()->print_errors = true;

    // Cleanup before all tests in-case this test suite was
    // previously interrupted and a cleanup wasn't performed
    cleanup();
}

void ADMCTest::cleanupTestCase() {

}

void ADMCTest::init() {
    parent_widget = new QWidget();
    
    const QString dn = test_arena_dn();
    const bool create_success = AD()->object_add(dn, CLASS_OU);

    QVERIFY2(create_success, "Failed to create test-arena");
}

// NOTE: can't just delete test arena while it has children
// because LDAP forbids deleting non-leaf objects. So need
// to delete leaves first.
void ADMCTest::delete_test_arena_recursive(const QString &parent) {
    const QHash<QString, AdObject> search_results = AD()->search(QString(), QList<QString>(), SearchScope_Children, parent);

    const bool has_children = (search_results.size() > 0);
    if (has_children) {
        for (const QString child : search_results.keys()) {
            delete_test_arena_recursive(child);
        }
    }

    const bool delete_success = AD()->object_delete(parent);
    QVERIFY2(delete_success, "Failed to delete test-arena or it's contents");
}

void ADMCTest::cleanup() {
    if (parent_widget != nullptr) {
        delete parent_widget;
        parent_widget = nullptr;
    }

    // Delete test arena, if it exists
    const QString dn = test_arena_dn();

    const QHash<QString, AdObject> search_results = AD()->search(QString(), QList<QString>(), SearchScope_Object, dn);
    const bool test_arena_exists = (search_results.size() == 1);
    if (test_arena_exists) {
        delete_test_arena_recursive(dn);
    }
}

void ADMCTest::object_add() {
    const QString name = TEST_USER;
    const QString dn = test_object_dn(name, CLASS_USER);

    const bool create_success = AD()->object_add(dn, CLASS_USER);

    QVERIFY2(create_success, "Failed to create object");
    QVERIFY2(object_exists(dn), "Created object doesn't exist");
}

void ADMCTest::object_delete() {
    const QString name = TEST_USER;
    const QString dn = test_object_dn(name, CLASS_USER);

    const bool create_success = AD()->object_add(dn, CLASS_USER);
    QVERIFY2(create_success, "Failed to create object for deletion");

    const bool delete_success = AD()->object_delete(dn);
    QVERIFY2(delete_success, "Failed to delete object");
    QVERIFY2(!object_exists(dn), "Deleted object exists");
}

void ADMCTest::object_menu_new_user() {
    const QString name = TEST_USER;
    const QString logon_name = TEST_USER_LOGON;
    const QString password = TEST_PASSWORD;
    const QString parent = test_arena_dn();
    const QString dn = test_object_dn(name, CLASS_USER);

    // Create user
    create(parent, CLASS_USER, parent_widget);
    auto create_dialog = parent_widget->findChild<CreateDialog *>();
    QVERIFY2((create_dialog != nullptr), "Failed to find create dialog");
    wait_for_widget_exposed(create_dialog);

    // Enter name
    QTest::keyClicks(QApplication::focusWidget(), name);

    // Enter logon name
    tab(4);
    QTest::keyClicks(QApplication::focusWidget(), logon_name);

    // Enter password
    tab(2);
    QTest::keyClicks(QApplication::focusWidget(), password);
    // Confirm password
    tab();
    QTest::keyClicks(QApplication::focusWidget(), password);

    create_dialog->accept();

    QVERIFY2(object_exists(dn), "Created user doesn't exist");

    QVERIFY(true);
}

void ADMCTest::object_menu_new_ou() {
    const QString name = TEST_OU;
    const QString parent = test_arena_dn();
    const QString dn = test_object_dn(name, CLASS_OU);

    // Create ou
    create(parent, CLASS_OU, parent_widget);
    auto create_dialog = parent_widget->findChild<CreateDialog *>();
    wait_for_widget_exposed(create_dialog);

    // Enter name
    QTest::keyClicks(QApplication::focusWidget(), name);

    create_dialog->accept();

    QVERIFY2(object_exists(dn), "Created OU doesn't exist");

    QVERIFY(true);
}

void ADMCTest::object_menu_new_computer() {
    const QString object_class = CLASS_COMPUTER;
    const QString name = TEST_COMPUTER;
    const QString parent = test_arena_dn();
    const QString dn = test_object_dn(name, object_class);

    // Open create dialog
    create(parent, object_class, parent_widget);
    auto create_dialog = parent_widget->findChild<CreateDialog *>();
    wait_for_widget_exposed(create_dialog);

    // Enter name
    QTest::keyClicks(QApplication::focusWidget(), name);

    // Enter logon name
    tab();
    QTest::keyClicks(QApplication::focusWidget(), name);

    create_dialog->accept();

    QVERIFY2(object_exists(dn), "Created computer doesn't exist");

    QVERIFY(true);
}

void ADMCTest::object_menu_new_group() {
    const QString object_class = CLASS_GROUP;
    const QString name = TEST_GROUP;
    const QString parent = test_arena_dn();
    const QString dn = test_object_dn(name, object_class);

    // Open create dialog
    create(parent, object_class, parent_widget);
    auto create_dialog = parent_widget->findChild<CreateDialog *>();
    wait_for_widget_exposed(create_dialog);

    // Enter name
    QTest::keyClicks(QApplication::focusWidget(), name);

    // Enter logon name
    tab();
    QTest::keyClicks(QApplication::focusWidget(), name);

    create_dialog->accept();

    QVERIFY2(object_exists(dn), "Created group doesn't exist");

    QVERIFY(true);
}

void ADMCTest::object_menu_move() {
    const QString parent = test_arena_dn();
    
    const QString user_name = TEST_USER;
    const QString user_dn = test_object_dn(user_name, CLASS_USER);

    const QString move_target_name = "move-target-ou";
    const QString move_target_dn = test_object_dn(move_target_name, CLASS_OU);

    const QString user_dn_after_move = dn_from_name_and_parent(user_name, move_target_dn, CLASS_USER);

    // Create test user
    const bool create_user_success = AD()->object_add(user_dn, CLASS_USER);
    QVERIFY2(create_user_success, "Failed to create user");
    QVERIFY2(object_exists(user_dn), "Created user doesn't exist");

    // Create move target
    const bool create_move_target_success = AD()->object_add(move_target_dn, CLASS_OU);
    QVERIFY2(create_move_target_success, "Failed to create move target");
    QVERIFY2(object_exists(move_target_dn), "Created move target doesn't exist");

    // Open move dialog
    move({user_dn}, parent_widget);
    auto move_dialog = parent_widget->findChild<SelectContainerDialog *>();
    QVERIFY2((move_dialog != nullptr), "Failed to find move dialog");
    wait_for_widget_exposed(move_dialog);

    QTreeView *move_dialog_view = qobject_cast<QTreeView *>(QApplication::focusWidget());
    QVERIFY2((move_dialog_view != nullptr), "Failed to cast move_dialog_view");

    // Press right to expand view item
    auto expand_current =
    []() {
        QTest::keyClick(QApplication::focusWidget(), Qt::Key_Right);
    };

    // Select move target in the view
    // First current item is the head
    expand_current();
    navigate_until_object(move_dialog_view, test_arena_dn());
    expand_current();
    navigate_until_object(move_dialog_view, move_target_dn);

    move_dialog->accept();

    QVERIFY2(object_exists(user_dn_after_move), "Moved object doesn't exist");

    QVERIFY(true);
}

void ADMCTest::object_menu_reset_password() {
    const QString user_name = TEST_USER;
    const QString user_dn = test_object_dn(user_name, CLASS_USER);
    const QString password = "pass123!";

    // Create test user
    const bool create_user_success = AD()->object_add(user_dn, CLASS_USER);
    QVERIFY2(create_user_success, "Failed to create user");
    QVERIFY2(object_exists(user_dn), "Created user doesn't exist");

    const QByteArray pwdLastSet_value_before =
    [=]() {
        const AdObject object = AD()->search_object(user_dn);

        return object.get_value(ATTRIBUTE_PWD_LAST_SET);
    }();

    // Open password dialog
    reset_password(user_dn, parent_widget);
    auto password_dialog = parent_widget->findChild<PasswordDialog *>();
    QVERIFY2((password_dialog != nullptr), "Failed to find password_dialog");
    wait_for_widget_exposed(password_dialog);

    // Enter password
    QTest::keyClicks(QApplication::focusWidget(), password);

    // Confirm password
    tab();
    QTest::keyClicks(QApplication::focusWidget(), password);

    password_dialog->accept();

    const QByteArray pwdLastSet_value_after =
    [=]() {
        const AdObject object = AD()->search_object(user_dn);
        
        return object.get_value(ATTRIBUTE_PWD_LAST_SET);
    }();

    const bool password_changed = (pwdLastSet_value_before != pwdLastSet_value_after);
    QVERIFY2(password_changed, "Failed to change password");
}

void ADMCTest::object_menu_disable_enable_account() {
    auto test_disable_enable =
    [=](const bool initial_disabled_state) {
        const QString dn =
        [=]() {
            const QString name =
            [=]() {
                if (initial_disabled_state) {
                    return QString("disabled-%1").arg(TEST_USER);
                } else {
                    return QString("enabled-%1").arg(TEST_USER);
                }
            }();

            return test_object_dn(name, CLASS_USER);
        }();

        const bool create_success = AD()->object_add(dn, CLASS_USER);
        QVERIFY2(create_success, qPrintable(QString("Failed to create user - %1").arg(dn)));
        QVERIFY2(object_exists(dn), qPrintable(QString("Created user doesn't exist - %1").arg(dn)));

        // Setup initial disabled state
        const bool set_disabled_success = AD()->user_set_account_option(dn, AccountOption_Disabled, initial_disabled_state);
        QVERIFY2(set_disabled_success, qPrintable(QString("Failed to set disabled account option for user - %1").arg(dn)));

        // Modify state using object menu
        if (initial_disabled_state) {
            enable_account({dn}, parent_widget);
        } else {
            disable_account({dn}, parent_widget);
        }

        // Check that final disabled state has changed
        const AdObject object = AD()->search_object(dn);
        const bool final_disabled_state = object.get_account_option(AccountOption_Disabled);
        const bool disabled_state_changed = (final_disabled_state != initial_disabled_state);

        const QString error_text =
        [=]() {
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

void ADMCTest::object_menu_find_simple()
{
    const QString parent = test_arena_dn();

    const QString user_name = TEST_USER;
    const QString user_dn = test_object_dn(user_name, CLASS_USER);

    // Create test user
    const bool create_user_success = AD()->object_add(user_dn, CLASS_USER);
    QVERIFY2(create_user_success, "Failed to create user");
    QVERIFY2(object_exists(user_dn), "Created user doesn't exist");

    find(parent, parent_widget);

    auto find_dialog = parent_widget->findChild<FindDialog *>();
    QVERIFY2((find_dialog != nullptr), "Failed to find find_dialog");
    wait_for_widget_exposed(find_dialog);

    tab();
    tab();
    tab();

    QTest::keyClicks(QApplication::focusWidget(), user_name);

    auto children = find_dialog->findChildren<QPushButton*>();
    QPushButton* find_button = nullptr;
    for (const auto& child : children) {
        if (child->text() == tr("Find")) {
            find_button = child;
        }
    }

    QVERIFY2((find_button != nullptr), "Failed to find find_button");

    QTest::mouseClick(find_button, Qt::LeftButton);

    auto find_results = find_dialog->findChild<QTreeView*>();

    QVERIFY2(find_results->model()->rowCount(), "No results found");
}

void ADMCTest::object_menu_find_advanced()
{
    const QString parent = test_arena_dn();

    const QString user_name = TEST_USER;
    const QString user_dn = test_object_dn(user_name, CLASS_USER);

    // Create test user
    const bool create_user_success = AD()->object_add(user_dn, CLASS_USER);
    QVERIFY2(create_user_success, "Failed to create user");
    QVERIFY2(object_exists(user_dn), "Created user doesn't exist");

    find(parent, parent_widget);

    auto find_dialog = parent_widget->findChild<FindDialog *>();
    QVERIFY2((find_dialog != nullptr), "Failed to find find_dialog");
    wait_for_widget_exposed(find_dialog);

    tab();
    tab();
    QTest::keyClick(QApplication::focusWidget(), Qt::Key_Right);
    QTest::keyClick(QApplication::focusWidget(), Qt::Key_Right);
    tab();

    QTest::keyClicks(QApplication::focusWidget(), QString("(objectClass=*)"));

    auto children = find_dialog->findChildren<QPushButton*>();
    QPushButton* find_button = nullptr;
    for (const auto& child : children) {
        if (child->text() == tr("Find")) {
            find_button = child;
        }
    }

    QVERIFY2((find_button != nullptr), "Failed to find find_button");

    QTest::mouseClick(find_button, Qt::LeftButton);    

    auto find_results = find_dialog->findChild<QTreeView*>();

    QVERIFY2(find_results->model()->rowCount(), "No results found");
}

void ADMCTest::object_menu_add_to_group() {
    const QString parent = test_arena_dn();
    
    const QString user_name = TEST_USER;
    const QString user_dn = test_object_dn(user_name, CLASS_USER);

    const QString group_name = TEST_GROUP;
    const QString group_dn = test_object_dn(group_name, CLASS_GROUP);

    // Create test user
    const bool create_user_success = AD()->object_add(user_dn, CLASS_USER);
    QVERIFY2(create_user_success, "Failed to create user");
    QVERIFY2(object_exists(user_dn), "Created user doesn't exist");

    // Create test group
    const bool create_group_success = AD()->object_add(group_dn, CLASS_GROUP);
    QVERIFY2(create_group_success, "Failed to create group");
    QVERIFY2(object_exists(group_dn), "Created group doesn't exist");

    // Open add to group dialog
    add_to_group({user_dn}, parent_widget);
    auto select_dialog = parent_widget->findChild<SelectDialog *>();
    QVERIFY2((select_dialog != nullptr), "Failed to find select_dialog");
    wait_for_widget_exposed(select_dialog);

    // Press "Add" button in select dialog
    tab();
    QTest::keyClick(QApplication::focusWidget(), Qt::Key_Space);

    // Find dialog has been opened, so switch to it
    auto find_select_dialog = select_dialog->findChild<FindSelectDialog *>();
    QVERIFY2((find_select_dialog != nullptr), "Failed to find find_select_dialog");
    wait_for_widget_exposed(find_select_dialog);

    // Enter group name in "Name" edit
    tab(3);
    QTest::keyClicks(QApplication::focusWidget(), group_name);

    // Press "Find" button
    tab(4);
    QTest::keyClick(QApplication::focusWidget(), Qt::Key_Space);

    // Switch to find results
    tab(2);
    QTreeView *find_results_view = qobject_cast<QTreeView *>(QApplication::focusWidget());
    QVERIFY2((find_results_view != nullptr), "Failed to cast find_results_view");

    // Select group in view
    navigate_until_object(find_results_view, group_dn);
    const QModelIndex selected_index = find_results_view->selectionModel()->currentIndex();
    const QString selected_dn = selected_index.data(Role_DN).toString();
    const QList<QList<QStandardItem *>> selected_rows = find_select_dialog->get_selected_rows();

    find_select_dialog->accept();

    select_dialog->accept();

    const AdObject group = AD()->search_object(group_dn);
    const QList<QString> group_members = group.get_strings(ATTRIBUTE_MEMBER);
    const bool user_is_member_of_group = group_members.contains(user_dn);
    QVERIFY2(user_is_member_of_group, "User did not become member of group");

    QVERIFY(true);
}

void ADMCTest::basic_rename(const QString& newname) {
    QTest::keyClicks(QApplication::focusWidget(), "A", Qt::ControlModifier);

    QTest::keyClick(QApplication::focusWidget(), Qt::Key_Delete);

    QTest::keyClicks(QApplication::focusWidget(), newname);
};

void ADMCTest::object_menu_rename_computer() {
    const QString computer_name = TEST_COMPUTER;
    const QString computer_renamed = computer_name + "2";

    test_object_rename(computer_name, computer_renamed, CLASS_COMPUTER, basic_rename);
}

void ADMCTest::object_menu_rename_user()
{
    const QString user_name = TEST_USER;
    const QString user_renamed = user_name + "2";

    test_object_rename(user_name, user_renamed, CLASS_USER, basic_rename);
}

void ADMCTest::object_menu_rename_ou()
{
    const QString ou_name = TEST_OU;
    const QString ou_renamed = ou_name + "2";

    test_object_rename(ou_name, ou_renamed, CLASS_OU, basic_rename);
}

void ADMCTest::object_menu_rename_group()
{
    const QString group_name = TEST_GROUP;
    const QString group_renamed = group_name + "2";

    auto group_rename =
    [this](const QString& newname) {
        ADMCTest::basic_rename(newname);
        tab();
        ADMCTest::basic_rename(newname);
    };

    test_object_rename(group_name, group_renamed, CLASS_GROUP, group_rename);
}

QString ADMCTest::test_arena_dn() {
    const QString head_dn = AD()->domain_head();
    const QString dn = QString("OU=test-arena,%1").arg(head_dn);

    return dn;
}

// TODO: for OU's need to change from cn to ou, SO make a f-n in ad_utils.cpp that does what's currently in CreateDialog::accept(). Takes object name, parent dn and object class, returns dn.
QString ADMCTest::test_object_dn(const QString &name, const QString &object_class) {
    const QString parent = test_arena_dn();
    const QString dn = dn_from_name_and_parent(name, parent, object_class);

    return dn;
}

void ADMCTest::test_object_rename(const QString& oldname, const QString& newname, const QString &object_class, std::function<void(const QString &)> rename_callback) {
    const QString parent = test_arena_dn();

    const QString dn = test_object_dn(oldname, object_class);

    const QString dn_after_rename = dn_rename(dn, newname);

    // Create test object
    const bool create_success = AD()->object_add(dn, object_class);
    QVERIFY2(create_success, qPrintable(QString("Failed to create %1").arg(object_class)));
    QVERIFY2(object_exists(dn), qPrintable(QString("Created %1 doesn't exist").arg(object_class)));

    // Open rename dialog
    rename({dn}, parent_widget);
    auto rename_dialog = parent_widget->findChild<RenameDialog *>();
    QVERIFY2((rename_dialog != nullptr), "Failed to find rename dialog");
    wait_for_widget_exposed(rename_dialog);

    rename_callback(newname);

    rename_dialog->accept();

    QVERIFY2(object_exists(dn_after_rename), "Renamed object doesn't exist");
}

bool ADMCTest::object_exists(const QString &dn) {
    const QHash<QString, AdObject> search_results = AD()->search(QString(), QList<QString>(), SearchScope_Object, dn);
    const bool exists = (search_results.size() == 1);

    return exists;
}

void ADMCTest::tab(const int n) {
    static int tab_number = 0;
    for (int i = 0; i < n; i++) {
        if (PRINT_FOCUS_WIDGET_BEFORE_TAB) {
            qInfo() << tab_number << "=" << QApplication::focusWidget();
        }

        QTest::keyClick(QApplication::focusWidget(), Qt::Key_Tab);

        if (PRINT_FOCUS_WIDGET_AFTER_TAB) {
            qInfo() << tab_number << "=" << QApplication::focusWidget();
        }
    }
}

// Go down the list of objects by pressing Down arrow
// until current item's dn equals to target dn
void ADMCTest::navigate_until_object(QTreeView *view, const QString &target_dn) {
    QModelIndex prev_index;

    while (true) {
        const QModelIndex current_index = view->selectionModel()->currentIndex();

        const QString current_dn = current_index.data(Role_DN).toString();
        const bool found_object = (current_dn == target_dn);
        if (found_object) {
            // NOTE: have to set current to select the row. If the first item in view happens to match and no navigation is done, then that first row won't be "selected". Widgets that select items from views rely on whole rows being selected, like they are when you click on them.
            view->setCurrentIndex(current_index);
            break;
        }

        // NOTE: when reached end of view, current index
        // will stop changing
        const bool navigated_to_end_of_view = (prev_index == current_index);
        QVERIFY2(!navigated_to_end_of_view, "Navigated to end of view and failed to find object");
        if (navigated_to_end_of_view) {
            break;
        }

        QTest::keyClick(QApplication::focusWidget(), Qt::Key_Down);

        prev_index = current_index;
    }
}

void ADMCTest::wait_for_widget_exposed(QWidget *widget) {
    const bool window_became_exposed = QTest::qWaitForWindowExposed(widget, 1000);
    QVERIFY(window_became_exposed);
}

QTEST_MAIN(ADMCTest)
