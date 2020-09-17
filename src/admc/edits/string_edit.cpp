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
#include "attribute_display_strings.h"
#include "utils.h"
#include "ad_interface.h"

#include <QLineEdit>
#include <QGridLayout>
#include <QMessageBox>
#include <QLabel>

void make_string_edits(const QList<QString> attributes, QMap<QString, StringEdit *> *string_edits_out, QList<AttributeEdit *> *edits_out, QObject *parent) {
    for (auto attribute : attributes) {
        auto edit = new StringEdit(attribute, parent);
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

StringEdit::StringEdit(const QString &attribute_arg, QObject *parent)
: AttributeEdit(parent)
{
    edit = new QLineEdit();
    attribute = attribute_arg;

    QObject::connect(
        edit, &QLineEdit::textChanged,
        [this]() {
            emit edited();
        });
}

void StringEdit::set_read_only(EditReadOnly read_only_arg) {
    read_only = read_only_arg;
    edit->setReadOnly(read_only == EditReadOnly_Yes);
}

void StringEdit::load(const QString &dn) {
    QString value;
    if (attribute == ATTRIBUTE_OBJECT_CLASS) {
        // NOTE: object class is multi-valued so need to get the "primary" class
        // TODO: not sure how to get the "primary" attribute, for now just getting the last one. I think what I need to do is get the most "derived" class? and that info should be in the scheme.
        const QList<QString> classes = AdInterface::instance()->attribute_get_multi(dn, attribute);
        value = classes.last();
    } else {
        value = AdInterface::instance()->attribute_get(dn, attribute);
    }

    edit->blockSignals(true);
    edit->setText(value);
    edit->blockSignals(false);

    original_value = value;

    emit edited();
}

void StringEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = get_attribute_display_string(attribute) + ":";
    const auto label = new QLabel(label_text);

    connect_changed_marker(this, label);
    append_to_grid_layout_with_label(layout, label, edit);
}

bool StringEdit::verify_input(QWidget *parent) {
    static const QList<QString> cant_be_empty = {
        ATTRIBUTE_NAME,
        ATTRIBUTE_SAMACCOUNT_NAME
    };

    if (cant_be_empty.contains(attribute)) {
        const QString new_value = edit->text();

        if (new_value.isEmpty()) {
            const QString attribute_string = get_attribute_display_string(attribute);
            const QString error_text = QString(QObject::tr("Attribute \"%1\" cannot be empty!").arg(attribute_string));
            QMessageBox::warning(parent, QObject::tr("Error"), error_text);

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
    const bool success = AdInterface::instance()->attribute_replace(dn, attribute, new_value);

    return success;
}
