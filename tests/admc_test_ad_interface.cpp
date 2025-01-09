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

#include "admc_test_ad_interface.h"

#include "globals.h"
#include "samba/dom_sid.h"

#include <QTest>

#define TEST_GPO "ADMCTestAdInterface_TEST_GPO"

void ADMCTestAdInterface::cleanup() {
    // Delete test gpo, if it was leftover from previous test
    const QString base = g_adconfig->domain_dn();
    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_DISPLAY_NAME, TEST_GPO);
    const QList<QString> attributes = QList<QString>();
    const QHash<QString, AdObject> search_results = ad.search(base, SearchScope_All, filter, attributes);

    if (!search_results.isEmpty()) {
        const QString dn = search_results.keys()[0];
        bool deleted_object;
        ad.gpo_delete(dn, &deleted_object);
    }

    ADMCTest::cleanup();
}

void ADMCTestAdInterface::create_and_gpo_delete() {
    // Create new gpo
    QString gpo_dn;
    const bool create_success = ad.gpo_add(TEST_GPO, gpo_dn);
    QVERIFY(create_success);
    QVERIFY(!gpo_dn.isEmpty());

    // Check perms
    bool gpo_check_perms_ok = true;
    const bool perms_are_ok = ad.gpo_check_perms(gpo_dn, &gpo_check_perms_ok);
    QVERIFY(gpo_check_perms_ok);
    QVERIFY(perms_are_ok);

    // Create test ou
    const QString ou_dn = test_object_dn(TEST_OU, CLASS_OU);
    ad.object_add(ou_dn, CLASS_OU);

    // Link gpo to ou
    {
        const AdObject ou_object = ad.search_object(ou_dn);
        Gplink gplink = Gplink(ou_object.get_string(ATTRIBUTE_GPLINK));
        gplink.add(gpo_dn);
        ad.attribute_replace_string(ou_dn, ATTRIBUTE_GPLINK, gplink.to_string());
    }

    auto ou_is_linked_to_gpo = [&]() {
        const AdObject ou_object = ad.search_object(ou_dn);
        Gplink gplink = Gplink(ou_object.get_string(ATTRIBUTE_GPLINK));
        const bool out = gplink.contains(gpo_dn);

        return out;
    };

    const bool linked_before = ou_is_linked_to_gpo();
    QCOMPARE(linked_before, true);

    // Delete again;
    bool deleted_object;
    const bool delete_created_success = ad.gpo_delete(gpo_dn, &deleted_object);
    QVERIFY(delete_created_success);

    const bool linked_after = ou_is_linked_to_gpo();
    QCOMPARE(linked_after, false);
}

void ADMCTestAdInterface::gpo_check_perms() {
    QString gpc_dn;
    const bool create_success = ad.gpo_add(TEST_GPO, gpc_dn);
    QVERIFY(create_success);
    QVERIFY(!gpc_dn.isEmpty());

    bool gpo_check_perms_ok_1 = true;
    const bool perms_before = ad.gpo_check_perms(gpc_dn, &gpo_check_perms_ok_1);
    QVERIFY(gpo_check_perms_ok_1);
    QCOMPARE(perms_before, true);

    // Change GPC perms so they don't match with GPT perms
    {
        security_descriptor *new_sd = [&]() {
            const AdObject gpc_object = ad.search_object(gpc_dn);
            security_descriptor *out = gpc_object.get_security_descriptor();

            // NOTE: S-1-1-0 is "WORLD"
            const QByteArray trustee_everyone = sid_string_to_bytes("S-1-1-0");

            const QList<QString> class_list = gpc_object.get_strings(ATTRIBUTE_OBJECT_CLASS);

            SecurityRight right{SEC_ADS_GENERIC_ALL, QByteArray(), QByteArray(), 0};

            security_descriptor_add_right(out, ad.adconfig(), class_list, trustee_everyone, right, true);

            return out;
        }();

        ad_security_replace_security_descriptor(ad, gpc_dn, new_sd);
    }

    bool gpo_check_perms_ok_2 = true;
    const bool perms_after = ad.gpo_check_perms(gpc_dn, &gpo_check_perms_ok_2);
    QVERIFY(gpo_check_perms_ok_2);
    QCOMPARE(perms_after, false);

    bool deleted_object;
    const bool delete_success = ad.gpo_delete(gpc_dn, &deleted_object);
    QVERIFY(delete_success);
}

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
    QCOMPARE(member_list, QList<QString>({user_dn}));
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

    for (int scope_i = 0; scope_i < GroupScope_COUNT; scope_i++) {
        const GroupScope scope = (GroupScope) scope_i;
        const bool success = ad.group_set_scope(group_dn, scope);
        QVERIFY(success);

        const AdObject group_object = ad.search_object(group_dn);
        const GroupScope current_scope = group_object.get_group_scope();
        QCOMPARE(current_scope, scope);
    }
}

void ADMCTestAdInterface::group_set_type() {
    const QString group_dn = test_object_dn(TEST_GROUP, CLASS_GROUP);
    const bool add_group_success = ad.object_add(group_dn, CLASS_GROUP);
    QVERIFY(add_group_success);

    for (int type_i = 0; type_i < GroupType_COUNT; type_i++) {
        const GroupType type = (GroupType) type_i;
        const bool success = ad.group_set_type(group_dn, type);
        QVERIFY(success);

        const AdObject group_object = ad.search_object(group_dn);
        const GroupType current_type = group_object.get_group_type();
        QCOMPARE(current_type, type);
    }
}

void ADMCTestAdInterface::user_set_account_option() {
    const QString user_dn = test_object_dn(TEST_USER, CLASS_USER);
    const bool add_user_success = ad.object_add(user_dn, CLASS_USER);
    QVERIFY(add_user_success);

    for (int option_i = 0; option_i < AccountOption_COUNT; option_i++) {
        const AccountOption option = (AccountOption) option_i;
        const bool success = ad.user_set_account_option(user_dn, option, true);
        QVERIFY(success);

        const AdObject object = ad.search_object(user_dn);
        const bool option_set = object.get_account_option(option, g_adconfig);
        QVERIFY(option_set);
    }
}

QTEST_MAIN(ADMCTestAdInterface)
