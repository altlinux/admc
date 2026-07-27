/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
 * Copyright (C) 2026 Artyom V. Poptsov
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

#include "password_dialog.h"
#include "ui_password_dialog.h"

#include "adldap.h"
#include "attribute_edits/account_option_edit.h"
#include "attribute_edits/password_edit.h"
#include "attribute_edits/unlock_edit.h"
#include "core/globals.h"
#include "core/settings.h"
#include "status.h"
#include "utils.h"

#include <QPushButton>

PasswordDialog::PasswordDialog(AdInterface &ad, const QString &target_arg, QWidget *parent)
: QDialog(parent) {
    ui = new Ui::PasswordDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    auto password_main_edit = new PasswordEdit(ui->password_main_edit, ui->password_confirm_edit, ui->show_password_check, this);

    pass_expired_edit = new AccountOptionEdit(ui->expired_check, AccountOption_PasswordExpired, this);

    auto unlock_edit = new UnlockEdit(ui->unlock_check, this);

    target = target_arg;

    edits = {
        password_main_edit,
        pass_expired_edit,
        unlock_edit,
    };

    const AdObject object = ad.search_object(target);

    AttributeEdit::load(edits, ad, object);


    const bool dont_expire_pass =
        object.get_account_option(AccountOption_DontExpirePassword,
                                  g_adconfig);
    const bool cant_change_pass =
        object.get_account_option(AccountOption_CantChangePassword,
                                  g_adconfig);
    const bool expired_check_enabled = !(dont_expire_pass || cant_change_pass);

    if (expired_check_enabled) {
        ui->expired_check->setChecked(true);
    } else {
        ui->expired_check->setEnabled(false);
        ui->expired_check->setToolTip(tr("Option is unavailable because a conflicting account option is currently enabled."));
    }

    required_list = {
        ui->password_main_edit,
        ui->password_confirm_edit,
    };

    for (QLineEdit *edit : required_list) {
        connect(
            edit, &QLineEdit::textChanged,
            this, &PasswordDialog::on_edited);
    }
    on_edited();

    settings_setup_dialog_geometry(SETTING_password_dialog_geometry, this);
}

PasswordDialog::~PasswordDialog() {
    delete ui;
}

void PasswordDialog::accept() {
    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    show_busy_indicator();

    const bool verify_success = AttributeEdit::verify(edits, ad, target);
    if (!verify_success) {
        return;
    }

    const bool apply_success = AttributeEdit::apply(edits, ad, target);

    hide_busy_indicator();

    g_status->display_ad_messages(ad, this);

    if (apply_success) {
        QDialog::accept();
    }
}

void PasswordDialog::on_edited() {
    bool all_required_filled = true;
    for (QLineEdit *edit : required_list) {
        if (edit->text().isEmpty()) {
            all_required_filled = false;
            break;
        }
    }

    auto ok_button = ui->button_box->button(QDialogButtonBox::Ok);
    ok_button->setEnabled(all_required_filled);
}
