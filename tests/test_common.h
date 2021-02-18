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

#ifndef TEST_COMMON_H
#define TEST_COMMON_H

// Common util f-ns shared between tests

class QString;
class QTreeView;

#define TEST_USER "test-user"
#define TEST_USER_LOGON "test-user-logon"
#define TEST_PASSWORD "pass123!"
#define TEST_OU "test-ou"
#define TEST_GROUP "test-group"
#define TEST_COMPUTER "test-computer"

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

// Presses the Tab button. Use to cycle through input
// widgets.
void tab(const int n = 1);

void navigate_until_object(QTreeView *view, const QString &target_dn);

#endif /* TEST_COMMON_H */
