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

#ifndef ADMC_TEST_LOGON_COMPUTERS_H
#define ADMC_TEST_LOGON_COMPUTERS_H

#include "admc_test.h"

class LogonComputersEdit;
class LogonComputersDialog;
class QPushButton;
class QListWidget;

class ADMCTestLogonComputers : public ADMCTest {
    Q_OBJECT

private slots:
    void init() override;

    void load();
    void emit_edited_signal();
    void add();
    void remove();
    void apply();

private:
    LogonComputersEdit *edit;
    LogonComputersDialog *dialog;
    QPushButton *open_dialog_button;
    QListWidget *list;
    QLineEdit *value_edit;
    QPushButton *add_button;
    QPushButton *remove_button;
    QString dn;

    void test_list_item(const int row, const QString &text);
};

#endif /* ADMC_TEST_LOGON_COMPUTERS_H */
