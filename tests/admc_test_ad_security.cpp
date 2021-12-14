/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
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

#include "admc_test_ad_security.h"

#include "samba/ndr_security.h"
#include "ad_security.h"

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

    sd = NULL;
    load_sd();
}

void ADMCTestAdSecurity::cleanup() {
    ADMCTest::cleanup();

    security_descriptor_free(sd);
}

void ADMCTestAdSecurity::add_right() {
    security_descriptor_add_right(sd, test_trustee, SEC_ADS_CREATE_CHILD, QByteArray(), true);

    check_state(test_trustee, SEC_ADS_CREATE_CHILD, QByteArray(), TestAdSecurityType_Allow);
}

void ADMCTestAdSecurity::remove_right() {
    security_descriptor_add_right(sd, test_trustee, SEC_ADS_CREATE_CHILD, QByteArray(), true);
    check_state(test_trustee, SEC_ADS_CREATE_CHILD, QByteArray(), TestAdSecurityType_Allow);
    
    security_descriptor_remove_right(sd, test_trustee, SEC_ADS_CREATE_CHILD, QByteArray(), true);
    check_state(test_trustee, SEC_ADS_CREATE_CHILD, QByteArray(), TestAdSecurityType_None);
}

void ADMCTestAdSecurity::remove_trustee() {
    const QList<uint32_t> mask_list = {
        SEC_ADS_CREATE_CHILD,
        SEC_STD_DELETE,
    };

    for (const uint32_t &mask : mask_list) {
        security_descriptor_add_right(sd, test_trustee, mask, QByteArray(), true);
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
        security_descriptor_add_right(sd, test_trustee, mask, QByteArray(), true);
        security_descriptor_add_right(sd, test_trustee, opposite, QByteArray(), true);

        check_state(test_trustee, mask, QByteArray(), TestAdSecurityType_Allow);
        check_state(test_trustee, opposite, QByteArray(), TestAdSecurityType_Allow);

        security_descriptor_remove_right(sd, test_trustee, mask, QByteArray(), true);

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

void ADMCTestAdSecurity::check_state(const QByteArray &trustee, const uint32_t access_mask, const QByteArray &object_type, const TestAdSecurityType type) const {
    const SecurityRightState state = security_descriptor_get_right(sd, trustee, access_mask, object_type);

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
    if (sd != NULL) {
        security_descriptor_free(sd);
    }

    sd = [&]() {
        const AdObject test_user = ad.search_object(test_user_dn);
        security_descriptor *out = test_user.get_security_descriptor();

        return out;
    }();
}

QTEST_MAIN(ADMCTestAdSecurity)
