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

#ifndef RENAME_USER_DIALOG_H
#define RENAME_USER_DIALOG_H

#include "rename_dialog.h"

class UpnEdit;
class SamNameEdit;

namespace Ui {
    class RenameUserDialog;
}

class RenameUserDialog final : public RenameDialog {
    Q_OBJECT

public:
    Ui::RenameUserDialog *ui;

    RenameUserDialog(QWidget *parent);
    ~RenameUserDialog();

public slots:
    void open() override;

private:
    UpnEdit *upn_edit;
    SamNameEdit *sam_name_edit;
};

#endif /* RENAME_USER_DIALOG_H */
