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

#include <QLineEdit>

#include "adldap.h"
#include "core/globals.h"
#include "core/line_edit_utils.h"
#include "ui/attribute_edit/string_edit.h"

StringEdit::StringEdit(QLineEdit *edit_arg, const QString &attribute_arg, QObject *parent)
: AttributeEdit(parent) {
    attribute = attribute_arg;
    edit = edit_arg;

    if (g_adconfig->get_attribute_is_number(attribute)) {
        line_edit_set_to_decimal_numbers_only(edit);
    }

    line_edit_limit_edit(edit, attribute);

    connect(
        edit, &QLineEdit::textChanged,
        this, &AttributeEdit::edited);
}

void StringEdit::load(AdInterface &ad, const AdObject &object) {
    Q_UNUSED(ad);

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
