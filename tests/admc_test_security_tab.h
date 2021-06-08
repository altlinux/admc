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

class ADMCTestSecurityTab : public ADMCTest {
Q_OBJECT

private slots:
    void init() override;

    void load();
    void allow_then_deny();
    void allow_full();
    void allow_full_deny_read();
    void allow_full_uncheck_read();
    void allow_read_uncheck_read_prop();
    void allow_read_deny_read_prop();

private:
    SecurityTab *security_tab;

    void uncheck_all_permissions();
    void set_permission_state(const QSet<AcePermission> &permission_set, const AceColumn column, const Qt::CheckState state);
    bool state_is(const QSet<AcePermission> &permission_set, const PermissionState state) const;
};

#endif /* ADMC_TEST_SECURITY_TAB_H */
