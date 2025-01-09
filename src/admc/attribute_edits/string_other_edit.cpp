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

#include "attribute_edits/string_other_edit.h"

#include "adldap.h"
#include "attribute_dialogs/list_attribute_dialog.h"
#include "attribute_edits/string_edit.h"
#include "globals.h"

#include <QLineEdit>
#include <QPushButton>

StringOtherEdit::StringOtherEdit(QLineEdit *line_edit_arg, QPushButton *other_button_arg, const QString &main_attribute, const QString &other_attribute_arg, QObject *parent)
: AttributeEdit(parent)
, other_attribute(other_attribute_arg) {
    main_edit = new StringEdit(line_edit_arg, main_attribute, parent);

    line_edit = line_edit_arg;

    other_button = other_button_arg;

    connect(
        main_edit, &AttributeEdit::edited,
        this, &AttributeEdit::edited);
    connect(
        other_button, &QPushButton::clicked,
        this, &StringOtherEdit::on_other_button);
}

void StringOtherEdit::load(AdInterface &ad, const AdObject &object) {
    main_edit->load(ad, object);

    other_values = object.get_values(other_attribute);
}

void StringOtherEdit::set_read_only(const bool read_only_arg) {
    read_only = read_only_arg;
    line_edit->setReadOnly(read_only);
}

bool StringOtherEdit::apply(AdInterface &ad, const QString &dn) const {
    const bool main_succcess = main_edit->apply(ad, dn);
    const bool other_success = ad.attribute_replace_values(dn, other_attribute, other_values);

    return (main_succcess && other_success);
}

void StringOtherEdit::on_other_button() {
    auto dialog = new ListAttributeDialog(other_values, other_attribute, read_only, other_button);
    dialog->setWindowTitle(tr("Edit other values"));
    dialog->open();

    connect(
        dialog, &QDialog::accepted,
        this,
        [this, dialog]() {
            other_values = dialog->get_value_list();

            emit edited();
        });
}
