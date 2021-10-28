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

#include "create_group_dialog.h"
#include "ui_create_group_dialog.h"

#include "adldap.h"
#include "utils.h"
#include "edits/string_edit.h"
#include "edits/sam_name_edit.h"
#include "edits/upn_edit.h"
#include "edits/group_scope_edit.h"
#include "edits/group_type_edit.h"

CreateGroupDialog::CreateGroupDialog(QWidget *parent)
: CreateObjectDialog(parent) {
    ui = new Ui::CreateGroupDialog();
    ui->setupUi(this);

    QList<AttributeEdit *> edit_list;
    sam_name_edit = new SamNameEdit(ui->sam_name_edit, ui->sam_name_domain_edit, &edit_list, this);
    new GroupScopeEdit(ui->scope_combo, &edit_list, this);
    new GroupTypeEdit(ui->type_combo, &edit_list, this);

    const QList<QLineEdit *> required_edits = {
        ui->sam_name_edit,
    };

    const QList<QWidget *> widget_list = {
        ui->name_edit,
        ui->sam_name_edit,
        ui->scope_combo,
        ui->type_combo,
    };

    init(ui->name_edit, ui->button_box, edit_list, required_edits, widget_list, CLASS_GROUP);
}

CreateGroupDialog::~CreateGroupDialog() {
    delete ui;
}

void CreateGroupDialog::open() {
    sam_name_edit->load_domain();

    CreateObjectDialog::open();
}
