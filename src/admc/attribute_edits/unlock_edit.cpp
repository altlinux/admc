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

#include "attribute_edits/unlock_edit.h"

#include "adldap.h"
#include "utils.h"

#include <QCheckBox>

UnlockEdit::UnlockEdit(QCheckBox *check_arg, QObject *parent)
: AttributeEdit(parent) {
    check = check_arg;

    connect(
        check, &QCheckBox::stateChanged,
        this, &AttributeEdit::edited);
}

QString UnlockEdit::label_text() {
    return tr("Unlock account");
}

void UnlockEdit::load(AdInterface &ad, const AdObject &object) {
    UNUSED_ARG(ad);
    UNUSED_ARG(object);

    check->setChecked(false);
}

bool UnlockEdit::apply(AdInterface &ad, const QString &dn) const {

    if (check->isChecked()) {
        const bool result = ad.user_unlock(dn);
        check->setChecked(false);

        return result;
    } else {
        return true;
    }
}
