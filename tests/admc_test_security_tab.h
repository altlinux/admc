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

#ifndef ADMC_TEST_SECURITY_TAB_H
#define ADMC_TEST_SECURITY_TAB_H

#include "admc_test.h"

#include "tabs/security_tab.h"

class SecurityTab;
class QStandardItemModel;

class ADMCTestSecurityTab : public ADMCTest {
Q_OBJECT

private slots:
    void initTestCase() override;
    void init() override;

    void allow_then_deny();
    void allow_full();
    void allow_full_deny_read();
    void allow_full_uncheck_read();
    void allow_read_uncheck_read_prop();
    void allow_read_deny_read_prop();

private:
    SecurityTab *security_tab;
    QStandardItemModel *ace_model;
    QHash<AcePermission, QHash<AceColumn, QStandardItem *>> permission_item_map;
    QSet<AcePermission> all_permissions;
    QSet<AcePermission> access_permissions;
    QSet<AcePermission> read_prop_permissions;
    QSet<AcePermission> write_prop_permissions;

    void uncheck_all_permissions();
    bool state_is(const QSet<AcePermission> &permission_list, const QSet<AceColumn> &checked_columns) const;
    void set_state(const AcePermission permission, const AceColumn column, const bool checked);
};

#endif /* ADMC_TEST_SECURITY_TAB_H */
