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

#ifndef CREATE_USER_DIALOG_H
#define CREATE_USER_DIALOG_H

#include "create_dialog.h"

class SamNameEdit;
class UpnEdit;

namespace Ui {
    class CreateUserDialog;
}

class CreateUserDialog final : public CreateDialog {
    Q_OBJECT

public:
    CreateUserDialog(QWidget *parent);

    void open() override;

private:
    Ui::CreateUserDialog *ui;
    SamNameEdit *sam_name_edit;
    UpnEdit *upn_edit;
};

#endif /* CREATE_USER_DIALOG_H */
