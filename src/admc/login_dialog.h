/**
 * Copyright (c) by: Mike Dawson mike _at_ no spam gp2x.org
 *
 * This file may be used subject to the terms and conditions of the
 * GNU Library General Public License Version 2, or any later version
 * at your option, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
**/

#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H

/**
 * Opened when app launches. Asks for username and password
 * and then authenticates through kerberos. Toggling "auto
 * login" option will skip this dialog on subsequent
 * launches unless authentication is required.
 */

#include <QDialog>

class QLineEdit;
class QCheckBox;

class LoginDialog final : public QDialog {
Q_OBJECT

public:
    LoginDialog(QWidget *parent);

private slots:
    void on_rejected();
    void login();

private:
    QLineEdit *principal_edit = nullptr;
    QLineEdit *password_edit = nullptr;
    QCheckBox *autologin_check;

};

#endif /* LOGIN_DIALOG_H */
