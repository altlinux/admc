/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

#include "attribute_edits/sam_name_edit.h"

#include "adldap.h"
#include "globals.h"
#include "utils.h"

#include <QLineEdit>
#include <QRegularExpression>

SamNameEdit::SamNameEdit(QLineEdit *edit_arg, QLineEdit *domain_edit, QObject *parent)
: AttributeEdit(parent) {
    edit = edit_arg;

    const QString domain_text = []() {
        const QString domain = g_adconfig->domain();
        const QString domain_name = domain.split(".")[0];
        const QString out = domain_name + "\\";

        return out;
    }();

    domain_edit->setText(domain_text);

    connect(
        edit, &QLineEdit::textChanged,
        this, &AttributeEdit::edited);
}

void SamNameEdit::load(AdInterface &ad, const AdObject &object) {
    UNUSED_ARG(ad);

    // NOTE: not using limit_edit() here because need
    // custom length here different from the upper
    // range defined in schema. According to microsoft,
    // it's 16 for computers and 20 for users, but
    // groups can also have sam names so defaulting to
    // value for users.
    const int max_length = [&]() {
        if (object.is_class(CLASS_COMPUTER)) {
            return 16;
        } else {
            return 20;
        }
    }();
    edit->setMaxLength(max_length);

    const QString value = object.get_string(ATTRIBUTE_SAM_ACCOUNT_NAME);
    edit->setText(value);
}

// NOTE: requirements are from here
// https://social.technet.microsoft.com/wiki/contents/articles/11216.active-directory-requirements-for-creating-objects.aspx#Note_Regarding_the_quot_quot_Character_in_sAMAccountName
bool SamNameEdit::verify(AdInterface &ad, const QString &dn) const {
    UNUSED_ARG(ad);
    UNUSED_ARG(dn);

    const QString new_value = edit->text();

    const bool contains_bad_chars = [&]() {
        const QRegularExpression sam_name_regexp = [&]() {
            const QString sam_name_bad_chars_escaped = QRegularExpression::escape(SAM_NAME_BAD_CHARS);
            const QString regexp_string = QString("[%1]").arg(sam_name_bad_chars_escaped);
            const QRegularExpression out = QRegularExpression(regexp_string);
    
            return out;
        }();

        const bool out = new_value.contains(sam_name_regexp);

        return out;
    }();

    const bool ends_with_dot = new_value.endsWith(".");

    const bool value_is_valid = (!contains_bad_chars && !ends_with_dot);

    if (!value_is_valid) {
        message_box_warning(edit, tr("Error"), "bad!");
    }

    return value_is_valid;
}

bool SamNameEdit::apply(AdInterface &ad, const QString &dn) const {
    const QString new_value = edit->text();
    const bool success = ad.attribute_replace_string(dn, ATTRIBUTE_SAM_ACCOUNT_NAME, new_value);

    return success;
}
