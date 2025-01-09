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

#ifndef ADMC_TEST_AD_INTERFACE_H
#define ADMC_TEST_AD_INTERFACE_H

#include "admc_test.h"

class ADMCTestAdInterface : public ADMCTest {
    Q_OBJECT

private slots:
    void cleanup() override;

    void create_and_gpo_delete();
    void gpo_check_perms();

    void object_add();
    void object_delete();
    void object_move();
    void object_rename();

    void group_add_member();
    void group_remove_member();
    void group_set_scope();
    void group_set_type();

    void user_set_account_option();

private:
};

#endif /* ADMC_TEST_AD_INTERFACE_H */
