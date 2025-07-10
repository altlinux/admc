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

#ifndef AUTHDIALOGBASE_H
#define AUTHDIALOGBASE_H

#include <QDialog>

/**
 * Base class for authentication dialogs
 */

class AuthDialogBase : public QDialog {
    Q_OBJECT

public:
    using QDialog::QDialog;
    virtual ~AuthDialogBase() = default;

    virtual void logout(bool delete_creds) = 0;

protected:
    virtual void setupWidgets() = 0;
    virtual void on_sign_in() = 0;
    virtual void on_show_passwd(bool show) = 0;
    virtual void show_error_message(const QString &error) = 0;

signals:
    void authenticated();
};


#endif // AUTHDIALOGBASE_H
