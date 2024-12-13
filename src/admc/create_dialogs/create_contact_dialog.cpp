/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2022 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
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

#include "create_contact_dialog.h"
#include "ui_create_contact_dialog.h"

#include "adldap.h"
#include "attribute_edits/string_edit.h"
#include "create_object_helper.h"
#include "settings.h"
#include "utils.h"

CreateContactDialog::CreateContactDialog(const QString &parent_dn, QWidget *parent)
: CreateObjectDialog(parent) {
    ui = new Ui::CreateContactDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    bool show_middle_name = false;
    const QVariant show_middle_name_variant = settings_get_variant(SETTING_show_middle_name_when_creating);
    if (!show_middle_name_variant.isNull()) {
        show_middle_name = show_middle_name_variant.toBool();
    }
    ui->middle_name_edit->setVisible(show_middle_name);
    ui->middle_name_label->setVisible(show_middle_name);

    auto first_name_edit = new StringEdit(ui->first_name_edit, ATTRIBUTE_FIRST_NAME, this);
    auto last_name_edit = new StringEdit(ui->last_name_edit, ATTRIBUTE_LAST_NAME, this);
    auto initials_edit = new StringEdit(ui->initials_edit, ATTRIBUTE_INITIALS, this);
    auto display_name_edit = new StringEdit(ui->display_name_edit, ATTRIBUTE_DISPLAY_NAME, this);
    auto middle_name_edit = new StringEdit(ui->middle_name_edit, ATTRIBUTE_MIDDLE_NAME, this);

    const QList<AttributeEdit *> edit_list = {
        first_name_edit,
        last_name_edit,
        initials_edit,
        display_name_edit,
        middle_name_edit,
    };

    const QList<QLineEdit *> required_list = {
        ui->first_name_edit,
        ui->last_name_edit,
        ui->full_name_edit,
        ui->display_name_edit,
    };

    setup_full_name_autofill(ui->first_name_edit, ui->last_name_edit, ui->middle_name_edit, ui->full_name_edit);

    helper = new CreateObjectHelper(ui->full_name_edit, ui->button_box, edit_list, required_list, CLASS_CONTACT, parent_dn, this);

    settings_setup_dialog_geometry(SETTING_create_contact_dialog_geometry, this);
}

CreateContactDialog::~CreateContactDialog() {
    delete ui;
}

void CreateContactDialog::accept() {
    const bool accepted = helper->accept();

    if (accepted) {
        QDialog::accept();
    }
}

QString CreateContactDialog::get_created_dn() const {
    return helper->get_created_dn();
}
