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

#ifndef ADMC_TEST_GPLINK_H
#define ADMC_TEST_GPLINK_H

#include <QObject>
#include <QTest>

class ADMCTestGplink : public QObject {
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void to_string();
    void equals();
    void contains_data();
    void contains();
    void add_data();
    void add();
    void remove_data();
    void remove();
    void move_up_data();
    void move_up();
    void move_down_data();
    void move_down();
    void get_option_data();
    void get_option();
    void set_option_data();
    void set_option();
    void get_gpo_list_data();
    void get_gpo_list();
    void get_gpo_order_data();
    void get_gpo_order();
};

#endif /* ADMC_TEST_GPLINK_H */
