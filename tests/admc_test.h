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

#ifndef ADMC_TEST_H
#define ADMC_TEST_H

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

#include <QObject>

#include <QTest>

#include <functional>

class QString;
class QTreeView;
class QPushButton;

class ADMCTest : public QObject {
    Q_OBJECT

private slots:
    // NOTE: initTestCase(), cleanupTestCase(), init() and
    // cleanup() are special slots called by QTest.

    // Called before first test
    void initTestCase();
    // Called after last test
    void cleanupTestCase();

    // Run before and after each test
    void init();
    void cleanup();

    // Tests
    void object_menu_add_to_group();
    void object_add();
    void object_delete();
    
    void object_menu_new_user();
    void object_menu_new_ou();
    void object_menu_new_computer();
    void object_menu_new_group();

    void object_menu_move();
    void object_menu_rename_computer();
    void object_menu_rename_user();
    void object_menu_rename_ou();
    void object_menu_rename_group();
    void object_menu_reset_password();
    void object_menu_disable_enable_account();

    void object_menu_find_simple();
    void object_menu_find_advanced();

private:
    // Use this as parents for widgets used inside tests.
    // For every test a new parent will be created and after
    // test completes, parent is deleted which deletes all
    // child widgets as well.
    QWidget *parent_widget = nullptr;

    // For easy setup and cleanup of each test, we use an
    // object named "test-arena" which is an OU. It is
    // created before every test and deleted after every
    // test. All test activity should happen inside this
    // object.
    QString test_arena_dn();

    // Creates dn for object with given name whose parent is
    // test arena. Class is used to determine suffix.
    QString test_object_dn(const QString &name, const QString &object_class);

    // Creates object of given type with specified name and
    // runs a specified rename function on this object.
    void test_object_rename(const QString& oldname, const QString& newname, const QString &object_class, std::function<void(const QString&)> rename_callback);

    // Common renamer for object.
    static void basic_rename(const QString& newname);

    // Tests object's existance on the server.
    bool object_exists(const QString &dn);

    // Presses the Tab button. Use to cycle through input
    // widgets.
    void tab(const int n = 1);

    void navigate_until_object(QTreeView *view, const QString &target_dn);
    void wait_for_widget_exposed(QWidget *widget);
};

#endif /* ADMC_TEST_H */
