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

#include "attribute_edits/string_list_edit.h"

#include "adldap.h"
#include "attribute_dialogs/list_attribute_dialog.h"
#include "utils.h"

#include <QPushButton>

StringListEdit::StringListEdit(QPushButton *button_arg, const QString &attribute_arg, QObject *parent)
: AttributeEdit(parent) {
    button = button_arg;
    attribute = attribute_arg;

    connect(
        button, &QPushButton::clicked,
        this, &StringListEdit::on_button);
}

void StringListEdit::load(AdInterface &ad, const AdObject &object) {
    UNUSED_ARG(ad);

    values = object.get_values(attribute);
}

bool StringListEdit::apply(AdInterface &ad, const QString &dn) const {
    const bool success = ad.attribute_replace_values(dn, attribute, values);

    return success;
}

void StringListEdit::on_button() {
    const bool read_only = false;
    auto dialog = new ListAttributeDialog(values, attribute, read_only, button);
    dialog->setWindowTitle(tr("Edit values"));
    dialog->open();

    connect(
        dialog, &QDialog::accepted,
        this,
        [this, dialog]() {
            values = dialog->get_value_list();

            emit edited();
        });
}
