/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
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


#include "krb_auth_dialog.h"
#include "ui_krb_auth_dialog.h"

#include <stdexcept>
#include "settings.h"


KrbAuthDialog::KrbAuthDialog(QWidget *parent) : AuthDialogBase(parent),
    ui(new Ui::KrbAuthDialog) {

    ui->setupUi(this);

    try {
        client = std::unique_ptr<Krb5Client>(new Krb5Client);
    }
    catch (const std::runtime_error& e) {
        show_error_message(e.what());
    }

    setupWidgets();
}

KrbAuthDialog::~KrbAuthDialog() {
    delete ui;
}

void KrbAuthDialog::setupWidgets() {
    ui->error_label->setHidden(true);
    ui->error_label->setStyleSheet("color: red");

    ui->ticket_available_label->setVisible(false);

    ui->principal_cmb_box->addItems(client->available_principals());
    ui->principal_cmb_box->setCurrentText(settings_get_variant(SETTING_last_logged_user).toString());

    connect(ui->principal_cmb_box, &QComboBox::currentTextChanged, this, &KrbAuthDialog::on_principal_selected);
    connect(ui->show_passwd_checkbox, &QCheckBox::toggled, this, &KrbAuthDialog::on_show_passwd);
    connect(ui->sign_in_button, &QPushButton::clicked, this, &KrbAuthDialog::on_sign_in);

    ui->principal_cmb_box->setFocus();

    adjustSize();
}

void KrbAuthDialog::on_sign_in() {
    const QString principal = ui->principal_cmb_box->currentText();
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

    bool ticket_active = client->tgt_data(principal).state == Krb5TgtState_Active;
    try {
        if (client->principal_has_cache(principal) && ticket_active) {
            client->set_current_principal(principal);
        }
        else {
            client->authenticate(principal, ui->password_edit->text());
            ui->principal_cmb_box->addItem(principal);
        }
    }
    catch (const std::runtime_error& e) {
        show_error_message(e.what());
        return;
    }

    settings_set_variant(SETTING_last_logged_user, principal);

    emit authenticated();
    ui->password_edit->clear();
    hide_passwd_widgets(true);
    ui->error_label->setHidden(true);
}

void KrbAuthDialog::on_show_passwd(bool show) {
    show ? ui->password_edit->setEchoMode(QLineEdit::Normal) :
           ui->password_edit->setEchoMode(QLineEdit::Password);
}

void KrbAuthDialog:: show_error_message(const QString &error) {
    ui->error_label->setHidden(false);
    error.isEmpty() ? ui->error_label->setText(tr("Authentication failed")) :
                      ui->error_label->setText(error);
}

void KrbAuthDialog::on_principal_selected(const QString &principal) {
    Krb5TgtState tgt_state = client->tgt_data(principal).state;

    // TODO: Check ways to renew expired tickets (with still valid renewal lifetime)
    switch (tgt_state) {
    case Krb5TgtState_Active:
//    case Krb5TgtState_Expired:
        hide_passwd_widgets(true);
        break;
    default:
        hide_passwd_widgets(false);
        ui->principal_cmb_box->setFocus();
    }

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
