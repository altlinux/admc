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

#include "admc_test_ad_security.h"

#include "ad_security.h"
#include "samba/ndr_security.h"

// NOTE: using "int" instead of "uint32_t" for test
// data because "uint32_t" is not supported by
// QTest::addColumn()

void ADMCTestAdSecurity::initTestCase() {
    // NOTE: important to set sd early here, because
    // base class initTestCase() calls cleanup() which
    // needs to not free sd
    sd = NULL;

    ADMCTest::initTestCase();
}

void ADMCTestAdSecurity::init() {
    ADMCTest::init();

    // Create test user
    test_user_dn = test_object_dn(TEST_USER, CLASS_USER);
    const bool create_test_user_success = ad.object_add(test_user_dn, CLASS_USER);
    QVERIFY(create_test_user_success);

    // Create test trustee (which is a user)
    test_trustee_dn = test_object_dn("test-trustee", CLASS_USER);
    const bool create_test_trustee_success = ad.object_add(test_trustee_dn, CLASS_USER);
    QVERIFY(create_test_trustee_success);

    test_trustee = [&]() {
        const AdObject test_trustee_object = ad.search_object(test_user_dn);
        const QByteArray out = test_trustee_object.get_value(ATTRIBUTE_OBJECT_SID);

        return out;
    }();

    load_sd();
}

void ADMCTestAdSecurity::cleanup() {
    ADMCTest::cleanup();

    security_descriptor_free(sd);
    sd = NULL;
}

void ADMCTestAdSecurity::add_right() {
    SecurityRight right{SEC_ADS_CREATE_CHILD, QByteArray(), QByteArray(), 0};
    security_descriptor_add_right(sd, ad.adconfig(), class_list, test_trustee, right, true);

    check_state(test_trustee, SEC_ADS_CREATE_CHILD, QByteArray(), TestAdSecurityType_Allow);
}

void ADMCTestAdSecurity::remove_right() {
    SecurityRight right{SEC_ADS_CREATE_CHILD, QByteArray(), QByteArray(), 0};
    security_descriptor_add_right(sd, ad.adconfig(), class_list, test_trustee, right, true);
    check_state(test_trustee, SEC_ADS_CREATE_CHILD, QByteArray(), TestAdSecurityType_Allow);

    security_descriptor_remove_right(sd, ad.adconfig(), class_list, test_trustee, right, true);
    check_state(test_trustee, SEC_ADS_CREATE_CHILD, QByteArray(), TestAdSecurityType_None);
}

void ADMCTestAdSecurity::remove_trustee() {
    const QList<uint32_t> mask_list = {
        SEC_ADS_CREATE_CHILD,
        SEC_STD_DELETE,
    };

    for (const uint32_t &mask : mask_list) {
        SecurityRight right{mask, QByteArray(), QByteArray(), 0};
        security_descriptor_add_right(sd, ad.adconfig(), class_list, test_trustee, right, true);
    }

    for (const uint32_t &mask : mask_list) {
        check_state(test_trustee, mask, QByteArray(), TestAdSecurityType_Allow);
    }

    security_descriptor_remove_trustee(sd, {test_trustee});

    for (const uint32_t &mask : mask_list) {
        check_state(test_trustee, mask, QByteArray(), TestAdSecurityType_None);
    }
}

