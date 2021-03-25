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
#include "editors/multi_editor.h"
#include "utils.h"
#include "ad/adldap.h"
#include "globals.h"
#include <QLineEdit>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>

StringOtherEdit::StringOtherEdit(const QString &main_attribute, const QString &other_attribute_arg, const QString &object_class, QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent)
, other_attribute(other_attribute_arg)
{
    main_edit = new StringEdit(main_attribute, object_class, nullptr, parent);

    QObject::connect(
        main_edit, &AttributeEdit::edited,
        [this]() {
            emit edited();
        });

    other_button = new QPushButton(tr("Other..."));
    connect(other_button, &QPushButton::clicked,
        [this]() {
            auto dialog = new MultiEditor(other_attribute, other_values, other_button);
            dialog->open();

            connect(
                dialog, &QDialog::accepted,
                [this, dialog]() {
                    other_values = dialog->get_new_values();

                    emit edited();
                });
        });
}

void StringOtherEdit::load_internal(const AdObject &object) {
    main_edit->load(object);
    
    other_values = object.get_values(other_attribute);
}

void StringOtherEdit::set_read_only(const bool read_only) {

}

void StringOtherEdit::add_to_layout(QFormLayout *layout) {
    auto sublayout = new QHBoxLayout();
    sublayout->addWidget(main_edit->edit);
    sublayout->addWidget(other_button);

    const QString label_text = adconfig->get_attribute_display_name(main_edit->attribute, main_edit->objectClass) + ":";
    layout->addRow(label_text, sublayout);
}

bool StringOtherEdit::apply(AdInterface &ad, const QString &dn) const {
    const bool main_succcess = main_edit->apply(ad, dn);
    const bool other_success = ad.attribute_replace_values(dn, other_attribute, other_values);

    return (main_succcess && other_success);
}
