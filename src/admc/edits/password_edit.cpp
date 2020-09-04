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

#include "edits/password_edit.h"
#include "utils.h"
#include "ad_interface.h"

#include <QLineEdit>
#include <QGridLayout>
#include <QMessageBox>
#include <QLabel>

PasswordEdit::PasswordEdit() {
    edit = new QLineEdit();
    confirm_edit = new QLineEdit();

    edit->setEchoMode(QLineEdit::Password);
    confirm_edit->setEchoMode(QLineEdit::Password);

    QObject::connect(
        edit, &QLineEdit::textChanged,
        [this]() {
            emit edited();
        });
}

void PasswordEdit::load(const QString &dn) {
    // NOTE: PasswordEdit does not load current value, it starts out blank and is not reloaded
    emit edited();
}

void PasswordEdit::add_to_layout(QGridLayout *layout) {
    const auto password_label = new QLabel(QObject::tr("Password:"));
    const auto confirm_label = new QLabel(QObject::tr("Confirm password:"));

    connect_changed_marker(this, password_label);
    connect_changed_marker(this, confirm_label);

    append_to_grid_layout_with_label(layout, password_label, edit);
    append_to_grid_layout_with_label(layout, confirm_label, confirm_edit);
}

bool PasswordEdit::verify_input(QWidget *parent) {
    const QString pass = edit->text();
    const QString confirm_pass = confirm_edit->text();

    if (pass != confirm_pass) {
        const QString error_text = QString(QObject::tr("Passwords don't match!"));
        QMessageBox::warning(parent, QObject::tr("Error"), error_text);

        return false;
    }

    return true;
}

bool PasswordEdit::changed() const {
    return true;
}

bool PasswordEdit::apply(const QString &dn) {
    const QString new_value = edit->text();

    const bool success = AdInterface::instance()->user_set_pass(dn, new_value);

    return success;
}