// Removing generic read while full control is
// allowed, should leave generic write, even though
// they share a bit, and vice versa
void ADMCTestAdSecurity::handle_generic_read_and_write_sharing_bit() {
    const QHash<uint32_t, uint32_t> opposite_map = {
        {SEC_ADS_GENERIC_READ, SEC_ADS_GENERIC_WRITE},
        {SEC_ADS_GENERIC_WRITE, SEC_ADS_GENERIC_READ},
    };

    for (const uint32_t &mask : opposite_map.keys()) {
        const uint32_t opposite = opposite_map[mask];
        SecurityRight right{mask, QByteArray(), QByteArray(), 0};
        SecurityRight right_opposite{opposite, QByteArray(), QByteArray(), 0};
        security_descriptor_add_right(sd, ad.adconfig(), class_list, test_trustee, right, true);
        security_descriptor_add_right(sd, ad.adconfig(), class_list, test_trustee, right_opposite, true);

        check_state(test_trustee, mask, QByteArray(), TestAdSecurityType_Allow);
        check_state(test_trustee, opposite, QByteArray(), TestAdSecurityType_Allow);

        security_descriptor_remove_right(sd, ad.adconfig(), class_list, test_trustee, right, true);

        check_state(test_trustee, opposite, QByteArray(), TestAdSecurityType_Allow);
    }
}

void ADMCTestAdSecurity::protected_against_deletion_data() {
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<TestAdSecurityType>("correct_type");

    QTest::newRow("enabled") << true << TestAdSecurityType_Deny;
    QTest::newRow("disabled") << false << TestAdSecurityType_None;
}

void ADMCTestAdSecurity::protected_against_deletion() {
    QFETCH(bool, enabled);
    QFETCH(TestAdSecurityType, correct_type);

    ad_security_set_protected_against_deletion(ad, test_user_dn, enabled);

    // NOTE: need to reload sd since this modifies it
    // on the server!
    load_sd();

    // Check state ourselves
    const QByteArray trustee_everyone = sid_string_to_bytes(SID_WORLD);
    const QList<uint32_t> protect_deletion_mask_list = {
        SEC_STD_DELETE,
        SEC_ADS_DELETE_TREE,
    };
    for (const uint32_t &mask : protect_deletion_mask_list) {
        check_state(trustee_everyone, mask, QByteArray(), correct_type);
    }

    // Check using the getter
    const AdObject object = ad.search_object(test_user_dn);
    const bool actual_from_get = ad_security_get_protected_against_deletion(object);
    QCOMPARE(actual_from_get, enabled);
}

void ADMCTestAdSecurity::cant_change_pass_data() {
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<TestAdSecurityType>("correct_type");

    QTest::newRow("enabled") << true << TestAdSecurityType_Deny;
    QTest::newRow("disabled") << false << TestAdSecurityType_Allow;
}

void ADMCTestAdSecurity::cant_change_pass() {
    QFETCH(bool, enabled);
    QFETCH(TestAdSecurityType, correct_type);

    ad_security_set_user_cant_change_pass(&ad, test_user_dn, enabled);

    // NOTE: need to reload sd since this modifies it
    // on the server!
    load_sd();

    // Check state ourselves
    const QList<QString> cant_change_pass_trustee_cn_list = {
        SID_NT_SELF,
        SID_WORLD,
    };
    for (const QString &trustee_cn : cant_change_pass_trustee_cn_list) {
        const QByteArray trustee = sid_string_to_bytes(trustee_cn);
        const QByteArray change_pass_right = ad.adconfig()->get_right_guid("User-Change-Password");
        check_state(trustee, SEC_ADS_CONTROL_ACCESS, change_pass_right, correct_type);
    }

    // Check using the getter
    const AdObject object = ad.search_object(test_user_dn);
    const bool actual_from_get = ad_security_get_user_cant_change_pass(&object, ad.adconfig());
    QCOMPARE(actual_from_get, enabled);
}

