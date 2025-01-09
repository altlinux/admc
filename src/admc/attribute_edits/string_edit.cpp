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

#include "attribute_edits/string_edit.h"

#include "adldap.h"
#include "globals.h"
#include "utils.h"

#include <QLineEdit>

StringEdit::StringEdit(QLineEdit *edit_arg, const QString &attribute_arg, QObject *parent)
: AttributeEdit(parent) {
    attribute = attribute_arg;
    edit = edit_arg;

    if (g_adconfig->get_attribute_is_number(attribute)) {
        set_line_edit_to_decimal_numbers_only(edit);
    }

    limit_edit(edit, attribute);

    connect(
        edit, &QLineEdit::textChanged,
        this, &AttributeEdit::edited);
}

void StringEdit::load(AdInterface &ad, const AdObject &object) {
    UNUSED_ARG(ad);

    const QString value = object.get_string(attribute);
    edit->setText(value);
}

bool StringEdit::apply(AdInterface &ad, const QString &dn) const {
    const QString new_value = edit->text().trimmed();
    const bool success = ad.attribute_replace_string(dn, attribute, new_value);

    return success;
}

void StringEdit::set_enabled(const bool enabled) {
    edit->setEnabled(enabled);
}
