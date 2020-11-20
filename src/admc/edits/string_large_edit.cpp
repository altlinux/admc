/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
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

#include "edits/string_large_edit.h"
#include "ad_interface.h"
#include "ad_config.h"

#include <QPlainTextEdit>
#include <QFormLayout>

StringLargeEdit::StringLargeEdit(const QString &attribute_arg, const QString &objectClass_arg, QObject *parent, QList<AttributeEdit *> *edits_out)
: AttributeEdit(edits_out, parent)
{
    attribute = attribute_arg;
    objectClass = objectClass_arg;
    
    edit = new QPlainTextEdit();

    QObject::connect(
        edit, &QPlainTextEdit::textChanged,
        [this]() {
            emit edited();
        });
}

void StringLargeEdit::load_internal(const AdObject &object) {
    const QString value = object.get_string(attribute);
    
    edit->setPlainText(value);
}

void StringLargeEdit::set_read_only(const bool read_only) {
    edit->setReadOnly(read_only);
}

void StringLargeEdit::add_to_layout(QFormLayout *layout) {
    const QString label_text = ADCONFIG()->get_attribute_display_name(attribute, objectClass) + ":";
    layout->addRow(label_text, edit);
}

bool StringLargeEdit::apply(const QString &dn) const {
    const QString new_value = edit->toPlainText();
    const bool success = AD()->attribute_replace_string(dn, attribute, new_value);

    return success;
}
