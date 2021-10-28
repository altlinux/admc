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

#include "create_ou_dialog.h"
#include "ui_create_ou_dialog.h"

#include "adldap.h"
#include "utils.h"
#include "edits/string_edit.h"
#include "edits/upn_edit.h"
#include "edits/protect_deletion_edit.h"

CreateOUDialog::CreateOUDialog(QWidget *parent)
: CreateObjectDialog(parent) {
    ui = new Ui::CreateOUDialog();
    ui->setupUi(this);

    QList<AttributeEdit *> edit_list;
    new ProtectDeletionEdit(ui->deletion_check, &edit_list, this);

    const QList<QLineEdit *> required_list = {
        ui->name_edit,
    };

    const QList<QWidget *> widget_list = {
        ui->name_edit,
        ui->deletion_check,
    };

    init(ui->name_edit, ui->button_box, edit_list, required_list, widget_list, CLASS_OU);
}

CreateOUDialog::~CreateOUDialog() {
    delete ui;
}
