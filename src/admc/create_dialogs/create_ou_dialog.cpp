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

#include "create_ou_dialog.h"
#include "ui_create_ou_dialog.h"

#include "adldap.h"
#include "attribute_edits/protect_deletion_edit.h"
#include "attribute_edits/string_edit.h"
#include "attribute_edits/upn_edit.h"
#include "create_object_helper.h"
#include "settings.h"
#include "utils.h"

CreateOUDialog::CreateOUDialog(const QString &parent_dn, QWidget *parent)
: CreateObjectDialog(parent) {
    ui = new Ui::CreateOUDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    auto deletion_edit = new ProtectDeletionEdit(ui->deletion_check, this);

    const QList<AttributeEdit *> edit_list = {
        deletion_edit,
    };

    const QList<QLineEdit *> required_list = {
        ui->name_edit,
    };

    helper = new CreateObjectHelper(ui->name_edit, ui->button_box, edit_list, required_list, CLASS_OU, parent_dn, this);

    settings_setup_dialog_geometry(SETTING_create_ou_dialog_geometry, this);
}

CreateOUDialog::~CreateOUDialog() {
    delete ui;
}

void CreateOUDialog::accept() {
    const bool accepted = helper->accept();

    if (accepted) {
        QDialog::accept();
    }
}

QString CreateOUDialog::get_created_dn() const {
    return helper->get_created_dn();
}
