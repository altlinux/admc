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


#ifndef KRB_AUTH_DIALOG_H
#define KRB_AUTH_DIALOG_H

#include "auth_dialog_base.h"
#include <memory>
#include "krb5client.h"

namespace Ui {
class KrbAuthDialog;
}

class Krb5Client;

/**
 * Kerberos authentication dialog
 */

class KrbAuthDialog final : public AuthDialogBase {
    Q_OBJECT

public:
    explicit KrbAuthDialog(QWidget *parent = nullptr);
    ~KrbAuthDialog();

private:
    Ui::KrbAuthDialog *ui;

    virtual void setupWidgets() override;
    virtual void on_sign_in() override;
    virtual void on_show_passwd(bool show) override;
    virtual void show_error_message(const QString &error = QString()) override;

    void on_use_system_credentials(bool use_system);
    void on_principal_selected(const QString &principal);
    void hide_passwd_widgets(bool hide);

    std::unique_ptr<Krb5Client> client = nullptr;
};

#endif // KRB_AUTH_DIALOG_H
