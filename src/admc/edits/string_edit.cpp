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
#include "filter.h"
#include "ad/ad_interface.h"
#include "ad/ad_utils.h"
#include "ad/ad_config.h"
#include "ad/ad_object.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QMessageBox>

QString get_domain_as_email_suffix();

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
    
    if (ADCONFIG()->get_attribute_is_number(attribute)) {
        set_line_edit_to_numbers_only(edit);
    }
    
    ADCONFIG()->limit_edit(edit, attribute);

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
            const int at_index = out.lastIndexOf('@');
            out = out.left(at_index);
        } else if (attribute == ATTRIBUTE_DN) {
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
        const QString domain = ADCONFIG()->domain();
        
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

bool StringEdit::verify(AdInterface &ad, const QString &dn) const {
    const QString new_value = get_new_value();

    if (attribute == ATTRIBUTE_USER_PRINCIPAL_NAME) {
        // Check that new upn is unique
        // NOTE: filter has to also check that it's not the same object because of attribute edit weirdness. If user edits logon name, then retypes original, then applies, the edit will apply because it was modified by the user, even if the value didn't change. Without "not_object_itself", this check would determine that object's logon name conflicts with itself.
        const QString filter =
        [=]() {
            const QString not_object_itself = filter_CONDITION(Condition_NotEquals, ATTRIBUTE_DN, dn);
            const QString same_upn = filter_CONDITION(Condition_Equals, ATTRIBUTE_USER_PRINCIPAL_NAME, new_value);

            return filter_AND({same_upn, not_object_itself});
        }();
        const QList<QString> search_attributes;
        const QString base = ADCONFIG()->domain_head();

        const QHash<QString, AdObject> search_results = ad.search(filter, search_attributes, SearchScope_All, base);

        const bool upn_not_unique = (search_results.size() > 0);

        if (upn_not_unique) {
            const QString text = tr("The specified user logon name already exists.");
            QMessageBox::warning(edit, tr("Error"), text);
            return false;
        }
    }

    return true;
}

bool StringEdit::apply(AdInterface &ad, const QString &dn) const {
    const QString new_value = get_new_value();
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

QString StringEdit::get_new_value() const {
    if (attribute == ATTRIBUTE_USER_PRINCIPAL_NAME) {
        // Get "user" from edit and combine into "user@domain.com"
        return edit->text() + get_domain_as_email_suffix();
    } else {
        return edit->text();
    }
}

// "DOMAIN.COM" => "@domain.com"
QString get_domain_as_email_suffix() {
    const QString domain = ADCONFIG()->domain();
    return "@" + domain.toLower();
}
