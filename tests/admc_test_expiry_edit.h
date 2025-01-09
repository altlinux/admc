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

#ifndef ADMC_TEST_EXPIRY_EDIT_H
#define ADMC_TEST_EXPIRY_EDIT_H

#include "admc_test.h"

class ExpiryEdit;
class QRadioButton;
class QDateEdit;

class ADMCTestExpiryEdit : public ADMCTest {
    Q_OBJECT

private slots:
    void initTestCase_data();
    void init() override;

    void edited_signal();
    void load();
    void apply_unmodified();
    void apply();

private:
    ExpiryEdit *edit;
    QString dn;
    QRadioButton *button;
    QDateEdit *date_edit;
};

#endif /* ADMC_TEST_EXPIRY_EDIT_H */
