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

#ifndef TEST_ADMC_H
#define TEST_ADMC_H

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

class QString;
class QTreeView;
class QPushButton;

class TestADMC : public QObject {
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
    void create_dialog_user();
    void create_dialog_ou();
    void object_menu_move();

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

    // Tests object's existance on the server.
    bool object_exists(const QString &dn);

    // Presses the Tab button. Use to cycle through input
    // widgets.
    void tab(const int n = 1);

    void delete_test_arena_recursive(const QString &dn);

    void navigate_until_object(QTreeView *view, const QString &target_dn);
    void wait_for_widget_exposed(QWidget *widget);
};

#endif /* TEST_ADMC_H */
