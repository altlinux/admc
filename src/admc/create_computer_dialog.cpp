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

#include "create_computer_dialog.h"
#include "ui_create_computer_dialog.h"

#include "adldap.h"
#include "edits/sam_name_edit.h"
#include "edits/upn_edit.h"
#include "utils.h"
#include "settings.h"

CreateComputerDialog::CreateComputerDialog(QWidget *parent)
: CreateObjectDialog(parent) {
    ui = new Ui::CreateComputerDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    QList<AttributeEdit *> edit_list;

    // TODO: "Assign this computer account as a pre-Windows 2000 computer". Is this needed?

    // TODO: "The following user or group may join this computer to a domain". Tried to figure out how this is implemented and couldn't see any easy ways via attributes, so probably something to do with setting ACL'S.

    // TODO: "This is a managed computer" checkbox and an edit for guid/uuid which I assume modifies objectGUID?

    sam_name_edit = new SamNameEdit(ui->sam_name_edit, ui->sam_name_domain_edit, &edit_list, this);

    const QList<QLineEdit *> required_list = {
        ui->name_edit,
        ui->sam_name_edit,
    };

    const QList<QWidget *> widget_list = {
        ui->name_edit,
        ui->sam_name_edit,
        // NOTE: not restoring sam domain state is intended
        // ui->sam_name_domain_edit,
    };

    // Autofill name -> sam account name
    connect(
        ui->name_edit, &QLineEdit::textChanged,
        this, &CreateComputerDialog::autofill_sam_name);

    init(ui->name_edit, ui->button_box, edit_list, required_list, widget_list, CLASS_COMPUTER);

    settings_setup_dialog_geometry(SETTING_create_computer_dialog_geometry, this);
}

CreateComputerDialog::~CreateComputerDialog() {
    delete ui;
}

void CreateComputerDialog::open() {
    sam_name_edit->load_domain();

    CreateObjectDialog::open();
}

void CreateComputerDialog::autofill_sam_name() {
    const QString name_input = ui->name_edit->text();
    ui->sam_name_edit->setText(name_input.toUpper());
}
