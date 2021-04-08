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

#include "edits/string_edit.h"

#include "utils.h"
#include "adldap.h"
#include "globals.h"

#include <QLineEdit>
#include <QFormLayout>

void StringEdit::make_many(const QList<QString> attributes, const QString &objectClass, QList<AttributeEdit *> *edits_out, QObject *parent) {
    for (auto attribute : attributes) {
        new StringEdit(attribute, objectClass, edits_out, parent);
    }
}

StringEdit::StringEdit(const QString &attribute_arg, const QString &objectClass_arg, QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent)
{
    attribute = attribute_arg;
    objectClass = objectClass_arg;
    
    edit = new QLineEdit();
    
    if (g_adconfig->get_attribute_is_number(attribute)) {
        set_line_edit_to_numbers_only(edit);
    }
    
    limit_edit(edit, attribute);

    QObject::connect(
        edit, &QLineEdit::textChanged,
        [this]() {
            emit edited();
        });
}

void StringEdit::load_internal(AdInterface &ad, const AdObject &object) {
    const QString value =
    [=]() {
        const QString raw_value = object.get_string(attribute);

        if (attribute == ATTRIBUTE_DN) {
            return dn_canonical(raw_value);
        } else {
            return raw_value;
        }
    }();
    
    edit->setText(value);
}

void StringEdit::set_read_only(const bool read_only) {
    edit->setReadOnly(read_only);
}

void StringEdit::add_to_layout(QFormLayout *layout) {
    const QString label_text = g_adconfig->get_attribute_display_name(attribute, objectClass) + ":";
    
    if (attribute == ATTRIBUTE_SAMACCOUNT_NAME) {
        const QString domain = g_adconfig->domain();
        
        const QString domain_name = domain.split(".")[0];
        const QString extra_edit_text = "\\" + domain_name;
        auto extra_edit = new QLineEdit();
        extra_edit->setEnabled(false);
        extra_edit->setText(extra_edit_text);

        auto sublayout = new QHBoxLayout();
        sublayout->addWidget(edit);
        sublayout->addWidget(extra_edit);

        layout->addRow(label_text, sublayout);
    } else {
        layout->addRow(label_text, edit);
    }
}

bool StringEdit::apply(AdInterface &ad, const QString &dn) const {
    const QString new_value = edit->text();
    const bool success = ad.attribute_replace_string(dn, attribute, new_value);

    return success;
}

QString StringEdit::get_input() const {
    return edit->text();
}

void StringEdit::set_input(const QString &value) {
    edit->setText(value);

    emit edited();
}

bool StringEdit::is_empty() const {
    const QString text = edit->text();

    return text.isEmpty();
}
