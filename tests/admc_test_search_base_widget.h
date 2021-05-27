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

#ifndef ADMC_TEST_SEARCH_BASE_WIDGET_H
#define ADMC_TEST_SEARCH_BASE_WIDGET_H

#include "admc_test.h"

class QComboBox;
class QPushButton;
class SelectBaseWidget;

class ADMCTestSelectBaseWidget : public ADMCTest {
Q_OBJECT

private slots:
    void init() override;

    void default_to_domain_head();
    void select_base();
    void select_base_multiple();
    void save_state();

private:
    SelectBaseWidget *select_base_widget;
    QComboBox *combo;
    QPushButton *browse_button;
    QList<QString> dn_list;

    void add_base(const QString &dn);
};

#endif /* ADMC_TEST_SEARCH_BASE_WIDGET_H */
