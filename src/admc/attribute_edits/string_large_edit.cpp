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

#include "attribute_edits/string_large_edit.h"

#include "adldap.h"
#include "utils.h"

#include <QPlainTextEdit>

StringLargeEdit::StringLargeEdit(QPlainTextEdit *edit_arg, const QString &attribute_arg, QObject *parent)
: AttributeEdit(parent) {
    attribute = attribute_arg;
    edit = edit_arg;

    connect(
        edit, &QPlainTextEdit::textChanged,
        this, &AttributeEdit::edited);
}

void StringLargeEdit::load(AdInterface &ad, const AdObject &object) {
    UNUSED_ARG(ad);

    const QString value = object.get_string(attribute);

    edit->setPlainText(value);
}

void StringLargeEdit::set_read_only(const bool read_only) {
    edit->setDisabled(read_only);
}

bool StringLargeEdit::apply(AdInterface &ad, const QString &dn) const {
    const QString new_value = edit->toPlainText();
    const bool success = ad.attribute_replace_string(dn, attribute, new_value);

    return success;
}
