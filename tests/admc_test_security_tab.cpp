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

void set_checked(QStandardItem *item, const bool checked);
bool is_checked(QStandardItem *item);

void ADMCTestSecurityTab::initTestCase() {
    ADMCTest::initTestCase();

    all_permissions =
    []() {
        QSet<AcePermission> out;

        for (int permission_i = 0; permission_i < AcePermission_COUNT; permission_i++) {
            const AcePermission permission = (AcePermission) permission_i;
            out.insert(permission);
        }

        return out;
    }();

    for (const AcePermission &permission : all_permissions) {
        const uint32_t mask = ace_permission_to_mask_map[permission];

        switch (mask) {
            case SEC_ADS_CONTROL_ACCESS: {
                access_permissions.insert(permission);

                break;
            }
            case SEC_ADS_READ_PROP: {
                read_prop_permissions.insert(permission);

                break;
            }
            case SEC_ADS_WRITE_PROP: {
                write_prop_permissions.insert(permission);

                break;
            }
            default: break;
        }
    }
}

void ADMCTestSecurityTab::init() {
    ADMCTest::init();

    security_tab = new SecurityTab();
    ace_model = security_tab->get_ace_model();

    permission_item_map =
    [&]() {
        QHash<AcePermission, QHash<AceColumn, QStandardItem *>> out;

        for (int row = 0; row < ace_model->rowCount(); row++) {
            QStandardItem *main_item = ace_model->item(row, 0);
            const AcePermission permission = (AcePermission) main_item->data(AcePermissionItemRole_Permission).toInt();

            const QList<AceColumn> column_list = {
                AceColumn_Allowed,
                AceColumn_Denied,
            };

            for (const AceColumn &column : column_list) {
                QStandardItem *item = ace_model->item(row, column);
                out[permission][column] = item;
            }
        }

        return out;
    }();

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

// When you allow some perm then deny it, the allow checkbox
// should become unchecked, aka they are exclusive.
void ADMCTestSecurityTab::allow_then_deny() {
    uncheck_all_permissions();

    // NOTE: permission doesn't matter, just picked some random one
    const AcePermission permission = AcePermission_SendAs;

    set_state(permission, AceColumn_Allowed, true);

    QVERIFY(state_is({permission}, {AceColumn_Allowed}));

    set_state(permission, AceColumn_Denied, true);
    QVERIFY(state_is({permission}, {AceColumn_Denied}));
}

// Allowing full should allow every permission
void ADMCTestSecurityTab::allow_full() {
    uncheck_all_permissions();

    set_state(AcePermission_FullControl, AceColumn_Allowed, true);

    QVERIFY(state_is({all_permissions}, {AceColumn_Allowed}));
}

// Allowing full and denying read, should allow everything
// except read permissions which should be denied.
void ADMCTestSecurityTab::allow_full_deny_read() {
    uncheck_all_permissions();

    set_state(AcePermission_FullControl, AceColumn_Allowed, true);
    set_state(AcePermission_Read, AceColumn_Denied, true);

    QVERIFY(state_is({AcePermission_FullControl}, {}));
    QVERIFY(state_is(access_permissions, {AceColumn_Allowed}));
    QVERIFY(state_is(write_prop_permissions, {AceColumn_Allowed}));
    
    QVERIFY(state_is({AcePermission_Read}, {AceColumn_Denied}));
    QVERIFY(state_is(read_prop_permissions, {AceColumn_Denied}));
}

// Unchecking read while full is allowed, should uncheck
// full and nothing else.
void ADMCTestSecurityTab::allow_full_uncheck_read() {
    uncheck_all_permissions();

    set_state(AcePermission_FullControl, AceColumn_Allowed, true);
    set_state(AcePermission_Read, AceColumn_Allowed, false);

    QVERIFY(state_is({AcePermission_FullControl}, {}));
    QVERIFY(state_is(access_permissions, {AceColumn_Allowed}));
    QVERIFY(state_is(write_prop_permissions, {AceColumn_Allowed}));
    QVERIFY(state_is(read_prop_permissions, {AceColumn_Allowed}));
    
    QVERIFY(state_is({AcePermission_Read}, {}));
}

// Unchecking a read prop while read is allowed, should
// uncheck read and nothing else.
void ADMCTestSecurityTab::allow_read_uncheck_read_prop() {
    uncheck_all_permissions();

    set_state(AcePermission_Read, AceColumn_Allowed, true);
    set_state(AcePermission_ReadWebInfo, AceColumn_Allowed, false);

    QVERIFY(state_is({AcePermission_Read}, {}));

    QVERIFY(state_is((read_prop_permissions - QSet<AcePermission>{AcePermission_ReadWebInfo}), {AceColumn_Allowed}));
}

// Denying a read prop while read is allowed, should
// uncheck read and deny that permission.
void ADMCTestSecurityTab::allow_read_deny_read_prop() {
    uncheck_all_permissions();

    set_state(AcePermission_Read, AceColumn_Allowed, true);
    set_state(AcePermission_ReadWebInfo, AceColumn_Denied, true);

    QVERIFY(state_is({AcePermission_Read}, {}));
    QVERIFY(state_is({AcePermission_ReadWebInfo}, {AceColumn_Denied}));

    QVERIFY(state_is((read_prop_permissions - QSet<AcePermission>{AcePermission_ReadWebInfo}), {AceColumn_Allowed}));
}

void ADMCTestSecurityTab::uncheck_all_permissions() {
    for (const AcePermission &permission : all_permissions) {
        QStandardItem *allowed = permission_item_map[permission][AceColumn_Allowed];
        QStandardItem *denied = permission_item_map[permission][AceColumn_Denied];

        set_checked(allowed, false);
        set_checked(denied, false);
    }
}

void set_checked(QStandardItem *item, const bool checked) {
    const Qt::CheckState state =
    [&]() {
        if (checked) {
            return Qt::Checked;
        } else {
            return Qt::Unchecked;
        }
    }();

    item->setCheckState(state);
}

bool is_checked(QStandardItem *item) {
    return (item->checkState() == Qt::Checked);
}

bool ADMCTestSecurityTab::state_is(const QSet<AcePermission> &permission_list, const QSet<AceColumn> &checked_columns) const {
    for (const AcePermission &permission : permission_list) {
        const QList<AceColumn> column_list = {
            AceColumn_Allowed,
            AceColumn_Denied,
        };

        for (const AceColumn &column : column_list) {
            QStandardItem *item = permission_item_map[permission][column];

            const bool should_be_checked = checked_columns.contains(column);
            const bool state_is_correct = (is_checked(item) == should_be_checked);

            if (!state_is_correct) {
                return false;
            }
        }
    }

    return true;
}

void ADMCTestSecurityTab::set_state(const AcePermission permission, const AceColumn column, const bool checked) {
    QStandardItem *item = permission_item_map[permission][column];

    set_checked(item, checked);
}

QTEST_MAIN(ADMCTestSecurityTab)
