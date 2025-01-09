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

#include "rename_user_dialog.h"
#include "ui_rename_user_dialog.h"

#include "adldap.h"
#include "attribute_edits/sam_name_edit.h"
#include "attribute_edits/string_edit.h"
#include "attribute_edits/upn_edit.h"
#include "rename_object_helper.h"
#include "settings.h"
#include "utils.h"

RenameUserDialog::RenameUserDialog(AdInterface &ad, const QString &target_arg, QWidget *parent)
: RenameObjectDialog(parent) {
    ui = new Ui::RenameUserDialog();
    ui->setupUi(this);

    auto first_name_edit = new StringEdit(ui->first_name_edit, ATTRIBUTE_FIRST_NAME, this);
    auto last_name_edit = new StringEdit(ui->last_name_edit, ATTRIBUTE_LAST_NAME, this);
    auto display_name_edit = new StringEdit(ui->full_name_edit, ATTRIBUTE_DISPLAY_NAME, this);

    auto upn_edit = new UpnEdit(ui->upn_prefix_edit, ui->upn_suffix_edit, this);
    upn_edit->init_suffixes(ad);

    auto sam_name_edit = new SamNameEdit(ui->sam_name_edit, ui->sam_name_domain_edit, this);

    const QList<AttributeEdit *> edit_list = {
        first_name_edit,
        last_name_edit,
        display_name_edit,
        upn_edit,
        sam_name_edit,
    };

    const QList<QLineEdit *> required_list = {
        ui->name_edit,
        ui->upn_prefix_edit,
        ui->sam_name_edit,
    };

    helper = new RenameObjectHelper(ad, target_arg, ui->name_edit, edit_list, this, required_list, ui->button_box);

    setup_lineedit_autofill(ui->upn_prefix_edit, ui->sam_name_edit);

    settings_setup_dialog_geometry(SETTING_rename_user_dialog_geometry, this);
}

void RenameUserDialog::accept() {
    const bool accepted = helper->accept();

    if (accepted) {
        QDialog::accept();
    }
}

QString RenameUserDialog::get_new_dn() const {
    return helper->get_new_dn();
}

RenameUserDialog::~RenameUserDialog() {
    delete ui;
}
