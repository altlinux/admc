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

#include "edits/string_other_edit.h"

#include "adldap.h"
#include "editors/multi_editor.h"
#include "edits/string_edit.h"
#include "globals.h"

#include <QLineEdit>
#include <QPushButton>

StringOtherEdit::StringOtherEdit(QLineEdit *line_edit_arg, QPushButton *other_button_arg, const QString &main_attribute, const QString &other_attribute_arg, QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent)
, other_attribute(other_attribute_arg) {
    main_edit = new StringEdit(line_edit_arg, main_attribute, nullptr, parent);
    
    connect(
        main_edit, &AttributeEdit::edited,
        [this]() {
            emit edited();
        });

    other_button = other_button_arg;

    other_editor = new MultiEditor(other_button);
    other_editor->set_attribute(other_attribute_arg);

    connect(
        other_button, &QPushButton::clicked,
        this, &StringOtherEdit::on_other_button);
    connect(
        other_editor, &QDialog::accepted,
        this, &StringOtherEdit::on_other_editor_accepted);
}

void StringOtherEdit::load_internal(AdInterface &ad, const AdObject &object) {
    main_edit->load(ad, object);

    other_values = object.get_values(other_attribute);
}

void StringOtherEdit::set_read_only(const bool read_only) {
    main_edit->set_read_only(read_only);
    other_editor->set_read_only(read_only);
}

bool StringOtherEdit::apply(AdInterface &ad, const QString &dn) const {
    const bool main_succcess = main_edit->apply(ad, dn);
    const bool other_success = ad.attribute_replace_values(dn, other_attribute, other_values);

    return (main_succcess && other_success);
}

void StringOtherEdit::on_other_button() {
    other_editor->set_value_list(other_values);
    other_editor->open();
}

void StringOtherEdit::on_other_editor_accepted() {
    other_values = other_editor->get_value_list();

    emit edited();
}
