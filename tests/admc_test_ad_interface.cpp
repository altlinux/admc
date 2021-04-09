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

#include "admc_test_ad_interface.h"

#include "adldap.h"
#include "create_dialog.h"
#include "utils.h"
#include "status.h"
#include "object_model.h"
#include "select_container_dialog.h"
#include "select_dialog.h"
#include "find_select_dialog.h"
#include "settings.h"
#include "rename_dialog.h"
#include "find_dialog.h"
#include "move_dialog.h"
#include "password_dialog.h"
#include "find_widget.h"
#include "object_actions.h"

#include <QTest>
#include <QDebug>
#include <QModelIndex>
#include <QTreeView>
#include <QPushButton>
#include <QComboBox>

void ADMCTestAdInterface::object_add() {
    const QString dn = test_object_dn(TEST_USER, CLASS_USER);

    const bool create_success = ad.object_add(dn, CLASS_USER);

    QVERIFY2(create_success, "Failed to create object");
    QVERIFY2(object_exists(dn), "Created object doesn't exist");
}

void ADMCTestAdInterface::object_delete() {
    const QString dn = test_object_dn(TEST_USER, CLASS_USER);

    const bool create_success = ad.object_add(dn, CLASS_USER);
    QVERIFY2(create_success, "Failed to create object for deletion");

    const bool delete_success = ad.object_delete(dn);
    QVERIFY2(delete_success, "Failed to delete object");
    QVERIFY2(!object_exists(dn), "Deleted object exists");
}

void ADMCTestAdInterface::object_move() {
    const QString user_dn = test_object_dn(TEST_USER, CLASS_USER);
    const bool add_user_success = ad.object_add(user_dn, CLASS_USER);
    QVERIFY(add_user_success);

    const QString ou_dn = test_object_dn(TEST_OU, CLASS_OU);
    const bool add_ou_success = ad.object_add(ou_dn, CLASS_OU);
    QVERIFY(add_ou_success);

    const bool move_success = ad.object_move(user_dn, ou_dn);
    QVERIFY(move_success);

    const QString user_dn_after_move = dn_move(user_dn, ou_dn);
    QVERIFY(object_exists(user_dn_after_move));
}

void ADMCTestAdInterface::object_rename() {
    const QString user_dn = test_object_dn(TEST_USER, CLASS_USER);
    const bool add_user_success = ad.object_add(user_dn, CLASS_USER);
    QVERIFY(add_user_success);

    const QString new_name = "new-name";

    const bool rename_success = ad.object_rename(user_dn, new_name);
    QVERIFY(rename_success);

    const QString new_dn = test_object_dn(new_name, CLASS_USER);
    QVERIFY(object_exists(new_dn));
}

void ADMCTestAdInterface::group_add_member() {
    const QString user_dn = test_object_dn(TEST_USER, CLASS_USER);
    const bool add_user_success = ad.object_add(user_dn, CLASS_USER);
    QVERIFY(add_user_success);

    const QString group_dn = test_object_dn(TEST_GROUP, CLASS_GROUP);
    const bool add_group_success = ad.object_add(group_dn, CLASS_GROUP);
    QVERIFY(add_group_success);

    const bool add_member_success = ad.group_add_member(group_dn, user_dn);
    QVERIFY(add_member_success);

    const AdObject group_object = ad.search_object(group_dn);
    const QList<QString> member_list = group_object.get_strings(ATTRIBUTE_MEMBER);
    QVERIFY(member_list == QList<QString>({user_dn}));
}

void ADMCTestAdInterface::group_remove_member() {
    group_add_member();

    const QString user_dn = test_object_dn(TEST_USER, CLASS_USER);
    const QString group_dn = test_object_dn(TEST_GROUP, CLASS_GROUP);

    const bool remove_member_success = ad.group_remove_member(group_dn, user_dn);
    QVERIFY(remove_member_success);

    const AdObject group_object = ad.search_object(group_dn);
    const QList<QString> member_list = group_object.get_strings(ATTRIBUTE_MEMBER);
    QVERIFY(member_list.isEmpty());
}

void ADMCTestAdInterface::group_set_scope() {
    const QString group_dn = test_object_dn(TEST_GROUP, CLASS_GROUP);
    const bool add_group_success = ad.object_add(group_dn, CLASS_GROUP);
    QVERIFY(add_group_success);


        const AdObject group_object2 = ad.search_object(group_dn);
        const GroupScope current_scope2 = group_object2.get_group_scope();

    for (int scope_i = 0; scope_i < GroupScope_COUNT; scope_i++) {
        const GroupScope scope = (GroupScope) scope_i;
        const bool success = ad.group_set_scope(group_dn, scope);

        const AdObject group_object = ad.search_object(group_dn);
        const GroupScope current_scope = group_object.get_group_scope();
        QVERIFY(current_scope == scope);
    }
}

void ADMCTestAdInterface::group_set_type() {
    const QString group_dn = test_object_dn(TEST_GROUP, CLASS_GROUP);
    const bool add_group_success = ad.object_add(group_dn, CLASS_GROUP);
    QVERIFY(add_group_success);

    for (int type_i = 0; type_i < GroupType_COUNT; type_i++) {
        const GroupType type = (GroupType) type_i;
        const bool success = ad.group_set_type(group_dn, type);

        const AdObject group_object = ad.search_object(group_dn);
        const GroupType current_type = group_object.get_group_type();
        QVERIFY(current_type == type);
    }
}

void ADMCTestAdInterface::user_set_account_option() {
    const QString user_dn = test_object_dn(TEST_USER, CLASS_USER);
    const bool add_user_success = ad.object_add(user_dn, CLASS_USER);
    QVERIFY(add_user_success);

    for (int option_i = 0; option_i < AccountOption_COUNT; option_i++) {
        const AccountOption option = (AccountOption) option_i;
        const bool success = ad.user_set_account_option(user_dn, option, true);

        const AdObject object = ad.search_object(user_dn);
        const bool option_set = object.get_account_option(option);
        QVERIFY(option_set);
    }
}

QTEST_MAIN(ADMCTestAdInterface)
