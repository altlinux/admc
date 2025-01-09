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

#ifndef ADMC_TEST_AD_SECURITY_H
#define ADMC_TEST_AD_SECURITY_H

#include "admc_test.h"

struct security_descriptor;

enum TestAdSecurityType {
    TestAdSecurityType_Allow,
    TestAdSecurityType_Deny,
    TestAdSecurityType_None,
};
Q_DECLARE_METATYPE(TestAdSecurityType)

class ADMCTestAdSecurity : public ADMCTest {
    Q_OBJECT

private slots:
    void initTestCase() override;
    void init() override;
    void cleanup() override;

    void add_right();
    void remove_right();
    void remove_trustee();
    void handle_generic_read_and_write_sharing_bit();
    void protected_against_deletion_data();
    void protected_against_deletion();
    void cant_change_pass_data();
    void cant_change_pass();
    void add_to_unset_opposite_data();
    void add_to_unset_opposite();
    void remove_to_set_subordinates_data();
    void remove_to_set_subordinates();
    void remove_to_unset_superior_data();
    void remove_to_unset_superior();
    void add_to_unset_opposite_superior_data();
    void add_to_unset_opposite_superior();

private:
    QString test_user_dn;
    QString test_trustee_dn;
    QByteArray test_trustee;
    security_descriptor *sd;
    const QList<QString> class_list = {CLASS_USER};

    void check_state(const QByteArray &trustee, const uint32_t access_mask, const QByteArray &object_type, const TestAdSecurityType type) const;
    void load_sd();
};

#endif /* ADMC_TEST_AD_SECURITY_H */