// Setting a right should unset it's opposite. For
// example, set allow for X, then set deny for Y, allow
// for X should go away as a result.
void ADMCTestAdSecurity::add_to_unset_opposite_data() {
    QTest::addColumn<bool>("first_allow");
    QTest::addColumn<int>("access_mask");
    QTest::addColumn<QByteArray>("object_type");
    QTest::addColumn<TestAdSecurityType>("expected_result");

    const QByteArray allowed_to_auth_object_type = ad.adconfig()->get_right_guid("Allowed-To-Authenticate");

    QTest::newRow("allow [create child]") << true << SEC_ADS_CREATE_CHILD << QByteArray() << TestAdSecurityType_Deny;
    QTest::newRow("deny [create child]") << false << SEC_ADS_CREATE_CHILD << QByteArray() << TestAdSecurityType_Allow;
    QTest::newRow("allow [allowed to authenticate]") << true << SEC_ADS_CONTROL_ACCESS << allowed_to_auth_object_type << TestAdSecurityType_Deny;
    QTest::newRow("deny [allowed to authenticate]") << false << SEC_ADS_CONTROL_ACCESS << allowed_to_auth_object_type << TestAdSecurityType_Allow;
}

void ADMCTestAdSecurity::add_to_unset_opposite() {
    QFETCH(bool, first_allow);
    QFETCH(int, access_mask);
    QFETCH(QByteArray, object_type);
    QFETCH(TestAdSecurityType, expected_result);

    SecurityRight right{(uint32_t)access_mask, object_type, QByteArray(), 0};

    security_descriptor_add_right(sd, ad.adconfig(), class_list, test_trustee, right, first_allow);
    security_descriptor_add_right(sd, ad.adconfig(), class_list, test_trustee, right, !first_allow);

    check_state(test_trustee, access_mask, object_type, expected_result);
}

// When a right is unset, if it has any subordinates,
// they should become set. Logic here is that
// subordinates are "contained" by the superior's ACE
// but when superior is unset, they need their own
// ACE'S. For example, unsetting "generic read" should
// set all subordinate rights for reading properties.
void ADMCTestAdSecurity::remove_to_set_subordinates_data() {
    QTest::addColumn<bool>("allow_superior");
    QTest::addColumn<int>("superior_mask");
    QTest::addColumn<QByteArray>("subordinate_right_type");
    QTest::addColumn<QList<int>>("subordinate_mask_list");
    QTest::addColumn<TestAdSecurityType>("expected_state");

    const QByteArray type_web_info = ad.adconfig()->get_right_guid("Web-Information");
    const QByteArray type_change_password = ad.adconfig()->get_right_guid("User-Change-Password");

    QTest::newRow("allow full control") << true << SEC_ADS_GENERIC_ALL << type_web_info << QList<int>({SEC_ADS_WRITE_PROP, SEC_ADS_READ_PROP}) << TestAdSecurityType_Allow;
    QTest::newRow("deny full control") << false << SEC_ADS_GENERIC_ALL << type_web_info << QList<int>({SEC_ADS_WRITE_PROP, SEC_ADS_READ_PROP}) << TestAdSecurityType_Deny;
    QTest::newRow("allow generic read") << true << SEC_ADS_GENERIC_READ << type_web_info << QList<int>({SEC_ADS_READ_PROP}) << TestAdSecurityType_Allow;
    QTest::newRow("deny generic read") << false << SEC_ADS_GENERIC_READ << type_web_info << QList<int>({SEC_ADS_READ_PROP}) << TestAdSecurityType_Deny;
    QTest::newRow("allow generic write") << true << SEC_ADS_GENERIC_WRITE << type_web_info << QList<int>({SEC_ADS_WRITE_PROP}) << TestAdSecurityType_Allow;
    QTest::newRow("deny generic write") << false << SEC_ADS_GENERIC_WRITE << type_web_info << QList<int>({SEC_ADS_WRITE_PROP}) << TestAdSecurityType_Deny;
    QTest::newRow("allow all extended rights") << true << SEC_ADS_CONTROL_ACCESS << type_change_password << QList<int>({SEC_ADS_CONTROL_ACCESS}) << TestAdSecurityType_Allow;
    QTest::newRow("deny all extended rights") << false << SEC_ADS_CONTROL_ACCESS << type_change_password << QList<int>({SEC_ADS_CONTROL_ACCESS}) << TestAdSecurityType_Deny;
}

