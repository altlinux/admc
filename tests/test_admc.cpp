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

#include "test_admc.h"

#include "ad_interface.h"
#include "ad_defines.h"
#include "ad_object.h"
#include "ad_utils.h"
#include "filter.h"
#include "create_dialog.h"
#include "utils.h"
#include "status.h"

#include <QTest>
#include <QDebug>

// Delay between tabbing through input widgets, in milliseconds. Set this to some non-zero value if you want to watch the test play out in real time.
#define TAB_DELAY 0

#define TEST_USER "test-user"
#define TEST_USER_LOGON "test-user-logon"
#define TEST_PASSWORD "pass123!"
#define TEST_OU "test-ou"

void TestADMC::initTestCase() {
    const bool connected = AD()->connect();
    QVERIFY2(connected, "Failed to connect to AD server");

    // NOTE: temporary band-aid until messages are routed correctly throgh AdInterface instance. This makes status error messages be printed to console. I think it's useful to understand why a test failed. When messages are collected in an AdInterface instances, can just print them here ourselves and avoid touching Status.
    STATUS()->print_errors = true;

    // Cleanup before all tests in-case this test suite was
    // previously interrupted and a cleanup wasn't performed
    cleanup();
}

void TestADMC::cleanupTestCase() {

}

void TestADMC::init() {
    parent_widget = new QWidget();
    
    const QString dn = test_arena_dn();
    const bool create_success = AD()->object_add(dn, CLASS_OU);

    QVERIFY2(create_success, "Failed to create test-arena");
}

// NOTE: can't just delete test arena while it has children
// because LDAP forbids deleting non-leaf objects. So need
// to delete leaves first.
void TestADMC::delete_test_arena_recursive(const QString &parent) {
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

void TestADMC::cleanup() {
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

void TestADMC::object_add() {
    const QString name = TEST_USER;
    const QString dn = test_object_dn(name, CLASS_USER);

    const bool create_success = AD()->object_add(dn, CLASS_USER);

    QVERIFY2(create_success, "Failed to create object");
    QVERIFY2(object_exists(dn), "Created object doesn't exist");
}

void TestADMC::object_delete() {
    const QString name = TEST_USER;
    const QString dn = test_object_dn(name, CLASS_USER);

    const bool create_success = AD()->object_add(dn, CLASS_USER);
    QVERIFY2(create_success, "Failed to create object for deletion");

    const bool delete_success = AD()->object_delete(dn);
    QVERIFY2(delete_success, "Failed to delete object");
    QVERIFY2(!object_exists(dn), "Deleted object exists");
}

void TestADMC::create_dialog_user() {
    const QString name = TEST_USER;
    const QString logon_name = TEST_USER_LOGON;
    const QString password = TEST_PASSWORD;
    const QString parent = test_arena_dn();
    const QString dn = test_object_dn(name, CLASS_USER);

    // Create user
    const auto create_dialog = new CreateDialog(parent, CLASS_USER, parent_widget);
    create_dialog->open();
    QApplication::setActiveWindow(create_dialog);

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

    // Accept dialog. Need to only tab once since we're at the last line edit. After that focus goes to create button
    tab();
    QTest::keyClick(QApplication::focusWidget(), Qt::Key_Enter);

    QVERIFY2(object_exists(dn), "Created user doesn't exist");

    QVERIFY(true);
}

void TestADMC::create_dialog_ou() {
    const QString name = TEST_OU;
    const QString parent = test_arena_dn();
    const QString dn = test_object_dn(name, CLASS_OU);

    // Create ou
    const auto create_dialog = new CreateDialog(parent, CLASS_OU, parent_widget);
    create_dialog->open();
    QApplication::setActiveWindow(create_dialog);

    // Enter name
    QTest::keyClicks(QApplication::focusWidget(), name);

    // Accept dialog. Need to only tab once since we're at the last line edit. After that focus goes to create button
    tab();
    QTest::keyClick(QApplication::focusWidget(), Qt::Key_Enter);

    QVERIFY2(object_exists(dn), "Created OU doesn't exist");

    QVERIFY(true);
}

QString TestADMC::test_arena_dn() {
    const QString head_dn = AD()->domain_head();
    const QString dn = QString("ou=test-arena,%1").arg(head_dn);

    return dn;
}

// TODO: for OU's need to change from cn to ou, SO make a f-n in ad_utils.cpp that does what's currently in CreateDialog::accept(). Takes object name, parent dn and object class, returns dn.
QString TestADMC::test_object_dn(const QString &name, const QString &object_class) {
    const QString parent = test_arena_dn();
    const QString dn = dn_from_name_and_parent(name, parent, object_class);

    return dn;
}

bool TestADMC::object_exists(const QString &dn) {
    const QHash<QString, AdObject> search_results = AD()->search(QString(), QList<QString>(), SearchScope_Object, dn);
    const bool exists = (search_results.size() == 1);

    return exists;
}

void TestADMC::tab(const int n) {
    for (int i = 0; i < n; i++) {
        QTest::keyClick(QApplication::focusWidget(), Qt::Key_Tab);

        if (TAB_DELAY > 0) {
            QTest::qWait(TAB_DELAY);
        }
    }
}

QTEST_MAIN(TestADMC)
