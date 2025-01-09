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

#include "create_group_dialog.h"
#include "ui_create_group_dialog.h"

#include "adldap.h"
#include "attribute_edits/group_scope_edit.h"
#include "attribute_edits/group_type_edit.h"
#include "attribute_edits/sam_name_edit.h"
#include "attribute_edits/string_edit.h"
#include "attribute_edits/upn_edit.h"
#include "create_object_helper.h"
#include "settings.h"
#include "utils.h"

CreateGroupDialog::CreateGroupDialog(const QString &parent_dn, QWidget *parent)
: CreateObjectDialog(parent) {
    ui = new Ui::CreateGroupDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    auto sam_name_edit = new SamNameEdit(ui->sam_name_edit, ui->sam_name_domain_edit, this);
    auto scope_edit = new GroupScopeEdit(ui->scope_combo, this);
    auto type_edit = new GroupTypeEdit(ui->type_combo, this);

    const QList<AttributeEdit *> edit_list = {
        sam_name_edit,
        scope_edit,
        type_edit,
    };

    const QList<QLineEdit *> required_edits = {
        ui->sam_name_edit,
    };

    helper = new CreateObjectHelper(ui->name_edit, ui->button_box, edit_list, required_edits, CLASS_GROUP, parent_dn, this);

    settings_setup_dialog_geometry(SETTING_create_group_dialog_geometry, this);

    // name -> sam account name
    connect(
        ui->name_edit, &QLineEdit::textChanged,
        this, &CreateGroupDialog::autofill_sam_name);
}

CreateGroupDialog::~CreateGroupDialog() {
    delete ui;
}

void CreateGroupDialog::accept() {
    const bool accepted = helper->accept();

    if (accepted) {
        QDialog::accept();
    }
}

QString CreateGroupDialog::get_created_dn() const {
    return helper->get_created_dn();
}

void CreateGroupDialog::autofill_sam_name() {
    const QString name_input = ui->name_edit->text();
    ui->sam_name_edit->setText(name_input);
}
