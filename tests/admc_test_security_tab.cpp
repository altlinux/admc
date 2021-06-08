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

#include "tabs/security_tab.h"
#include "samba/ndr_security.h"

#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QTreeView>
#include <QStandardItemModel>

void ADMCTestSecurityTab::init() {
    ADMCTest::init();

    security_tab = new SecurityTab();

    // Create test user
    const QString name = TEST_USER;
    const QString dn = test_object_dn(name, CLASS_USER);
    const bool create_success = ad.object_add(dn, CLASS_USER);
    QVERIFY(create_success);

    const QList<QString> attribute_list =
    [&]() {
        QList<QString> out;

        const AdObject object = ad.search_object(dn);
        out.append(object.attributes());
        out.append(ATTRIBUTE_SECURITY_DESCRIPTOR);

        return out;
    }();

    const AdObject object = ad.search_object(dn, attribute_list);

    security_tab->load(ad, object);

    auto layout = new QVBoxLayout();
    parent_widget->setLayout(layout);
    layout->addWidget(security_tab);

    parent_widget->show();
    QVERIFY(QTest::qWaitForWindowExposed(parent_widget, 1000));
}

// NOTE: just checking that the default security descriptor
// is laoded correctly. Creating custom security descriptors
// is too complicated at the moment.
void ADMCTestSecurityTab::load() {
    QVERIFY(security_tab->set_trustee("Account Operators"));
    QVERIFY(state_is(all_permissions, PermissionState_Allowed));

    const QSet<AcePermission> administrators_allowed =
    [&]() {
        QSet<AcePermission> out = all_permissions;
        out -= AcePermission_FullControl;
        out -= AcePermission_DeleteChild;

        return out;
    }();
    const QSet<AcePermission> administrators_none = all_permissions - administrators_allowed;

    QVERIFY(security_tab->set_trustee("Administrators"));
    QVERIFY(state_is(administrators_allowed, PermissionState_Allowed));
    QVERIFY(state_is(administrators_none, PermissionState_None));
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

    QVERIFY(state_is({all_permissions}, PermissionState_None));
    
    set_permission_state({AcePermission_FullControl}, AceColumn_Allowed, Qt::Checked);

    QVERIFY(state_is({all_permissions}, PermissionState_Allowed));
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

void ADMCTestSecurityTab::uncheck_all_permissions() {
    set_permission_state(all_permissions, AceColumn_Allowed, Qt::Unchecked);
    set_permission_state(all_permissions, AceColumn_Denied, Qt::Unchecked);
}

bool ADMCTestSecurityTab::state_is(const QSet<AcePermission> &permission_set, const PermissionState state) const {
    const QSet<AceColumn> &checked_columns =
    [&]() -> QSet<AceColumn> {
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
                const QString column_name =
                [&]() {
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
