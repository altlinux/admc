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

#ifndef ADMC_TEST_H
#define ADMC_TEST_H

/**
 * Base test class for testing ADMC. Implements init and
 * cleanup f-ns that create a fresh testing environment for
 * each test.
 */

#include <QTest>

#include "adldap.h"

class QTreeView;
class SelectObjectDialog;
class SelectBaseWidget;
class QFormLayout;
class AttributeEdit;

#define TEST_USER "ADMCTEST-test-user"
#define TEST_USER_LOGON "ADMCTEST-test-user-logon"
#define TEST_PASSWORD "ADMCTEST-pass123!"
#define TEST_OU "ADMCTEST-test-ou"
#define TEST_GROUP "ADMCTEST-test-group"
// NOTE: use shorter length for computer to fit within
// 16 char length limit for sam account name
#define TEST_COMPUTER "ADMCTEST-pc"
#define TEST_OBJECT "ADMCTEST-object"

class ADMCTest : public QObject {
    Q_OBJECT

public slots:
    // NOTE: initTestCase(), cleanupTestCase(), init() and
    // cleanup() are special slots called by QTest.

    // Called before first test
    virtual void initTestCase();
    // Called after last test
    virtual void cleanupTestCase();

    // Called before and after each test
    virtual void init();
    virtual void cleanup();

protected:
    AdInterface ad;

    // Use this as parents for widgets used inside tests.
    // For every test a new parent will be created and after
    // test completes, parent is deleted which deletes all
    // child widgets as well.
    QWidget *parent_widget = nullptr;

    // This list is just for passing to edit ctors
    QList<AttributeEdit *> edits;

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

    void select_in_select_dialog(SelectObjectDialog *select_dialog, const QString &dn);

    // This is for closing message boxes opened using
    // open(). Won't work for QMessageBox static f-ns
    void close_message_box();

    bool message_box_is_open() const;

    // Selects an object via an already open select object
    // dialog. Object must be inside test arena
    void select_object_dialog_select(const QString &dn);

    // Adds a widget to layout in parent widget
    void add_widget(QWidget *widget);

    void test_edit_apply_unmodified(AttributeEdit *edit, const QString &dn);

    // Add a base to the base combo. Note that it is also
    // automatically selected.
    void select_base_widget_add(SelectBaseWidget *widget, const QString &dn);

private:
    QFormLayout *layout;
};

void navigate_until_object(QTreeView *view, const QString &target_dn, const int dn_role);
void test_lineedit_autofill(QLineEdit *src_edit, QLineEdit *dest_edit);

#endif /* ADMC_TEST_H */