void ADMCTestAdSecurity::remove_to_set_subordinates() {
    QFETCH(bool, allow_superior);
    QFETCH(int, superior_mask);
    QFETCH(QByteArray, subordinate_right_type);
    QFETCH(QList<int>, subordinate_mask_list);
    QFETCH(TestAdSecurityType, expected_state);

    SecurityRight right{(uint32_t)superior_mask, QByteArray(), QByteArray(), 0};

    // Add superior
    security_descriptor_add_right(sd, ad.adconfig(), class_list, test_trustee, right, allow_superior);

    // Superior should be set
    check_state(test_trustee, superior_mask, QByteArray(), expected_state);

    // Subordinate should be set
    for (const int &subordinate_mask : subordinate_mask_list) {
        check_state(test_trustee, subordinate_mask, subordinate_right_type, expected_state);
    }

    // Remove superior
    security_descriptor_remove_right(sd, ad.adconfig(), class_list, test_trustee, right, allow_superior);

    // Superior should be unset
    check_state(test_trustee, superior_mask, QByteArray(), TestAdSecurityType_None);

    // But subordinates should be unset
    for (const int &subordinate_mask : subordinate_mask_list) {
        check_state(test_trustee, subordinate_mask, subordinate_right_type, TestAdSecurityType_None);
    }
}

// When a right is unset, if it has any superiors and
// they are set, they should become unset. For example,
// unsetting "allow read some property" should unset
// "generic read" and "full control".
void ADMCTestAdSecurity::remove_to_unset_superior_data() {
    QTest::addColumn<bool>("allow");
    QTest::addColumn<QList<int>>("superior_mask_list");
    QTest::addColumn<int>("subordinate_mask");
    QTest::addColumn<TestAdSecurityType>("state_before_remove");

    QTest::newRow("allow generic read") << true << QList<int>({SEC_ADS_GENERIC_READ}) << SEC_ADS_READ_PROP << TestAdSecurityType_Allow;
    QTest::newRow("deny generic read") << false << QList<int>({SEC_ADS_GENERIC_READ}) << SEC_ADS_READ_PROP << TestAdSecurityType_Deny;
    QTest::newRow("allow generic write") << true << QList<int>({SEC_ADS_GENERIC_WRITE}) << SEC_ADS_WRITE_PROP << TestAdSecurityType_Allow;
    QTest::newRow("deny generic write") << false << QList<int>({SEC_ADS_GENERIC_WRITE}) << SEC_ADS_WRITE_PROP << TestAdSecurityType_Deny;
    QTest::newRow("allow generic all and read") << true << QList<int>({SEC_ADS_GENERIC_ALL, SEC_ADS_GENERIC_READ}) << SEC_ADS_READ_PROP << TestAdSecurityType_Allow;
    QTest::newRow("deny generic all and read") << false << QList<int>({SEC_ADS_GENERIC_ALL, SEC_ADS_GENERIC_READ}) << SEC_ADS_READ_PROP << TestAdSecurityType_Deny;
}

void ADMCTestAdSecurity::remove_to_unset_superior() {
    QFETCH(bool, allow);
    QFETCH(QList<int>, superior_mask_list);
    QFETCH(int, subordinate_mask);
    QFETCH(TestAdSecurityType, state_before_remove);

    const QByteArray subordinate_right_type = ad.adconfig()->get_right_guid("Web-Information");

    // Add superiors
    // NOTE: there might be overlap but that's ok
    for (const int &superior_mask : superior_mask_list) {
        SecurityRight right{(uint32_t)superior_mask, QByteArray(), QByteArray(), 0};
        security_descriptor_add_right(sd, ad.adconfig(), class_list, test_trustee, right, allow);
    }

    // Superior should be set
    for (const int &superior_mask : superior_mask_list) {
        check_state(test_trustee, superior_mask, QByteArray(), state_before_remove);
    }

    // Subordinate should be set
    check_state(test_trustee, subordinate_mask, subordinate_right_type, state_before_remove);

    // Remove subordinate
    SecurityRight right{(uint32_t)subordinate_mask, subordinate_right_type, QByteArray(), 0};
    security_descriptor_remove_right(sd, ad.adconfig(), class_list, test_trustee, right, allow);

    // Superiors should be unset
    for (const int &superior_mask : superior_mask_list) {
        check_state(test_trustee, superior_mask, QByteArray(), TestAdSecurityType_None);
    }

    // And subordinates also should be unset
    check_state(test_trustee, subordinate_mask, subordinate_right_type, TestAdSecurityType_None);
}

