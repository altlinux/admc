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

#include "attribute_edits/delegation_edit.h"

#include "adldap.h"
#include "globals.h"
#include "utils.h"

#include <QRadioButton>

DelegationEdit::DelegationEdit(QRadioButton *off_button_arg, QRadioButton *on_button_arg, QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent) {
    off_button = off_button_arg;
    on_button = on_button_arg;

    connect(
        off_button, &QAbstractButton::clicked,
        this, &AttributeEdit::edited);
    connect(
        on_button, &QAbstractButton::clicked,
        this, &AttributeEdit::edited);
}

void DelegationEdit::load_internal(AdInterface &ad, const AdObject &object) {
    UNUSED_ARG(ad);

    const bool is_on = object.get_account_option(AccountOption_TrustedForDelegation, g_adconfig);

    if (is_on) {
        on_button->setChecked(true);
    } else {
        off_button->setChecked(true);
    }
}

void DelegationEdit::set_read_only(const bool read_only) {
    on_button->setEnabled(read_only);
    off_button->setEnabled(read_only);
}

bool DelegationEdit::apply(AdInterface &ad, const QString &dn) const {
    const bool is_on = [&]() {
        if (on_button->isChecked()) {
            return true;
        } else if (off_button->isChecked()) {
            return false;
        }

        return false;
    }();

    const bool success = ad.user_set_account_option(dn, AccountOption_TrustedForDelegation, is_on);

    return success;
}
