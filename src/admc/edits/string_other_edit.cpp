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

#include "edits/string_other_edit.h"
#include "edits/string_edit.h"
#include "edit_dialogs/string_multi_edit_dialog.h"
#include "utils.h"
#include "ad_interface.h"
#include "ad_config.h"

#include <QLineEdit>
#include <QGridLayout>
#include <QMessageBox>
#include <QLabel>
#include <QPushButton>

StringOtherEdit::StringOtherEdit(const QString &main_attribute, const QString &other_attribute_arg, const QString &object_class, QObject *parent, QList<AttributeEdit *> *edits_out)
: AttributeEdit(parent)
, other_attribute(other_attribute_arg)
{
    main_edit = new StringEdit(main_attribute, object_class, parent);
    // NOTE: connect to main_edit's label to display changes for
    // "other" attribute through it
    connect_changed_marker(main_edit->label);

    QObject::connect(
        main_edit, &AttributeEdit::edited,
        [this]() {
            emit edited();
        });

    other_button = new QPushButton(tr("Other..."));
    connect(other_button, &QPushButton::clicked,
        [this]() {
            auto dialog = new StringMultiEditDialog(other_attribute, current_other_values);
            dialog->open();

            connect(
                dialog, &QDialog::accepted,
                [this, dialog]() {
                    current_other_values = dialog->get_new_values();

                    emit edited();
                });
        });

    AttributeEdit::append_to_list(edits_out);
}

void StringOtherEdit::load(const AdObject &object) {
    main_edit->load(object);
    
    original_other_values = object.get_bytes_list(other_attribute);
}

void StringOtherEdit::reset() {
    main_edit->reset();

    current_other_values = original_other_values;

    emit edited();
}

void StringOtherEdit::set_read_only(const bool read_only) {

}

void StringOtherEdit::add_to_layout(QGridLayout *layout) {
    QLabel *label = main_edit->label;
    QLineEdit *edit = main_edit->edit;

    const int row = layout->rowCount();
    layout->addWidget(label, row, 0);
    layout->addWidget(edit, row, 1);
    layout->addWidget(other_button, row, 2);
}

bool StringOtherEdit::verify() const {
    return main_edit->verify();
}

bool StringOtherEdit::changed() const {
    const bool main_changed = main_edit->changed();
    const bool other_changed = (current_other_values != original_other_values);

    return (main_changed || other_changed);
}

bool StringOtherEdit::apply(const QString &dn) const {
    const bool main_succcess = main_edit->apply(dn);
    const bool other_success = AD()->attribute_replace_values(dn, other_attribute, current_other_values);

    return (main_succcess && other_success);
}
