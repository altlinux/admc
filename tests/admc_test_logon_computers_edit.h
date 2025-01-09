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

#ifndef ADMC_TEST_LOGON_COMPUTERS_EDIT_H
#define ADMC_TEST_LOGON_COMPUTERS_EDIT_H

#include "admc_test.h"

class LogonComputersEdit;
class LogonComputersDialog;
class QPushButton;
class QListWidget;

class ADMCTestLogonComputersEdit : public ADMCTest {
    Q_OBJECT

private slots:
    void init() override;

    void load_empty();
    void load();
    void emit_edited_signal();
    void add();
    void remove();
    void apply_unmodified();
    void apply();

private:
    LogonComputersEdit *edit;
    LogonComputersDialog *dialog;
    QListWidget *list;
    QLineEdit *value_edit;
    QPushButton *open_dialog_button;
    QPushButton *add_button;
    QPushButton *remove_button;
    QString dn;

    void test_list_item(const int row, const QString &text);
    void load_and_open_dialog();
    void open_dialog();
};

#endif /* ADMC_TEST_LOGON_COMPUTERS_EDIT_H */
