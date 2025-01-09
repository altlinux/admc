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

#include "create_shared_folder_dialog.h"
#include "ui_create_shared_folder_dialog.h"

#include "adldap.h"
#include "attribute_edits/protect_deletion_edit.h"
#include "attribute_edits/string_edit.h"
#include "attribute_edits/upn_edit.h"
#include "create_object_helper.h"
#include "settings.h"
#include "utils.h"

CreateSharedFolderDialog::CreateSharedFolderDialog(const QString &parent_dn, QWidget *parent)
: CreateObjectDialog(parent) {
    ui = new Ui::CreateSharedFolderDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    auto path_edit = new StringEdit(ui->path_edit, ATTRIBUTE_UNC_NAME, this);

    const QList<AttributeEdit *> edit_list = {
        path_edit,
    };

    const QList<QLineEdit *> required_list = {
        ui->name_edit,
        ui->path_edit,
    };

    helper = new CreateObjectHelper(ui->name_edit, ui->button_box, edit_list, required_list, CLASS_SHARED_FOLDER, parent_dn, this);

    settings_setup_dialog_geometry(SETTING_create_shared_folder_dialog_geometry, this);
}

CreateSharedFolderDialog::~CreateSharedFolderDialog() {
    delete ui;
}

void CreateSharedFolderDialog::accept() {
    const bool accepted = helper->accept();

    if (accepted) {
        QDialog::accept();
    }
}

QString CreateSharedFolderDialog::get_created_dn() const {
    return helper->get_created_dn();
}
