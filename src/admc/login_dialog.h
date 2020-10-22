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

#include <QDialog>

class QLineEdit;
class QCheckBox;

class LoginDialog final : public QDialog {
Q_OBJECT

public:
    LoginDialog(QWidget *parent);

private slots:
    void on_login_button();
    void on_rejected();

private:
    QLineEdit *domain_edit = nullptr;
    QLineEdit *site_edit = nullptr;
    QCheckBox *autologin_check;
};

#endif /* LOGIN_DIALOG_H */
