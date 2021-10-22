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
#include "utils.h"
#include "settings.h"
#include "edits/string_edit.h"
#include "edits/sam_name_edit.h"
#include "edits/upn_edit.h"
#include "edits/password_edit.h"
#include "edits/account_option_edit.h"

CreateUserDialog::CreateUserDialog(QWidget *parent)
: CreateDialog(parent) {
    ui = new Ui::CreateUserDialog();
    ui->setupUi(this);

    QList<AttributeEdit *> edit_list;
    new StringEdit(ui->first_name_edit, ATTRIBUTE_FIRST_NAME, &edit_list, this);
    new StringEdit(ui->last_name_edit, ATTRIBUTE_LAST_NAME, &edit_list, this);
    new StringEdit(ui->initials_edit, ATTRIBUTE_INITIALS, &edit_list, this);
    sam_name_edit = new SamNameEdit(ui->sam_name_edit, ui->sam_name_domain_edit, &edit_list, this);

    new PasswordEdit(ui->password_main_edit, ui->password_confirm_edit, &edit_list, this);

    upn_edit = new UpnEdit(ui->upn_prefix_edit, ui->upn_suffix_edit, &edit_list, this);

    const QHash<AccountOption, QCheckBox *> check_map = {
        {AccountOption_PasswordExpired, ui->must_change_pass_check},
        {AccountOption_CantChangePassword, ui->cant_change_pass_check},
        {AccountOption_DontExpirePassword, ui->dont_expire_pass_check},
        {AccountOption_Disabled, ui->disabled_check},
    };

    for (const AccountOption &option : check_map.keys()) {
        QCheckBox *check = check_map[option];
        new AccountOptionEdit(check, option, &edit_list, this);
    }

    account_option_setup_conflicts(check_map);

    // Setup autofills
    // (first name + last name) -> full name
    auto autofill_full_name = [=]() {
        const QString full_name_value = [=]() {
            const QString first_name = ui->first_name_edit->text();
            const QString last_name = ui->last_name_edit->text();

            const bool last_name_first = settings_get_bool(SETTING_last_name_before_first_name);
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
    };
    connect(
        ui->first_name_edit, &QLineEdit::textChanged,
        autofill_full_name);
    connect(
        ui->last_name_edit, &QLineEdit::textChanged,
        autofill_full_name);

    // upn -> sam account name
    connect(
        ui->upn_prefix_edit, &QLineEdit::textChanged,
        [=]() {
            const QString upn_input = ui->upn_prefix_edit->text();
            ui->sam_name_edit->setText(upn_input);
        });

    const QList<QLineEdit *> required_list = {
        ui->name_edit,
        ui->first_name_edit,
        ui->sam_name_edit,
    };

    const QList<QWidget *> widget_list = {
        ui->name_edit,
        ui->first_name_edit,
        ui->last_name_edit,
        ui->initials_edit,
        ui->sam_name_edit,
        // NOTE: not restoring sam account name domain state
        // is intended
        // ui->sam_name_domain_edit,
        ui->password_confirm_edit,
        ui->upn_prefix_edit,
        // NOTE: not restoring upn suffix state is intended
        // ui->upn_suffix_edit,
        ui->must_change_pass_check,
        ui->cant_change_pass_check,
        ui->dont_expire_pass_check,
        ui->disabled_check,
    };

    init(ui->name_edit, ui->button_box, edit_list, required_list, widget_list,  CLASS_USER);
}

CreateUserDialog::~CreateUserDialog() {
    delete ui;
}

void CreateUserDialog::open() {
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    upn_edit->init_suffixes(ad);
    sam_name_edit->load_domain();

    CreateDialog::open();
}
