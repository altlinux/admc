/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2026 BaseALT Ltd.
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

#include <QDebug>
#include <stdexcept>

#include "core/settings.h"
#include "krb5client.h"
#include "ui/dialog/auth/krb_auth.h"
#include "ui/dialog/auth/ui_krb_auth.h"
#include "ui/utils.h"

enum {
    ROW_NEW_PASSWORD = 4,
    ROW_PASSWORD_CONFIRMATION = 5
};

KrbAuthDialog::KrbAuthDialog(QWidget *parent, Krb5Client *krb_client_arg)
    : AuthDialogBase(parent), ui(new Ui::KrbAuthDialog),
      state(KRB5_DIALOG_STATE_AUTHENTICATION),
      client(krb_client_arg)
{
    ui->setupUi(this);
    switch_ui_to_authentication();
    setupWidgets();
}

KrbAuthDialog::~KrbAuthDialog() {
    delete ui;
}

void KrbAuthDialog::logout(bool delete_creds) {
    if (! delete_creds) {
        return;
    }

    const int index =
        ui->principal_cmb_box->findText(client->current_principal());
    ui->principal_cmb_box->removeItem(index);
    ui->principal_cmb_box->clearEditText();
}

void KrbAuthDialog::setupWidgets() {
    ui->error_label->setHidden(true);
    ui->error_label->setStyleSheet("color: red");
    ui->error_label->setReadOnly(true);

    ui->ticket_available_label->setVisible(false);

    ui->principal_cmb_box->addItems(client->available_principals());

    connect(ui->principal_cmb_box, &QComboBox::currentTextChanged,
            this, &KrbAuthDialog::on_principal_selected);
    connect(ui->show_passwd_checkbox, &QCheckBox::toggled,
            this, &KrbAuthDialog::on_show_passwd);
    connect(ui->sign_in_button, &QPushButton::clicked,
            this, &KrbAuthDialog::on_sign_in);
    connect(ui->system_cache_chkbox, &QCheckBox::toggled,
            this, &KrbAuthDialog::on_use_system_credentials);
    connect(ui->remember_checkbox, &QCheckBox::toggled,
            this, &KrbAuthDialog::remember_principal);

    bool use_system_creds =
        (! settings_get_variant(SETTING_use_system_credentials).isNull())
        && settings_get_bool(SETTING_use_system_credentials);
    ui->system_cache_chkbox->setChecked(use_system_creds);
    on_use_system_credentials(use_system_creds);

    const QString curr_principal = client->current_principal();
    bool remember_checked = ((! client->system_principal().isEmpty())
                             && (curr_principal == client->system_principal()))
        || settings_are_creds_saved(curr_principal);
    ui->remember_checkbox->setChecked(remember_checked);
    ui->remember_checkbox->setDisabled(
        curr_principal == client->system_principal());

    ui->principal_cmb_box->setFocus();

    adjustSize();
}

/**
 * Switch the UI to the password change state.
 */
void KrbAuthDialog::switch_ui_to_password_change() {
    ui->formLayout->setRowVisible(ROW_NEW_PASSWORD, true);
    ui->formLayout->setRowVisible(ROW_PASSWORD_CONFIRMATION, true);
    ui->password_new_edit->setText("");
    ui->password_confirm_edit->setText("");

    state = KRB5_DIALOG_STATE_PASSWORD_CHANGE;
}

/**
 * Switch the UI to the password authentication state.
 */
void KrbAuthDialog::switch_ui_to_authentication() {
    ui->formLayout->setRowVisible(ROW_NEW_PASSWORD, false);
    ui->formLayout->setRowVisible(ROW_PASSWORD_CONFIRMATION, false);

    ui->password_edit->setText("");

    state = KRB5_DIALOG_STATE_AUTHENTICATION;
}

/**
 * Verify if a given password is valid.
 *
 * @param pass A new password string.
 * @param pass_confirm A new password confirmation string.
 * @retrun true if passwords match and are valid, false otherwise.
 */
bool KrbAuthDialog::verify_password(const QString &pass,
                                           const QString &pass_confirm) {
    if (pass.isEmpty()) {
        show_error_message(QString(tr("Password cannot be empty.")));
        return false;
    }

    if (pass != pass_confirm) {
        show_error_message(QString(tr("Passwords do not match.")));
        return false;
    }

    const bool can_encode = pass.isValidUtf16();
    if (! can_encode) {
        show_error_message(
            QString(tr("Password contains invalid characters.")));
        return false;
    }

    return true;
}

/**
 * Change the principal password.
 *
 * @param principal A principal name.
 * @return true if password was successfully changed, false otherwise.
 */
bool KrbAuthDialog::change_password(const QString &principal) {
    const QString PASS = ui->password_new_edit->text();
    const QString PASS_CONFIRMATION = ui->password_confirm_edit->text();

    if (! verify_password(PASS, PASS_CONFIRMATION)) {
        return false;
    }

    try {
        client->change_password(
            principal,
            ui->password_edit->text(),
            ui->password_confirm_edit->text(),
            enterprise);
    } catch (KerberosError &error) {
        krb5_error_code result = error.get_error_code();
        show_error_message(tr("Failed to update password:")
                           + error.what());
        if (result == KRB5_KPASSWD_SUCCESS) {
            show_error_message(tr("Password rejected"));
        }
        return false;
    }
    return true;
}

