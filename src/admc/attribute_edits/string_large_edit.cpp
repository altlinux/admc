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

#include "attribute_edits/string_large_edit.h"

#include "adldap.h"
#include "globals.h"
#include "utils.h"

#include <QPlainTextEdit>

StringLargeEdit::StringLargeEdit(QPlainTextEdit *edit_arg, const QString &attribute_arg, QObject *parent)
: AttributeEdit(parent) {
    attribute = attribute_arg;
    edit = edit_arg;
    ignore_on_text_changed = false;

    connect(
        edit, &QPlainTextEdit::textChanged,
        this, &AttributeEdit::edited);
    connect(
        edit, &QPlainTextEdit::textChanged,
        this, &StringLargeEdit::on_text_changed);
}

void StringLargeEdit::load(AdInterface &ad, const AdObject &object) {
    UNUSED_ARG(ad);

    const QString value = object.get_string(attribute);

    edit->setPlainText(value);
}

bool StringLargeEdit::apply(AdInterface &ad, const QString &dn) const {
    const QString new_value = edit->toPlainText();
    const bool success = ad.attribute_replace_string(dn, attribute, new_value);

    return success;
}

// NOTE: this is a custom length limit mechanism
// because QPlainText doesn't have it built-in
void StringLargeEdit::on_text_changed() {
    ignore_on_text_changed = true;
    {
        const int range_upper = g_adconfig->get_attribute_range_upper(attribute);

        const QString value = edit->toPlainText();

        if (value.length() > range_upper) {
            const QString shortened_value = value.left(range_upper);
            edit->setPlainText(shortened_value);
        }
    }
    ignore_on_text_changed = false;
}
