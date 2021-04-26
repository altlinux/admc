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

#include "multi_edits/string_multi_edit.h"

#include "adldap.h"
#include "globals.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QLabel>

StringMultiEdit::StringMultiEdit(const QString &attribute_arg, QList<AttributeMultiEdit *> &edits_out, QObject *parent)
: AttributeMultiEdit(edits_out, parent)
{
    attribute = attribute_arg;

    // NOTE: default to using user object class for
    // attribute display name because multi edits are mostly
    // for users
    const QString label_text = g_adconfig->get_attribute_display_name(attribute, CLASS_USER) + ":";
    label->setText(label_text);

    edit = new QLineEdit();

    connect(
        edit, &QLineEdit::textChanged,
        this, &StringMultiEdit::edited);

    set_enabled(false);
}

void StringMultiEdit::add_to_layout(QFormLayout *layout) {
    layout->addRow(check_and_label_wrapper, edit);
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
