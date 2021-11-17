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

#include "attribute_multi_edits/string_multi_edit.h"

#include "adldap.h"
#include "globals.h"

#include <QLineEdit>

StringMultiEdit::StringMultiEdit(QLineEdit *edit_arg, QCheckBox *check, const QString &attribute_arg, QList<AttributeMultiEdit *> &edits_out, QObject *parent)
: AttributeMultiEdit(check, edits_out, parent) {
    attribute = attribute_arg;

    edit = edit_arg;

    connect(
        edit, &QLineEdit::textChanged,
        this, &AttributeMultiEdit::edited);

    set_enabled(false);
}

bool StringMultiEdit::apply_internal(AdInterface &ad, const QString &target) {
    const QString new_value = edit->text();
    return ad.attribute_replace_string(target, attribute, new_value);
}

void StringMultiEdit::set_enabled(const bool enabled) {
    if (!enabled) {
        edit->setText(QString());
    }

    edit->setEnabled(enabled);
}
