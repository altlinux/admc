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

#include "admc_test_security_tab.h"

#include "samba/ndr_security.h"
#include "tabs/security_tab.h"

#include <QComboBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>

void ADMCTestSecurityTab::init() {
    ADMCTest::init();

    security_tab = new SecurityTab();
    add_widget(security_tab);

    // Create test user
    const QString name = TEST_USER;
    test_user_dn = test_object_dn(name, CLASS_USER);
    const bool create_success = ad.object_add(test_user_dn, CLASS_USER);
    QVERIFY(create_success);

    const AdObject object = ad.search_object(test_user_dn);

    security_tab->load(ad, object);
}

// NOTE: just checking that the default security descriptor
// is laoded correctly. Creating custom security descriptors
// is too complicated at the moment.
void ADMCTestSecurityTab::load() {
    auto test_allowed = [&](const QString &trustee_name, const QSet<AcePermission> allowed_set) {
        QVERIFY(security_tab->set_trustee(trustee_name));

        const QSet<AcePermission> none_set = all_permissions - allowed_set;
        QVERIFY2(state_is(allowed_set, PermissionState_Allowed), qPrintable(trustee_name));
        QVERIFY2(state_is(none_set, PermissionState_None), qPrintable(trustee_name));
    };

    test_allowed("Account Operators", all_permissions);

    test_allowed("Administrators", [&]() {
        QSet<AcePermission> out = all_permissions;
        out -= AcePermission_FullControl;
        out -= AcePermission_DeleteChild;

        return out;
    }());

    test_allowed("Authenticated Users", [&]() {
        QSet<AcePermission> out;
        out += AcePermission_ReadGeneralInfo;
        out += AcePermission_ReadPersonalInfo;
        out += AcePermission_ReadPublicInfo;
        out += AcePermission_ReadWebInfo;

        return out;
    }());

    test_allowed("Cert Publishers", {});

    test_allowed("Domain Admins", all_permissions);

    test_allowed("ENTERPRISE DOMAIN CONTROLLERS", {});

    test_allowed("Enterprise Admins", all_permissions);

    test_allowed("Everyone", [&]() {
        QSet<AcePermission> out;
        out += AcePermission_ChangePassword;

        return out;
    }());

    test_allowed("Pre-Windows 2000 Compatible Access", [&]() {
        QSet<AcePermission> out;
        out += AcePermission_ReadAccountRestrictions;
        out += AcePermission_ReadGeneralInfo;
        out += AcePermission_ReadGroupMembership;
        out += AcePermission_ReadLogonInfo;
        out += AcePermission_ReadRemoteAccessInfo;

        return out;
    }());

    test_allowed("RAS and IAS Servers", [&]() {
        QSet<AcePermission> out;
        out += AcePermission_ReadAccountRestrictions;
        out += AcePermission_ReadGroupMembership;
        out += AcePermission_ReadLogonInfo;
        out += AcePermission_ReadRemoteAccessInfo;

        return out;
    }());

    test_allowed("SELF", [&]() {
        QSet<AcePermission> out;
        out += AcePermission_Read;
        out += read_prop_permissions;
        out += AcePermission_ChangePassword;
        out += AcePermission_ReceiveAs;
        out += AcePermission_SendAs;
        out += AcePermission_WritePersonalInfo;
        out += AcePermission_WritePhoneAndMailOptions;
        out += AcePermission_WritePrivateInfo;
        out += AcePermission_WriteWebInfo;

        return out;
    }());

    test_allowed("SYSTEM", all_permissions);

    test_allowed("Terminal Server License Servers", [&]() {
        QSet<AcePermission> out;
        out += AcePermission_ReadTerminalServerLicenseServer;
        out += AcePermission_WriteTerminalServerLicenseServer;

        return out;
    }());

    test_allowed("Windows Authorization Access Group", {});
}

// When you allow some perm then deny it, the allow checkbox
// should become unchecked, aka they are exclusive.
void ADMCTestSecurityTab::allow_then_deny() {
    uncheck_all_permissions();

    // NOTE: permission doesn't matter, just picked some random one
    const AcePermission permission = AcePermission_SendAs;
    set_permission_state({permission}, AceColumn_Allowed, Qt::Checked);
    QVERIFY(state_is({permission}, PermissionState_Allowed));

    set_permission_state({permission}, AceColumn_Denied, Qt::Checked);
    QVERIFY(state_is({permission}, PermissionState_Denied));
}

// Allowing full should allow every permission
void ADMCTestSecurityTab::allow_full() {
    uncheck_all_permissions();

    QVERIFY(state_is(all_permissions, PermissionState_None));

    set_permission_state({AcePermission_FullControl}, AceColumn_Allowed, Qt::Checked);

    QVERIFY(state_is(all_permissions, PermissionState_Allowed));
}

// Allowing full and denying read, should allow everything
// except read permissions which should be denied.
void ADMCTestSecurityTab::allow_full_deny_read() {
    uncheck_all_permissions();

    set_permission_state({AcePermission_FullControl}, AceColumn_Allowed, Qt::Checked);
    set_permission_state({AcePermission_Read}, AceColumn_Denied, Qt::Checked);

    QVERIFY(state_is({AcePermission_FullControl}, PermissionState_None));
    QVERIFY(state_is(access_permissions, PermissionState_Allowed));
    QVERIFY(state_is(write_prop_permissions, PermissionState_Allowed));

    QVERIFY(state_is({AcePermission_Read}, PermissionState_Denied));
    QVERIFY(state_is(read_prop_permissions, PermissionState_Denied));
}

