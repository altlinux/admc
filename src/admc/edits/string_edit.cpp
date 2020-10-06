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
#include "server_configuration.h"

#include <QLineEdit>
#include <QGridLayout>
#include <QMessageBox>
#include <QLabel>

void make_string_edits(const AdObject &object, const QList<QString> attributes, const QString &objectClass, QMap<QString, StringEdit *> *string_edits_out, QList<AttributeEdit *> *edits_out, QObject *parent) {
    for (auto attribute : attributes) {
        auto edit = new StringEdit(object, attribute, objectClass, parent);
        string_edits_out->insert(attribute, edit);
        edits_out->append((AttributeEdit *)edit);
    }
}

void setup_string_edit_autofills(const QMap<QString, StringEdit *> string_edits, const StringEdit *name_edit) {
    // Get QLineEdit's out of string edits
    QMap<QString, QLineEdit *> edits;
    for (auto attribute : string_edits.keys()) {
        edits[attribute] = string_edits[attribute]->edit;
    }

    if (name_edit != nullptr) {
        edits[ATTRIBUTE_NAME] = name_edit->edit;
    }

    // Autofill (first name + last name) into full name
    if (edits.contains(ATTRIBUTE_FIRST_NAME) && edits.contains(ATTRIBUTE_LAST_NAME) && edits.contains(ATTRIBUTE_DISPLAY_NAME)) {
        auto autofill =
        [=]() {
            const QString first_name = edits[ATTRIBUTE_FIRST_NAME]->text(); 
            const QString last_name = edits[ATTRIBUTE_LAST_NAME]->text();
            const QString full_name = first_name + " " + last_name; 

            edits[ATTRIBUTE_DISPLAY_NAME]->setText(full_name);
        };

        QObject::connect(
            edits[ATTRIBUTE_FIRST_NAME], &QLineEdit::textChanged,
            autofill);
        QObject::connect(
            edits[ATTRIBUTE_LAST_NAME], &QLineEdit::textChanged,
            autofill);
    }

    // Autofill name into samaccount name
    if (edits.contains(ATTRIBUTE_NAME) && edits.contains(ATTRIBUTE_SAMACCOUNT_NAME)) {
        QObject::connect(
            edits[ATTRIBUTE_NAME], &QLineEdit::textChanged,
            [=] () {
                edits[ATTRIBUTE_SAMACCOUNT_NAME]->setText(edits[ATTRIBUTE_NAME]->text());
            });
    }
}

StringEdit::StringEdit(const AdObject &object, const QString &attribute_arg, const QString &objectClass_arg, QObject *parent)
: AttributeEdit(parent)
{
    edit = new QLineEdit();
    attribute = attribute_arg;
    objectClass = objectClass_arg;

    original_value =
    [this, object]() {
        if (attribute == ATTRIBUTE_OBJECT_CLASS) {
            // NOTE: object class is multi-valued so need to get the "primary" class
            // TODO: not sure how to get the "primary" attribute, for now just getting the last one. I think what I need to do is get the most "derived" class? and that info should be in the scheme.
            const QList<QString> classes = object.get_strings(ATTRIBUTE_OBJECT_CLASS);
            return classes.last();
        } else {
            return object.get_string(attribute);
        }
    }();
    edit->setText(original_value);

    QObject::connect(
        edit, &QLineEdit::textChanged,
        [this]() {
            emit edited();
        });
}

void StringEdit::set_read_only(const bool read_only) {
    edit->setReadOnly(read_only);
}

void StringEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = get_attribute_display_name(attribute, objectClass) + ":";
    const auto label = new QLabel(label_text);

    connect_changed_marker(label);
    append_to_grid_layout_with_label(layout, label, edit);
}

bool StringEdit::verify(QWidget *parent) {
    static const QList<QString> cant_be_empty = {
        ATTRIBUTE_NAME,
        ATTRIBUTE_SAMACCOUNT_NAME
    };

    if (cant_be_empty.contains(attribute)) {
        const QString new_value = edit->text();

        if (new_value.isEmpty()) {
            const QString attribute_name = get_attribute_display_name(attribute, objectClass);
            const QString error_text = QString(tr("Attribute \"%1\" cannot be empty!").arg(attribute_name));
            QMessageBox::warning(parent, tr("Error"), error_text);

            return false;
        }
    }

    return true;
}

bool StringEdit::changed() const {
    const QString new_value = edit->text();
    return (new_value != original_value);
}

bool StringEdit::apply(const QString &dn) {
    // NOTE: name can't be replaced regularly so don't apply it. Need to get value from this edit and manually rename/create object
    if (attribute == ATTRIBUTE_NAME) {
        return true;
    }

    const QString new_value = edit->text();
    const bool success = AD()->attribute_replace_string(dn, attribute, new_value);

    return success;
}
