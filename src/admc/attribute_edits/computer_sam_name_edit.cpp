/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#include "attribute_edits/computer_sam_name_edit.h"

#include "adldap.h"
#include "attribute_edits/sam_name_edit.h"
#include "globals.h"
#include "utils.h"

#include <QLineEdit>
#include <QRegularExpression>

ComputerSamNameEdit::ComputerSamNameEdit(QLineEdit *edit_arg, QLineEdit *domain_edit, QObject *parent)
: AttributeEdit(parent) {
    edit = edit_arg;

    edit->setMaxLength(SAM_NAME_COMPUTER_MAX_LENGTH);

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

void ComputerSamNameEdit::load(AdInterface &ad, const AdObject &object) {
    UNUSED_ARG(ad);

    // NOTE: display value without the '$' at the end
    const QString value = [&]() {
        QString out = object.get_string(ATTRIBUTE_SAM_ACCOUNT_NAME);

        if (out.endsWith('$')) {
            out.chop(1);
        }

        return out;
    }();
    edit->setText(value);
}

// NOTE: requirements are from here
// https://social.technet.microsoft.com/wiki/contents/articles/11216.active-directory-requirements-for-creating-objects.aspx#Note_Regarding_the_quot_quot_Character_in_sAMAccountName
bool ComputerSamNameEdit::verify(AdInterface &ad, const QString &dn) const {
    UNUSED_ARG(ad);
    UNUSED_ARG(dn);

    const bool out = sam_name_edit_verify(edit);

    return out;
}

bool ComputerSamNameEdit::apply(AdInterface &ad, const QString &dn) const {
    const QString name = edit->text().trimmed();
    const QString new_value = QString("%1$").arg(name);
    const bool success = ad.attribute_replace_string(dn, ATTRIBUTE_SAM_ACCOUNT_NAME, new_value);

    return success;
}

void ComputerSamNameEdit::set_enabled(const bool enabled) {
    edit->setEnabled(enabled);
}
