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

#include "edits/string_multi_edit.h"
#include "attributes_tab_dialogs/attributes_tab_dialog_string_multi.h"
#include "utils.h"
#include "ad_interface.h"
#include "ad_config.h"

#include <QLineEdit>
#include <QGridLayout>
#include <QMessageBox>
#include <QLabel>
#include <QPushButton>

StringMultiEdit::StringMultiEdit(const QString &attribute_arg, const QString &objectClass_arg, QObject *parent)
: AttributeEdit(parent)
{
    edit = new QLineEdit();
    attribute = attribute_arg;
    objectClass = objectClass_arg;

    QObject::connect(
        edit, &QLineEdit::textChanged,
        [this]() {
            emit edited();
        });
}

void StringMultiEdit::load(const AdObject &object) {
    original = object.get_bytes_list(attribute);
}

void StringMultiEdit::reset() {
    current = original;

    load_current_into_edit();
}

void StringMultiEdit::set_read_only(const bool read_only) {
    // NOTE: can't be read only
}

void StringMultiEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = ADCONFIG()->get_attribute_display_name(attribute, objectClass) + ":";
    const auto label = new QLabel(label_text);
    connect_changed_marker(label);

    auto other_button = new QPushButton(tr("Other..."));
    connect(other_button, &QPushButton::clicked,
        [this]() {
            auto dialog = new AttributesTabDialogStringMulti(attribute, get_current());
            dialog->open();

            connect(
                dialog, &QDialog::accepted,
                [this, dialog]() {
                    current = dialog->get_new_values();

                    load_current_into_edit();

                    emit edited();
                });
        });

    const int row = layout->rowCount();
    layout->addWidget(label, row, 0);
    layout->addWidget(edit, row, 1);
    layout->addWidget(other_button, row, 2);
}

bool StringMultiEdit::verify() const {
    return true;
}

bool StringMultiEdit::changed() const {
    return (get_current() != original);
}

bool StringMultiEdit::apply(const QString &dn) const {
    // NOTE: name can't be replaced regularly so don't apply it. Need to get value from this edit and manually rename/create object
    if (attribute == ATTRIBUTE_NAME) {
        return true;
    }

    const bool success = AD()->attribute_replace_values(dn, attribute, current);

    return success;
}

// Returns current values with edit value added in at first position
// Use this instead of directly accessing current
QList<QByteArray> StringMultiEdit::get_current() const {
    QList<QByteArray> current_out = current;

    const QString first_value = edit->text();
    if (!first_value.isEmpty()) {
        current_out[0] = first_value.toUtf8();
    }

    return current_out;
}

void StringMultiEdit::load_current_into_edit() {
    const QString first_value =
    [this]() {
        if (original.isEmpty()) {
            return QString();
        } else {
            return QString::fromUtf8(original.first());
        }
    }();
    edit->setText(first_value);
}
