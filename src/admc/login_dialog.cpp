/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
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

#include "login_dialog.h"
#include "ad_interface.h"
#include "settings.h"
#include "utils.h"

#include <krb5.h>

#include <QDebug>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QString>
#include <QGridLayout>
#include <QApplication>
#include <QCheckBox>

LoginDialog::LoginDialog(QWidget *parent)
: QDialog(parent)
{
    setWindowTitle(tr("Login"));
    setAttribute(Qt::WA_DeleteOnClose);

    const auto principal_edit_label = new QLabel(tr("Login as:"), this);
    principal_edit = new QLineEdit(this);
    const QString last_principal = SETTINGS()->get_variant(VariantSetting_Principal).toString();
    if (!last_principal.isEmpty()) {
        principal_edit->setText("administrator");
    } else {
        principal_edit->setText(last_principal);
    }

    const auto password_edit_label = new QLabel(tr("Password:"), this);
    password_edit = new QLineEdit(this);
    password_edit->setEchoMode(QLineEdit::Password);

    const bool principal_entered = !principal_edit->text().isEmpty();
    if (principal_entered) {
        password_edit->setFocus(Qt::NoFocusReason);
    }

    autologin_check = new QCheckBox(tr("Auto-login next time"));

    const auto login_button = new QPushButton(tr("Login"), this);
    login_button->setAutoDefault(false);

    const auto layout = new QGridLayout();
    setLayout(layout);
    append_to_grid_layout_with_label(layout, principal_edit_label, principal_edit);
    append_to_grid_layout_with_label(layout, password_edit_label, password_edit);
    layout->addWidget(autologin_check);
    layout->addWidget(login_button);

    connect(
        login_button, &QAbstractButton::clicked,
        this, &LoginDialog::login);
    connect(
        password_edit, &QLineEdit::returnPressed,
        this, &LoginDialog::login);
    connect(
        this, &QDialog::rejected,
        this, &LoginDialog::on_rejected);
}

// TODO: Implement changing password in here? Error code should be KRB5KDC_ERR_KEY_EXP
void LoginDialog::login() {
    krb5_error_code result;
    krb5_context context;
    krb5_get_init_creds_opt *options = nullptr;
    krb5_ccache ccache = nullptr;
    krb5_principal principal = nullptr;
    krb5_creds my_creds;

    memset(&my_creds, 0, sizeof(my_creds));

    const QString password = password_edit->text();
    const QByteArray password_bytes = password.toUtf8();
    const char *password_cstr = password_bytes.constData();

    result = krb5_init_context(&context);
    if (result) {
        QMessageBox::critical(this, tr("Authentication error"), tr("Failed to init krb5 context"));
        return;
    }

    // Opens message box with the message followed by an
    // error string for last krb5 error.
    auto error_message =
    [this, context, &result](const QString &message) {
        const char *krb5_error_message_cstr = krb5_get_error_message(context, result);
        const QString krb5_error_message = QString(krb5_error_message_cstr);
        krb5_free_error_message(context, krb5_error_message_cstr);

        const QString message_with_krb5_error = QString("%1. Error: %2.").arg(message, krb5_error_message);
        QMessageBox::critical(this, tr("Authentication error"), message_with_krb5_error);
    };

    // Parse principal name into principal struct
    const QString principal_name = principal_edit->text();
    const QByteArray principal_name_bytes = principal_name.toUtf8();
    const char *principal_name_cstr = principal_name_bytes.constData();
    result = krb5_parse_name(context, principal_name_cstr, &principal);
    if (result) {
        error_message(tr("Failed to parse principal name"));
        goto cleanup;
    }

    // Use default ccache
    // TODO: kinit attempts to use an existing ccache for selected principal, if it exists and only if that fails does it use the default one. Not sure when and how another cache for principal might exist alongside the default one.
    result = krb5_cc_default(context, &ccache);
    if (result) {
        error_message(tr("Failed to get default ccache"));
        goto cleanup;
    }
    
    result = krb5_get_init_creds_opt_alloc(context, &options);
    if (result) {
        error_message(tr("Failed to init krb5 options"));
        goto cleanup;
    }

    result = krb5_get_init_creds_opt_set_out_ccache(context, options, ccache);
    if (result) {
        error_message(tr("Failed to set out ccache"));
        goto cleanup;
    }

    result = krb5_get_init_creds_password(context, &my_creds, principal, password_cstr, nullptr, nullptr, 0, nullptr, options);

    if (result) {
        error_message(tr("Failed to get initial credentials"));
    } else {
        qDebug() << "Got initial credentials";

        const bool login_success = AD()->login();

        if (login_success) {
            qDebug() << "Logged in successfully";

            SETTINGS()->set_variant(VariantSetting_Principal, principal_edit->text());

            const bool autologin_checked = checkbox_is_checked(autologin_check);
            SETTINGS()->set_bool(BoolSetting_AutoLogin, autologin_checked);

            accept();
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Failed to login!"));
        }
    }

    cleanup: {
        krb5_free_cred_contents(context, &my_creds);
        krb5_free_principal(context, principal);
        if (ccache != nullptr) {
            krb5_cc_close(context, ccache);
        }
        if (options != nullptr) {
            krb5_get_init_creds_opt_free(context, options);
        }
        krb5_free_context(context);
    }
}

void LoginDialog::on_rejected() {
    QApplication::closeAllWindows();
    QApplication::quit();
}
