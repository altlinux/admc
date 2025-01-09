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

#ifndef ADMC_TEST_MEMBERS_TAB_H
#define ADMC_TEST_MEMBERS_TAB_H

#include "admc_test.h"

class QTreeView;
class AttributeEdit;
class QStandardItemModel;
class QPushButton;

class ADMCTestMembersTab : public ADMCTest {
    Q_OBJECT

private slots:
    void init() override;

    void load_empty();
    void load();
    void remove();
    void add();

private:
    AttributeEdit *edit;
    QTreeView *view;
    QStandardItemModel *model;
    QString user_dn;
    QString group_dn;
    QPushButton *add_button;
    QPushButton *remove_button;
};

#endif /* ADMC_TEST_MEMBERS_TAB_H */