void KrbAuthDialog::authenticate(const QString &principal) {
    if (principal.isEmpty()) {
        show_error_message(tr("Enter your Kerberos principal"));
        return;
    }

    if (principal == client->current_principal()) {
        show_error_message(tr("Account already in use"));
        return;
    }

    if (ui->password_edit->isVisible() && ui->password_edit->text().isEmpty()) {
        show_error_message(tr("Enter the password"));
        return;
    }

    if (! client->current_principal().isEmpty()) {
        bool delete_creds =
            (! settings_are_creds_saved(client->current_principal()));
        logout(delete_creds);
        client->logout(delete_creds);
    }

    bool ticket_active =
        (client->tgt_data(principal).state == Krb5TgtState_Active);
    try {
        if (client->principal_has_cache(principal) && ticket_active) {
            client->set_current_principal(principal);
        }
        else {
            QString error_message;
            try {
                client->authenticate(principal, ui->password_edit->text());
            } catch (KerberosError &first_error) {
                const krb5_error_code rc = first_error.get_error_code();
                error_message += first_error.what() + QString("\n");
                if (rc == KRB5KDC_ERR_KEY_EXP) {
                    show_error_message(tr("Password expired."));
                    switch_ui_to_password_change();
                    enterprise = false;
                    return;
                } else {
                    try {
                        client->authenticate(principal, ui->password_edit->text(),
                                             true);
                    } catch (KerberosError &second_error) {
                        const krb5_error_code rc = second_error.get_error_code();
                        error_message += second_error.what() + QString("\n");
                        if (rc == KRB5KDC_ERR_KEY_EXP) {
                            switch_ui_to_password_change();
                            enterprise = true;
                            return;
                        } else {
                            throw std::runtime_error(error_message.toStdString());
                        }
                    }
                }
            }
            ui->principal_cmb_box->addItem(principal);
        }
    }
    catch (const std::runtime_error& e) {
        show_error_message(e.what());
        return;
    }

    settings_set_variant(SETTING_last_logged_user, principal);
    remember_principal(ui->remember_checkbox->checkState() == Qt::Checked);

    emit authenticated();
    ui->password_edit->clear();
    hide_passwd_widgets(true);
    ui->error_label->setHidden(true);
}

void KrbAuthDialog::on_sign_in() {
    const QString principal = ui->principal_cmb_box->currentText();
    bool result;
    switch (state) {
    case KRB5_DIALOG_STATE_AUTHENTICATION:
        authenticate(principal);
        break;
    case KRB5_DIALOG_STATE_PASSWORD_CHANGE:
        result = change_password(principal);
        if (result) {
            ui->password_edit->setText(ui->password_new_edit->text());
            authenticate(principal);
            switch_ui_to_authentication();
        }
    }
}

void KrbAuthDialog::on_show_passwd(bool show) {
    if (show) {
        ui->password_edit->setEchoMode(QLineEdit::Normal);
        ui->password_new_edit->setEchoMode(QLineEdit::Normal);
        ui->password_confirm_edit->setEchoMode(QLineEdit::Normal);
    } else {
        ui->password_edit->setEchoMode(QLineEdit::Password);
        ui->password_new_edit->setEchoMode(QLineEdit::Password);
        ui->password_confirm_edit->setEchoMode(QLineEdit::Password);
    }
}

void KrbAuthDialog:: show_error_message(const QString &error) {
    ui->error_label->setHidden(false);
    error.isEmpty() ? ui->error_label->setText(tr("Authentication failed")) :
                      ui->error_label->setText(error);
}

void KrbAuthDialog::on_principal_selected(const QString &principal) {
    Krb5TgtState tgt_state = client->tgt_data(principal).state;
    // TODO: Check ways to renew expired tickets (with still valid renewal
    // lifetime)
    switch (tgt_state) {
    case Krb5TgtState_Active:
//    case Krb5TgtState_Expired:
        hide_passwd_widgets(true);
        break;
    default:
        hide_passwd_widgets(false);
        ui->principal_cmb_box->setFocus();
    }

    switch_ui_to_authentication();
    ui->error_label->hide();

    bool checked = ((! principal.isEmpty())
                    && (principal == client->system_principal()))
        || settings_are_creds_saved(principal);
    ui->remember_checkbox->setChecked(checked);
    ui->remember_checkbox->setDisabled(principal == client->system_principal());

    adjustSize();
}

void KrbAuthDialog::hide_passwd_widgets(bool hide) {
    ui->passwd_label->setHidden(hide);
    ui->password_edit->setHidden(hide);
    ui->show_passwd_checkbox->setHidden(hide);
    ui->ticket_available_label->setVisible(hide);
    ui->principal_cmb_box->setFocus();
    adjustSize();
}

void KrbAuthDialog::remember_principal(bool remember) {
    QStringList remembered_principals = settings_get_remembered_principals();
    const QString principal = ui->principal_cmb_box->currentText();

    if (remember && (! remembered_principals.contains(principal))) {
        remembered_principals.append(principal);
    }
    else if ((! remember) && remembered_principals.contains(principal)) {
        remembered_principals.removeAll(principal);
    }

    client->update_temp_caches(remembered_principals);
    settings_set_variant(SETTING_remembered_principals, remembered_principals);
}

void KrbAuthDialog::on_use_system_credentials(bool use_system) {
    const QString principal = use_system ? client->system_principal() :
                                           client->current_principal();
    switch_ui_to_authentication();
    hide_passwd_widgets(! principal.isEmpty());
    if (principal.isEmpty() && use_system) {
        show_error_message(tr("Failed to find system credentials"));
    }
    else {
        ui->error_label->setHidden(true);
    }

    settings_set_variant(SETTING_use_system_credentials, use_system);
    ui->principal_cmb_box->setCurrentText(principal);
    ui->principal_cmb_box->setDisabled(use_system);
    adjustSize();
}
