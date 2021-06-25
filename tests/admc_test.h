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

#ifndef ADMC_TEST_H
#define ADMC_TEST_H

/**
 * Base test class for testing ADMC. Implements init and
 * cleanup f-ns that create a fresh testing environment for
 * each test.
 */

#include <QObject>

#include <QTest>

#include "adldap.h"

class QString;
class QTreeView;
class QPushButton;
class SelectObjectDialog;

#define TEST_USER "test-user"
#define TEST_USER_LOGON "test-user-logon"
#define TEST_PASSWORD "pass123!"
#define TEST_OU "test-ou"
#define TEST_GROUP "test-group"
#define TEST_COMPUTER "test-computer"

class ADMCTest : public QObject {
    Q_OBJECT

public slots:
    // NOTE: initTestCase(), cleanupTestCase(), init() and
    // cleanup() are special slots called by QTest.

    // Called before first test
    virtual void initTestCase();
    // Called after last test
    void cleanupTestCase();

    // Called before and after each test
    virtual void init();
    void cleanup();

protected:
    AdInterface ad;

    // Use this as parents for widgets used inside tests.
    // For every test a new parent will be created and after
    // test completes, parent is deleted which deletes all
    // child widgets as well.
    QWidget *parent_widget = nullptr;

    void init_test_case();

    // For easy setup and cleanup of each test, we use an
    // object named "test-arena" which is an OU. It is
    // created before every test and deleted after every
    // test. All test activity should happen inside this
    // object.
    QString test_arena_dn();

    // Creates dn for object with given name whose parent is
    // test arena. Class is used to determine suffix.
    QString test_object_dn(const QString &name, const QString &object_class);

    // Tests object's existance on the server.
    bool object_exists(const QString &dn);

    // Call this after pressing "Find" button. Needed
    // because find results are loaded in separate thread.
    void wait_for_find_results_to_load(QTreeView *view);

    // Message boxes block executation because they are
    // opened using exec(). Therefore when testing f-ns that
    // can open messageboxes, call this to to close
    // messageboxes later.
    void close_message_box_later();

    void select_in_select_dialog(SelectObjectDialog *select_dialog, const QString &name);

private:
    void close_message_box_slot();
};

void navigate_until_object(QTreeView *view, const QString &target_dn, const int dn_role);

// Presses the Tab button. Use to cycle through input
// widgets.
void tab(const int n = 1);

#endif /* ADMC_TEST_H */
