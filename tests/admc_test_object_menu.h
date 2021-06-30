/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
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

#ifndef ADMC_TEST_OBJECT_MENU_H
#define ADMC_TEST_OBJECT_MENU_H

#include "admc_test.h"

class ADMCTestObjectMenu : public ADMCTest {
    Q_OBJECT

private slots:
    void object_menu_new_user();
    void object_menu_new_ou();
    void object_menu_new_computer();
    void object_menu_new_group();

    void object_menu_move();
    void object_menu_rename();
    void object_menu_reset_password();
    void object_menu_disable_enable_account();

    void object_menu_find_simple();
    void object_menu_find_advanced();

private:

};

#endif /* ADMC_TEST_OBJECT_MENU_H */