// Unchecking read while full is allowed, should uncheck
// full and nothing else.
void ADMCTestSecurityTab::allow_full_uncheck_read() {
    uncheck_all_permissions();

    set_permission_state({AcePermission_FullControl}, AceColumn_Allowed, Qt::Checked);
    set_permission_state({AcePermission_Read}, AceColumn_Allowed, Qt::Unchecked);

    QVERIFY(state_is({AcePermission_FullControl}, PermissionState_None));
    QVERIFY(state_is(access_permissions, PermissionState_Allowed));
    QVERIFY(state_is(write_prop_permissions, PermissionState_Allowed));
    QVERIFY(state_is(read_prop_permissions, PermissionState_Allowed));

    QVERIFY(state_is({AcePermission_Read}, PermissionState_None));
}

// Unchecking a read prop while read is allowed, should
// uncheck read and nothing else.
void ADMCTestSecurityTab::allow_read_uncheck_read_prop() {
    uncheck_all_permissions();

    set_permission_state({AcePermission_Read}, AceColumn_Allowed, Qt::Checked);
    set_permission_state({AcePermission_ReadWebInfo}, AceColumn_Allowed, Qt::Unchecked);

    QVERIFY(state_is({AcePermission_Read}, PermissionState_None));

    QVERIFY(state_is((read_prop_permissions - QSet<AcePermission>{AcePermission_ReadWebInfo}), PermissionState_Allowed));
}

// Denying a read prop while read is allowed, should
// uncheck read and deny that permission.
void ADMCTestSecurityTab::allow_read_deny_read_prop() {
    uncheck_all_permissions();

    set_permission_state({AcePermission_Read}, AceColumn_Allowed, Qt::Checked);
    set_permission_state({AcePermission_ReadWebInfo}, AceColumn_Denied, Qt::Checked);

    QVERIFY(state_is({AcePermission_Read}, PermissionState_None));
    QVERIFY(state_is({AcePermission_ReadWebInfo}, PermissionState_Denied));

    QVERIFY(state_is((read_prop_permissions - QSet<AcePermission>{AcePermission_ReadWebInfo}), PermissionState_Allowed));
}

void ADMCTestSecurityTab::apply() {
    QVERIFY(security_tab->set_trustee("Cert Publishers"));

    // Check/uncheck some permissions in the tab
    const QSet<AcePermission> allowed_set = {
        AcePermission_CreateChild,
        AcePermission_ReadPersonalInfo,
    };
    const QSet<AcePermission> denied_set = {
        AcePermission_WriteWebInfo,
        AcePermission_WritePersonalInfo,
        AcePermission_DeleteChild,
    };
    const QSet<AcePermission> none_set = all_permissions - allowed_set - denied_set;
    uncheck_all_permissions();
    set_permission_state(allowed_set, AceColumn_Allowed, Qt::Checked);
    set_permission_state(denied_set, AceColumn_Denied, Qt::Checked);

    // Apply
    const bool apply_success = security_tab->apply(ad, test_user_dn);
    QVERIFY(apply_success);

    // Reload tab
    const AdObject updated_object = ad.search_object(test_user_dn);
    security_tab->load(ad, updated_object);

    // Verify that state loaded correctly
    QVERIFY(security_tab->set_trustee("Cert Publishers"));
    QVERIFY(state_is(allowed_set, PermissionState_Allowed));
    QVERIFY(state_is(denied_set, PermissionState_Denied));
    QVERIFY(state_is(none_set, PermissionState_None));
}

void ADMCTestSecurityTab::uncheck_all_permissions() {
    set_permission_state(all_permissions, AceColumn_Allowed, Qt::Unchecked);
    set_permission_state(all_permissions, AceColumn_Denied, Qt::Unchecked);
}

bool ADMCTestSecurityTab::state_is(const QSet<AcePermission> &permission_set, const PermissionState state) const {
    const QSet<AceColumn> &checked_columns = [&]() -> QSet<AceColumn> {
        switch (state) {
            case PermissionState_Allowed: return {AceColumn_Allowed};
            case PermissionState_Denied: return {AceColumn_Denied};
            case PermissionState_None: return {};
        }
        return {};
    }();

    for (const AcePermission &permission : permission_set) {
        const QList<AceColumn> column_list = {
            AceColumn_Allowed,
            AceColumn_Denied,
        };

        for (const AceColumn &column : column_list) {
            QStandardItem *item = security_tab->get_item(permission, column);

            const bool should_be_checked = checked_columns.contains(column);
            const bool is_checked = (item->checkState() == Qt::Checked);
            const bool state_is_correct = (is_checked == should_be_checked);

            if (!state_is_correct) {
                const QString permission_name = ace_permission_to_name_map[permission];
                const QString column_name = [&]() {
                    switch (column) {
                        case AceColumn_Allowed: return "Allowed";
                        case AceColumn_Denied: return "Denied";
                        default: break;
                    }

                    return "unknown";
                }();

                qInfo().noquote() << QString("Incorrect state:\n\tpermission = %1\n\tcolumn = %2\n\tcurrent state = %3\n\tcorrect state = %4").arg(permission_name, column_name, QString::number(is_checked), QString::number(should_be_checked));

                return false;
            }
        }
    }

    return true;
}

void ADMCTestSecurityTab::set_permission_state(const QSet<AcePermission> &permission_set, const AceColumn column, const Qt::CheckState state) {
    for (const AcePermission &permission : permission_set) {
        QStandardItem *item = security_tab->get_item(permission, column);
        item->setCheckState(state);
    }
}

QTEST_MAIN(ADMCTestSecurityTab)
