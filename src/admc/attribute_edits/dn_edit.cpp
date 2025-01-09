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

#include "attribute_edits/dn_edit.h"

#include "adldap.h"
#include "utils.h"

#include <QLineEdit>

DNEdit::DNEdit(QLineEdit *edit_arg, QObject *parent)
: AttributeEdit(parent) {
    edit = edit_arg;

    limit_edit(edit, ATTRIBUTE_DN);

    connect(
        edit, &QLineEdit::textChanged,
        this, &AttributeEdit::edited);
}

void DNEdit::load(AdInterface &ad, const AdObject &object) {
    UNUSED_ARG(ad);

    const QString dn = object.get_dn();
    const QString dn_as_canonical = dn_canonical(dn);

    edit->setText(dn_as_canonical);
}
