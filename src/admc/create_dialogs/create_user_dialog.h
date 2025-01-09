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

#ifndef CREATE_USER_DIALOG_H
#define CREATE_USER_DIALOG_H

#include "create_object_dialog.h"

class AdInterface;
class CreateObjectHelper;

namespace Ui {
class CreateUserDialog;
}

class CreateUserDialog final : public CreateObjectDialog {
    Q_OBJECT

public:
    Ui::CreateUserDialog *ui;

    // NOTE: user_class can be either CLASS_USER or
    // CLASS_INET_ORG_PERSON. This is so that this
    // dialog can be reused for both classes.
    CreateUserDialog(AdInterface &ad, const QString &parent_dn, const QString &user_class, QWidget *parent);
    ~CreateUserDialog();

    void accept() override;
    QString get_created_dn() const override;

private:
    CreateObjectHelper *helper;

    void autofill_full_name();
};

#endif /* CREATE_USER_DIALOG_H */
