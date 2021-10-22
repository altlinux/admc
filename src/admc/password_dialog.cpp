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

#include "password_dialog.h"
#include "ui_password_dialog.h"

#include "adldap.h"
#include "edits/account_option_edit.h"
#include "edits/password_edit.h"
#include "edits/unlock_edit.h"
#include "globals.h"
#include "status.h"
#include "utils.h"

PasswordDialog::PasswordDialog(const QString &target_arg, QWidget *parent)
: QDialog(parent) {
    ui = new Ui::PasswordDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    target = target_arg;

    AdInterface ad;
    if (ad_failed(ad)) {
        close();
        return;
    }

    const AdObject object = ad.search_object(target);

    new PasswordEdit(ui->password_main_edit, ui->password_confirm_edit, &edits, this);

    auto pass_expired_edit = new AccountOptionEdit(ui->expired_check, AccountOption_PasswordExpired, &edits, this);

    new UnlockEdit(ui->unlock_check, &edits, this);

    edits_load(edits, ad, object);

    const bool expired_check_enabled = [&]() {
        const bool dont_expire_pass = object.get_account_option(AccountOption_DontExpirePassword, g_adconfig);
        const bool cant_change_pass = object.get_account_option(AccountOption_CantChangePassword, g_adconfig);
        const bool out = !(dont_expire_pass || cant_change_pass);

        return out;
    }();
    
    if (expired_check_enabled) {
        ui->expired_check->setChecked(true);

        // NOTE: always set expired option to modified, so that
        // it always applies, even if this option is already
        // turned on. This is for consistent and understandable
        // messaging to user.
        pass_expired_edit->set_modified(true);
    } else {
        ui->expired_check->setEnabled(false);
        ui->expired_check->setToolTip(tr("Option is unavailable because a conflicting account option is currently enabled."));
    }

    g_status()->display_ad_messages(ad, this);
}

PasswordDialog::~PasswordDialog() {
    delete ui;
}

void PasswordDialog::accept() {
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    show_busy_indicator();

    const bool verify_success = edits_verify(ad, edits, target);
    if (!verify_success) {
        return;
    }

    const bool apply_success = edits_apply(ad, edits, target);

    hide_busy_indicator();

    g_status()->display_ad_messages(ad, this);

    if (apply_success) {
        QDialog::accept();
    }
}
