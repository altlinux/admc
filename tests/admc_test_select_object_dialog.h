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

#ifndef ADMC_TEST_SELECT_OBJECT_DIALOG_H
#define ADMC_TEST_SELECT_OBJECT_DIALOG_H

#include "admc_test.h"

class QLineEdit;
class QPushButton;
class SelectObjectDialog;

class ADMCTestSelectObjectDialog : public ADMCTest {
    Q_OBJECT

private slots:
    void init() override;

    void empty();
    void no_matches();
    void one_match();
    void multiple_matches();
    void one_match_duplicate();
    void multiple_match_duplicate();

private:
    SelectObjectDialog *dialog;
    QLineEdit *edit;
    QPushButton *add_button;

    void select_object_in_multi_match_dialog(const QString &name, const QString &dn);
};

#endif /* ADMC_TEST_SELECT_OBJECT_DIALOG_H */
