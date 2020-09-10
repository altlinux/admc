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

#include "xml_ubyte_edit.h"

#include <QLineEdit>
#include <QGridLayout>
#include <QMessageBox>
#include <QLabel>

XmlUByteEdit::XmlUByteEdit(const XmlAttribute &attribute_arg)
: attribute(attribute_arg) {
    edit = new QLineEdit();

    // edit->setInputMask("000");

    QObject::connect(
        edit, &QLineEdit::textChanged,
        [this]() {
            emit edited();
        });
}

void XmlUByteEdit::load(const QDomDocument &doc) {
    const QDomElement parent_element = get_element_by_tag_name(doc, attribute.parent_name());
    const QString value = parent_element.attribute(attribute.name(), QString());

    original_value = value;

    edit->blockSignals(true);
    edit->setText(original_value);
    edit->blockSignals(false);

    emit edited();
}

void XmlUByteEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = attribute.display_string() + ":";
    const auto label = new QLabel(label_text);

    // TODO: connect_changed_marker(this, label);
    
    // TODO: shared usage via append_to_grid_layout_with_label(layout, label, edit);
    const int row = layout->rowCount();
    layout->addWidget(label, row, 0);
    layout->addWidget(edit, row, 1);
}

bool XmlUByteEdit::verify_input(QWidget *parent) {
    const QString new_value = edit->text();
    const int value_int = new_value.toInt();
    const bool is_ubyte = (0 <= value_int && value_int <= 255);
    const bool leading_zeroes = new_value.startsWith("0");

    const bool verified = is_ubyte && !leading_zeroes;

    if (!verified) {
        const QString error = QString(tr("Attribute \"%1\" must have values in range of 0-255 (inclusive).")).arg(attribute.name());
        QMessageBox::warning(parent, tr("Error"), error);
    }

    return verified;
}

bool XmlUByteEdit::changed() const {
    const QString new_value = edit->text();
    return (new_value != original_value);
}

bool XmlUByteEdit::apply(QDomDocument *doc) {
    const QString new_value = edit->text();

    QDomElement parent_element = get_element_by_tag_name(*doc, attribute.parent_name());
    parent_element.setAttribute(attribute.name(), new_value);

    return true;
}
