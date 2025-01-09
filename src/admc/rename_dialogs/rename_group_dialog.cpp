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

#include "rename_group_dialog.h"
#include "ui_rename_group_dialog.h"

#include "ad_defines.h"
#include "attribute_edits/sam_name_edit.h"
#include "rename_object_helper.h"
#include "settings.h"
#include "utils.h"

RenameGroupDialog::RenameGroupDialog(AdInterface &ad, const QString &target_arg, QWidget *parent)
: RenameObjectDialog(parent) {
    ui = new Ui::RenameGroupDialog();
    ui->setupUi(this);

    auto sam_name_edit = new SamNameEdit(ui->sam_name_edit, ui->sam_name_domain_edit, this);

    const QList<AttributeEdit *> edit_list = {
        sam_name_edit,
    };

    QList<QLineEdit *> requred_list = {
        ui->name_edit,
        ui->sam_name_edit,
    };

    helper = new RenameObjectHelper(ad, target_arg, ui->name_edit, edit_list, this, requred_list, ui->button_box);
    setup_lineedit_autofill(ui->name_edit, ui->sam_name_edit);

    settings_setup_dialog_geometry(SETTING_rename_group_dialog_geometry, this);
}

void RenameGroupDialog::accept() {
    const bool accepted = helper->accept();

    if (accepted) {
        QDialog::accept();
    }
}

QString RenameGroupDialog::get_new_dn() const {
    return helper->get_new_dn();
}

RenameGroupDialog::~RenameGroupDialog() {
    delete ui;
}
