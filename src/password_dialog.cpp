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

#include "password_dialog.h"
#include "ad_interface.h"

#include <QLineEdit>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

// TODO: find out exact password rules and add them to dialog?
// or display more info about constraint violations

PasswordDialog::PasswordDialog(const QString &target_arg, QWidget *parent)
: QDialog(parent)
{
    target = target_arg;

    const QString target_name = extract_name_from_dn(target);
    const QString top_label_text = QString(tr("Resetting password of \"%1\"")).arg(target_name);
    const auto top_label = new QLabel(top_label_text, this);

    const auto new_password_label = new QLabel(tr("New password:"), this);
    new_password_edit = new QLineEdit(this);
    new_password_edit->setEchoMode(QLineEdit::Password);

    const auto confirm_password_label = new QLabel(tr("Confirm password:"), this);
    confirm_password_edit = new QLineEdit(this);
    confirm_password_edit->setEchoMode(QLineEdit::Password);

    const auto ok_button = new QPushButton(tr("OK"), this);
    const auto cancel_button = new QPushButton(tr("Cancel"), this);

    const auto layout = new QGridLayout(this);
    layout->addWidget(top_label, 0, 0);
    layout->addWidget(new_password_label, 1, 0);
    layout->addWidget(new_password_edit, 1, 1, 1, 2);
    layout->addWidget(confirm_password_label, 2, 0);
    layout->addWidget(confirm_password_edit, 2, 1, 1, 2);
    layout->addWidget(cancel_button, 3, 0, Qt::AlignLeft);
    layout->addWidget(ok_button, 3, 2, Qt::AlignRight);

    connect(
        ok_button, &QAbstractButton::clicked,
        this, &PasswordDialog::on_ok_button);
    connect(
        cancel_button, &QAbstractButton::clicked,
        this, &PasswordDialog::on_cancel_button);
}

void PasswordDialog::on_ok_button(bool) {
    const QString new_password = new_password_edit->text();
    const QString confirm_password = confirm_password_edit->text();

    if (new_password != confirm_password) {
        QMessageBox::warning(this, tr("Warning"), tr("Passwords don't match!"));
        
        return;
    }

    const bool success = AdInterface::instance()->set_pass(target, new_password);

    if (success) {
        done(QDialog::Accepted);
    } else {
        const QString error = AdInterface::instance()->get_error_str();
        QMessageBox::warning(this, "Warning", QString(tr("Failed to set password! %1")).arg(error));
    }
}

void PasswordDialog::on_cancel_button(bool) {
    done(QDialog::Rejected);
}
