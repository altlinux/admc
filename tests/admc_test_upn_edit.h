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

#ifndef ADMC_TEST_UPN_EDIT_H
#define ADMC_TEST_UPN_EDIT_H

#include "admc_test.h"

class UpnEdit;
class QLineEdit;
class QComboBox;

class ADMCTestUpnEdit : public ADMCTest {
    Q_OBJECT

private slots:
    void init() override;

    void length_limit();
    void test_load();
    void test_emit_edited();
    void apply_unmodified();
    void test_apply_suffix();
    void test_apply_prefix();
    void test_apply_prefix_and_suffix();
    void test_reset();
    void verify_bad_chars_data();
    void verify_bad_chars();
    void verify_conflict();

private:
    UpnEdit *upn_edit;
    QLineEdit *prefix_edit;
    QComboBox *suffix_edit;
    QString dn;

    QString get_upn();
    QString get_current_upn();
    bool edit_state_equals_to_server_state();
    void change_suffix_in_edit();
};

#endif /* ADMC_TEST_UPN_EDIT_H */
