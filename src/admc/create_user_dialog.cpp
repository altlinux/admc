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

#include "create_user_dialog.h"
#include "ui_create_user_dialog.h"

#include "adldap.h"
#include "attribute_edits/account_option_edit.h"
#include "attribute_edits/password_edit.h"
#include "attribute_edits/sam_name_edit.h"
#include "attribute_edits/string_edit.h"
#include "attribute_edits/upn_edit.h"
#include "settings.h"
#include "utils.h"
#include "create_object_helper.h"

CreateUserDialog::CreateUserDialog(AdInterface &ad, const QString &parent_dn, QWidget *parent)
: CreateObjectDialog(parent) {
    ui = new Ui::CreateUserDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    auto first_name_edit = new StringEdit(ui->first_name_edit, ATTRIBUTE_FIRST_NAME, this);
    auto last_name_edit = new StringEdit(ui->last_name_edit, ATTRIBUTE_LAST_NAME, this);
    auto initials_edit = new StringEdit(ui->initials_edit, ATTRIBUTE_INITIALS, this);
    auto sam_name_edit = new SamNameEdit(ui->sam_name_edit, ui->sam_name_domain_edit, CLASS_USER, this);
    auto password_edit = new PasswordEdit(ui->password_main_edit, ui->password_confirm_edit, this);

    auto upn_edit = new UpnEdit(ui->upn_prefix_edit, ui->upn_suffix_edit, this);
    upn_edit->init_suffixes(ad);

    const QHash<AccountOption, QCheckBox *> check_map = {
        {AccountOption_PasswordExpired, ui->must_change_pass_check},
        {AccountOption_CantChangePassword, ui->cant_change_pass_check},
        {AccountOption_DontExpirePassword, ui->dont_expire_pass_check},
        {AccountOption_Disabled, ui->disabled_check},
    };

    QList<AttributeEdit *> option_edit_list;

    for (const AccountOption &option : check_map.keys()) {
        QCheckBox *check = check_map[option];
        auto edit = new AccountOptionEdit(check, option, this);

        option_edit_list.append(edit);
    }

    account_option_setup_conflicts(check_map);

    // (first name + last name) -> full name
    connect(
        ui->first_name_edit, &QLineEdit::textChanged,
        this, &CreateUserDialog::autofill_full_name);
    connect(
        ui->last_name_edit, &QLineEdit::textChanged,
        this, &CreateUserDialog::autofill_full_name);

    // upn -> sam account name
    connect(
        ui->upn_prefix_edit, &QLineEdit::textChanged,
        this, &CreateUserDialog::autofill_sam_name);

    const QList<QLineEdit *> required_list = {
        ui->name_edit,
        ui->first_name_edit,
        ui->sam_name_edit,
    };

    const QList<AttributeEdit *> edit_list = [&]() {
        QList<AttributeEdit *> out;

        out = {
            first_name_edit,
            last_name_edit,
            initials_edit,
            sam_name_edit,
            password_edit,
            upn_edit,
        };

        out.append(option_edit_list);

        return out;
    }();

    helper = new CreateObjectHelper(ui->name_edit, ui->button_box, edit_list, required_list, CLASS_USER, parent_dn, this);

    settings_setup_dialog_geometry(SETTING_create_user_dialog_geometry, this);
}

CreateUserDialog::~CreateUserDialog() {
    delete ui;
}

void CreateUserDialog::accept() {
    const bool accepted = helper->accept();

    if (accepted) {
        QDialog::accept();
    }
}

QString CreateUserDialog::get_created_dn() const {
    return helper->get_created_dn();
}

void CreateUserDialog::autofill_full_name() {
    const QString full_name_value = [=]() {
        const QString first_name = ui->first_name_edit->text();
        const QString last_name = ui->last_name_edit->text();

        const bool last_name_first = settings_get_variant(SETTING_last_name_before_first_name).toBool();
        if (!first_name.isEmpty() && !last_name.isEmpty()) {
            if (last_name_first) {
                return last_name + " " + first_name;
            } else {
                return first_name + " " + last_name;
            }
        } else if (!first_name.isEmpty()) {
            return first_name;
        } else if (!last_name.isEmpty()) {
            return last_name;
        } else {
            return QString();
        }
    }();

    ui->name_edit->setText(full_name_value);
}

void CreateUserDialog::autofill_sam_name() {
    const QString upn_input = ui->upn_prefix_edit->text();
    ui->sam_name_edit->setText(upn_input);
}