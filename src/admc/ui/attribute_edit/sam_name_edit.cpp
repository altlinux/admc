/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2021-2026 BaseALT Ltd.
 * Copyright (C) 2021-2022 Dmitry Degtyarev
 * Copyright (C) 2025 Semyon Knyazev
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

#include <QLineEdit>
#include <QRegularExpression>

#include "adldap.h"
#include "ui/attribute_edit/sam_name_edit.h"
#include "core/globals.h"
#include "ui/message_box.h"
#include "ui/utils.h"           // string_contains_bad_chars

SamNameEdit::SamNameEdit(QLineEdit *edit_arg, QLineEdit *domain_edit,
                         QObject *parent)
: AttributeEdit(parent) {
    edit = edit_arg;

    if (! edit->isReadOnly()) {
        edit->setMaxLength(SAM_NAME_MAX_LENGTH);
    }

    const QString domain = g_adconfig->domain();
    const QString domain_name = domain.split(".")[0];
    const QString domain_text = domain_name + "\\";

    domain_edit->setText(domain_text);

    connect(
        edit, &QLineEdit::textChanged,
        this, &AttributeEdit::edited);
}

/**
 * Get a trimmed value of the Samba name from the input field.
 * @param edit An edit field to get the value from.
 * @return The trimmed value as a string.
 */
static QString get_value(const QLineEdit *edit) {
    return edit->text().trimmed();
}

void SamNameEdit::load(AdInterface &ad, const AdObject &object) {
    Q_UNUSED(ad);
    edit->setText(object.get_string(ATTRIBUTE_SAM_ACCOUNT_NAME));
}

bool SamNameEdit::verify(AdInterface &ad, const QString &dn) const {
    Q_UNUSED(ad);
    Q_UNUSED(dn);
    return sam_name_edit_verify(edit);
}

// NOTE: requirements are from here
// https://social.technet.microsoft.com/wiki/contents/articles/11216.active-directory-requirements-for-creating-objects.aspx#Note_Regarding_the_quot_quot_Character_in_sAMAccountName
bool sam_name_edit_verify(QLineEdit *edit) {
    const QString new_value = get_value(edit);
    const bool contains_bad_chars =
        string_contains_bad_chars(new_value, SAM_NAME_BAD_CHARS);
    const bool ends_with_dot = new_value.endsWith(".");
    const bool value_is_valid = (!contains_bad_chars && !ends_with_dot);

    if (!value_is_valid) {
        const QString error_text =
            QString(
                QCoreApplication::translate(
                    "SamNameEdit",
                    "Input field for Logon name (pre-Windows 2000) contains one or more of the following illegal characters: @ \" [ ] : ; | = + * ? < > / \\ ,"));
        message_box_warning(
            edit,
            QCoreApplication::translate("SamNameEdit", "Error"),
            error_text);

        return false;
    }

    return true;
}

bool SamNameEdit::apply(AdInterface &ad, const QString &dn) const {
    const QString new_value = get_value(edit);
    return ad.attribute_replace_string(
        dn, ATTRIBUTE_SAM_ACCOUNT_NAME, new_value);
}

void SamNameEdit::set_enabled(const bool enabled) {
    edit->setEnabled(enabled);
}
