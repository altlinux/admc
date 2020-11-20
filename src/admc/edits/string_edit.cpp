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
#include "ad_interface.h"
#include "ad_utils.h"
#include "ad_config.h"

#include <QLineEdit>
#include <QFormLayout>

void make_string_edits(const QList<QString> attributes, const QString &objectClass, QObject *parent, QList<AttributeEdit *> *edits_out) {
    for (auto attribute : attributes) {
        new StringEdit(attribute, objectClass, parent, edits_out);
    }
}

// "DOMAIN.COM" => "@domain.com"
QString get_domain_as_email_suffix() {
    const QString domain = AD()->domain();
    return "@" + domain.toLower();
}

StringEdit::StringEdit(const QString &attribute_arg, const QString &objectClass_arg, QObject *parent, QList<AttributeEdit *> *edits_out)
: AttributeEdit(edits_out, parent)
{
    attribute = attribute_arg;
    objectClass = objectClass_arg;
    
    edit = new QLineEdit();
    if (ADCONFIG()->attribute_is_number(attribute)) {
        set_line_edit_to_numbers_only(edit);
    }

    QObject::connect(
        edit, &QLineEdit::textChanged,
        [this]() {
            emit edited();
        });
}

void StringEdit::load_internal(const AdObject &object) {
    const QString value =
    [=]() {
        QString out = object.get_string(attribute);
        if (attribute == ATTRIBUTE_USER_PRINCIPAL_NAME) {
            // Take "user" from "user@domain.com"
            out = out.split("@")[0];
        } else if (attribute == ATTRIBUTE_DISTINGUISHED_NAME) {
            out = dn_canonical(out);
        }

        return out;
    }();
    
    edit->setText(value);
}

void StringEdit::set_read_only(const bool read_only) {
    edit->setReadOnly(read_only);
}

void StringEdit::add_to_layout(QFormLayout *layout) {
    const QString label_text = ADCONFIG()->get_attribute_display_name(attribute, objectClass) + ":";
    
    if (attribute == ATTRIBUTE_USER_PRINCIPAL_NAME) {
        const QString extra_edit_text = get_domain_as_email_suffix();
        auto extra_edit = new QLineEdit();
        extra_edit->setEnabled(false);
        extra_edit->setText(extra_edit_text);

        auto sublayout = new QHBoxLayout();
        sublayout->addWidget(edit);
        sublayout->addWidget(extra_edit);

        layout->addRow(label_text, sublayout);
    } else if (attribute == ATTRIBUTE_SAMACCOUNT_NAME) {
        const QString domain = AD()->domain();
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

bool StringEdit::apply(const QString &dn) const {
    const QString new_value =
    [this]() {
        if (attribute == ATTRIBUTE_USER_PRINCIPAL_NAME) {
            // Get "user" from edit and combine into "user@domain.com"
            return edit->text() + get_domain_as_email_suffix();
        } else {
            return edit->text();
        }
    }();

    const bool success = AD()->attribute_replace_string(dn, attribute, new_value);

    return success;
}

QString StringEdit::get_input() const {
    return edit->text();
}

void StringEdit::set_input(const QString &value) {
    edit->setText(value);

    emit edited();
}
