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

#include "xml_string_edit.h"

#include <QLineEdit>
#include <QGridLayout>
#include <QMessageBox>
#include <QLabel>

XmlStringEdit::XmlStringEdit(const XmlAttribute &attribute_arg)
: attribute(attribute_arg) {
    edit = new QLineEdit();

    QObject::connect(
        edit, &QLineEdit::textChanged,
        [this]() {
            emit edited();
        });
}

void XmlStringEdit::load(const QDomDocument &doc) {
    const QDomElement parent_element = get_element_by_tag_name(doc, attribute.parent_name());
    const QString value = parent_element.attribute(attribute.name(), QString());

    original_value = value;

    edit->blockSignals(true);
    edit->setText(original_value);
    edit->blockSignals(false);

    emit edited();
}

void XmlStringEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = attribute.display_string() + ":";
    const auto label = new QLabel(label_text);

    // TODO: connect_changed_marker(this, label);
    
    // TODO: shared usage via append_to_grid_layout_with_label(layout, label, edit);
    const int row = layout->rowCount();
    layout->addWidget(label, row, 0);
    layout->addWidget(edit, row, 1);
}

bool XmlStringEdit::verify_input(QWidget *parent) {
    return true;
}

bool XmlStringEdit::changed() const {
    const QString new_value = edit->text();
    return (new_value != original_value);
}

void XmlStringEdit::apply(QDomDocument *doc) {
    const QString new_value = edit->text();

    QDomElement parent_element = get_element_by_tag_name(*doc, attribute.parent_name());
    parent_element.setAttribute(attribute.name(), new_value);
}
