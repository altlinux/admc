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
#include "ad_config.h"
#include "settings.h"

#include <QLineEdit>
#include <QGridLayout>
#include <QMessageBox>
#include <QLabel>

StringEdit *make_string_edit(const QString &attribute, const QString &objectClass, QObject *parent, QMap<QString, StringEdit *> *map_out, QList<AttributeEdit *> *edits_out) {
    const auto edit = new StringEdit(attribute, objectClass, parent);
    map_out->insert(attribute, edit);
    edits_out->append(edit);

    return edit;
}

void make_string_edits(const QList<QString> attributes, const QString &objectClass, QObject *parent, QMap<QString, StringEdit *> *map_out, QList<AttributeEdit *> *edits_out) {
    for (auto attribute : attributes) {
        make_string_edit(attribute, objectClass, parent, map_out, edits_out);
    }
}

void StringEdit::setup_autofill(const QList<StringEdit *> &string_edits) {
    // Get QLineEdit's out of string edits
    QMap<QString, QLineEdit *> edits;
    for (StringEdit *string_edit : string_edits) {
        const QString attribute = string_edit->attribute;
        QLineEdit *edit = string_edit->edit;

        edits[attribute] = edit;
    }

    // Autofill (first name + last name) into full name
    if (edits.contains(ATTRIBUTE_FIRST_NAME) && edits.contains(ATTRIBUTE_LAST_NAME) && edits.contains(ATTRIBUTE_DISPLAY_NAME)) {
        auto autofill =
        [=]() {
            const QString full_name =
            [edits]() {
                const QString first_name = edits[ATTRIBUTE_FIRST_NAME]->text(); 
                const QString last_name = edits[ATTRIBUTE_LAST_NAME]->text();

                const bool last_name_first = SETTINGS()->get_bool(BoolSetting_LastNameBeforeFirstName);
                if (last_name_first) {
                    return last_name + " " + first_name;
                } else {
                    return first_name + " " + last_name;
                }
            }();

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

// "DOMAIN.COM" => "@domain.com"
QString get_domain_as_email_suffix() {
    const QString domain = AD()->domain();
    return "@" + domain.toLower();
}

StringEdit::StringEdit(const QString &attribute_arg, const QString &objectClass_arg, QObject *parent)
: AttributeEdit(parent)
{
    edit = new QLineEdit();
    attribute = attribute_arg;
    objectClass = objectClass_arg;

    const QString label_text = ADCONFIG()->get_attribute_display_name(attribute, objectClass) + ":";
    label = new QLabel(label_text);
    connect_changed_marker(label);

    QObject::connect(
        edit, &QLineEdit::textChanged,
        [this]() {
            emit edited();
        });
}

void StringEdit::load(const AdObject &object) {
    const QString value = object.get_string(attribute);

    if (attribute == ATTRIBUTE_USER_PRINCIPAL_NAME) {
        // Take "user" from "user@domain.com"
        original_value = value.split("@")[0];
    } else {
        original_value = value;
    }
}

void StringEdit::reset() {
    edit->setText(original_value);
}

void StringEdit::set_read_only(const bool read_only) {
    edit->setReadOnly(read_only);
}

void StringEdit::add_to_layout(QGridLayout *layout) {
    if (attribute == ATTRIBUTE_USER_PRINCIPAL_NAME) {
        const QString extra_edit_text = get_domain_as_email_suffix();
        auto extra_edit = new QLineEdit();
        extra_edit->setEnabled(false);
        extra_edit->setText(extra_edit_text);

        const int row = layout->rowCount();
        layout->addWidget(label, row, 0);
        layout->addWidget(edit, row, 1);
        layout->addWidget(extra_edit, row, 2);
    } else if (attribute == ATTRIBUTE_SAMACCOUNT_NAME) {
        const QString domain = AD()->domain();
        const QString domain_name = domain.split(".")[0];
        const QString extra_edit_text = "\\" + domain_name;
        auto extra_edit = new QLineEdit();
        extra_edit->setEnabled(false);
        extra_edit->setText(extra_edit_text);

        const int row = layout->rowCount();
        layout->addWidget(label, row, 0);
        layout->addWidget(extra_edit, row, 1);
        layout->addWidget(edit, row, 2);
    } else {
        append_to_grid_layout_with_label(layout, label, edit);
    }
}

bool StringEdit::verify() const {
    return true;
}

bool StringEdit::changed() const {
    const QString new_value = edit->text();
    return (new_value != original_value);
}

bool StringEdit::apply(const QString &dn) const {
    // NOTE: name can't be replaced regularly so don't apply it. Need to get value from this edit and manually rename/create object
    if (attribute == ATTRIBUTE_NAME) {
        return true;
    }

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
