/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2026 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#include "attribute_edits/password_edit.h"

#include "adldap.h"
#include "core/globals.h"
#include "core/settings.h"
#include "utils.h"

#include <QCheckBox>
#include <QLineEdit>

PasswordEdit::PasswordEdit(QLineEdit *edit_arg, QLineEdit *confirm_edit_arg, QCheckBox *show_password_check, QObject *parent)
: AttributeEdit(parent) {
    edit = edit_arg;
    confirm_edit = confirm_edit_arg;

    limit_edit(edit, ATTRIBUTE_PASSWORD);
    limit_edit(confirm_edit, ATTRIBUTE_PASSWORD);

    connect(
        edit, &QLineEdit::textChanged,
        this, &AttributeEdit::edited);
    connect(
        show_password_check, &QCheckBox::toggled,
        this, &PasswordEdit::on_show_password_check);

    const bool show_password_is_ON = settings_get_bool(SETTING_show_password);
    show_password_check->setChecked(show_password_is_ON);
}

void PasswordEdit::load(AdInterface &ad, const AdObject &object) {
    UNUSED_ARG(ad);
    UNUSED_ARG(object);

    edit->clear();
    confirm_edit->clear();
}

bool PasswordEdit::verify(AdInterface &ad, const QString &) const {
    UNUSED_ARG(ad);

    const QString pass = edit->text();
    const QString confirm_pass = confirm_edit->text();

    if (pass.isEmpty()) {
        const QString error_text = QString(tr("Password cannot be empty."));
        message_box_warning(edit, tr("Error"), error_text);

        return false;
    }

    if (pass != confirm_pass) {
        const QString error_text = QString(tr("Passwords don't match!"));
        message_box_warning(edit, tr("Error"), error_text);

        return false;
    }

    const bool can_encode = pass.isValidUtf16();
    if (!can_encode) {
        const QString error_text = QString(tr("Password contains invalid characters"));
        message_box_warning(edit, tr("Error"), error_text);

        return false;
    }

    return true;
}

bool PasswordEdit::apply(AdInterface &ad, const QString &dn) const {
    const QString new_value = edit->text();

    const bool success = ad.user_set_pass(dn, new_value);

    return success;
}

QLineEdit *PasswordEdit::get_edit() const {
    return edit;
}

QLineEdit *PasswordEdit::get_confirm_edit() const {
    return confirm_edit;
}

void PasswordEdit::on_show_password_check(bool checked) {
    QLineEdit::EchoMode echo_mode;
    if (checked) {
        echo_mode = QLineEdit::Normal;
    } else {
        echo_mode = QLineEdit::Password;
    }

    edit->setEchoMode(echo_mode);
    confirm_edit->setEchoMode(echo_mode);

    settings_set_variant(SETTING_show_password, checked);
}
