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
        this, &LoginDialog::on_login_button);
    connect(
        this, &QDialog::rejected,
        this, &LoginDialog::on_rejected);
}

void LoginDialog::on_login_button() {
    // Authenticate using krb5
    krb5_context context;
    krb5_ccache ccache = NULL;
    krb5_principal principal;
    krb5_error_code result;
    bool switch_to_cache = false;

    result = krb5_init_context(&context);
    if (result) {
        printf("krb5_init_context failed\n");
        return;
    }

    // Put principal name into principal struct
    const QString principal_name = principal_edit->text();
    const QByteArray principal_name_bytes = principal_name.toUtf8();
    const char *principal_name_cstr = principal_name_bytes.constData();
    result = krb5_parse_name(context, principal_name_cstr, &principal);
    if (result) {
        printf("krb5_parse_name failed\n");
        return;
        // goto cleanup;
    }

    // Use default ccache
    // TODO: kinit attempts to use an existing ccache for this principal, if it exists and only if that fails does it use the default one. Not sure when and how another cache for principal might exist alongside the default one.
    result = krb5_cc_default(context, &ccache);
    if (result) {
        printf("Failed to get default ccache\n");
        return;
        // goto cleanup;
    }

    krb5_creds my_creds;
    memset(&my_creds, 0, sizeof(my_creds));
    
    krb5_get_init_creds_opt *options = NULL;
    result = krb5_get_init_creds_opt_alloc(context, &options);
    if (result) {
        printf("krb5_get_init_creds_opt_alloc fail\n");
        return;
    }

    result = krb5_get_init_creds_opt_set_out_ccache(context, options, ccache);
    if (result) {
        printf("Failed to set out ccache\n");
        // goto cleanup;
        return;
    }

    const krb5_deltat start_time = 0;
    const char *service_name = NULL;
    void *prompter_data = (void *)this;

    const QString password = password_edit->text();
    const QByteArray password_bytes = password.toUtf8();
    const char *password_cstr = password_bytes.constData();

    result = krb5_get_init_creds_password(context, &my_creds, principal, password_cstr, NULL, prompter_data, start_time, service_name, options);

    if (result == 0) {
        printf("krb5_get_init_creds_password success\n");
        const bool login_success = AD()->login();

        if (login_success) {
            SETTINGS()->set_variant(VariantSetting_Principal, principal_edit->text());

            const bool autologin_checked = checkbox_is_checked(autologin_check);
            SETTINGS()->set_bool(BoolSetting_AutoLogin, autologin_checked);

            accept();
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Failed to login!"));
        }
    } else {
        const QString error_string =
        [result]() {
            if (result == KRB5KDC_ERR_KEY_EXP) {
                // TODO: Implement changing password in here?
                return tr("Password expired, change it in kinit.");
            } else if (result == KRB5KDC_ERR_PREAUTH_FAILED) {
                return tr("Incorrect password");
            } else {
                return QString(tr("Unknown krb5 error: %1")).arg( result);
            }
        }();

        QMessageBox::critical(this, tr("Authentication error"), error_string);
    }

    krb5_free_principal(context, principal);
    // TODO: shouldn't need this check?
    if (ccache != NULL) {
        krb5_cc_close(context, ccache);
    }
    krb5_free_context(context);
}

void LoginDialog::on_rejected() {
    QApplication::closeAllWindows();
    QApplication::quit();
}