// Setting a right should unset it's opposite superior.
// For example, "Create child" right is subordinate to
// "Full control", so if full control is allowed and
// then you deny "Create child", full control stops
// being allowed.
void ADMCTestAdSecurity::add_to_unset_opposite_superior_data() {
    QTest::addColumn<bool>("first_allow");
    QTest::addColumn<TestAdSecurityType>("expected_create_child");
    QTest::addColumn<TestAdSecurityType>("expected_full_control");

    QTest::newRow("allow full control, then deny create child") << true << TestAdSecurityType_Deny << TestAdSecurityType_None;
    QTest::newRow("deny full control, then allow create child") << false << TestAdSecurityType_Allow << TestAdSecurityType_None;
}

void ADMCTestAdSecurity::add_to_unset_opposite_superior() {
    QFETCH(bool, first_allow);
    QFETCH(TestAdSecurityType, expected_create_child);
    QFETCH(TestAdSecurityType, expected_full_control);

    SecurityRight right_generic{SEC_ADS_GENERIC_ALL, QByteArray(), QByteArray(), 0};
    SecurityRight right_create_child{SEC_ADS_CREATE_CHILD, QByteArray(), QByteArray(), 0};

    security_descriptor_add_right(sd, ad.adconfig(), class_list, test_trustee, right_generic, first_allow);
    security_descriptor_add_right(sd, ad.adconfig(), class_list, test_trustee, right_create_child, !first_allow);

    check_state(test_trustee, SEC_ADS_CREATE_CHILD, QByteArray(), expected_create_child);
    check_state(test_trustee, SEC_ADS_GENERIC_ALL, QByteArray(), expected_full_control);
}

void ADMCTestAdSecurity::check_state(const QByteArray &trustee, const uint32_t access_mask, const QByteArray &object_type, const TestAdSecurityType type) const {
    SecurityRight right_generic{access_mask, object_type, QByteArray(), 0};
    const SecurityRightState state = security_descriptor_get_right_state(sd, trustee, right_generic);

    const bool inherited_allow = state.get(SecurityRightStateInherited_Yes, SecurityRightStateType_Allow);
    const bool inherited_deny = state.get(SecurityRightStateInherited_Yes, SecurityRightStateType_Deny);
    const bool object_allow = state.get(SecurityRightStateInherited_No, SecurityRightStateType_Allow);
    const bool object_deny = state.get(SecurityRightStateInherited_No, SecurityRightStateType_Deny);

    QCOMPARE(inherited_allow, false);
    QCOMPARE(inherited_deny, false);

    switch (type) {
        case TestAdSecurityType_Allow: {
            QCOMPARE(object_allow, true);
            QCOMPARE(object_deny, false);

            break;
        }
        case TestAdSecurityType_Deny: {
            QCOMPARE(object_allow, false);
            QCOMPARE(object_deny, true);

            break;
        }
        case TestAdSecurityType_None: {
            QCOMPARE(object_allow, false);
            QCOMPARE(object_deny, false);

            break;
        }
    }
}

void ADMCTestAdSecurity::load_sd() {
    security_descriptor_free(sd);

    sd = [&]() {
        const AdObject test_user = ad.search_object(test_user_dn);
        security_descriptor *out = test_user.get_security_descriptor();

        return out;
    }();
}

QTEST_MAIN(ADMCTestAdSecurity)
