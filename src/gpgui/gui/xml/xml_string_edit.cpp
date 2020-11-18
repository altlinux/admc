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
#include "utils.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QLabel>

XmlStringEdit::XmlStringEdit(const XmlAttribute &attribute_arg, QObject *parent)
: XmlEdit(parent)
, attribute(attribute_arg)
{
    edit = new QLineEdit();

    QObject::connect(
        edit, &QLineEdit::textChanged,
        [this]() {
            emit edited();
        });
}

void XmlStringEdit::load(const QDomDocument &doc) {
    original_value = get_xml_attribute(doc, attribute);

    edit->blockSignals(true);
    edit->setText(original_value);
    edit->blockSignals(false);

    emit edited();
}

void XmlStringEdit::add_to_layout(QGridLayout *layout) {
    add_xml_edit_to_layout(layout, attribute, edit, this);
}

bool XmlStringEdit::verify_input(QWidget *parent) {
    return true;
}

bool XmlStringEdit::modified() const {
    const QString new_value = edit->text();
    return (new_value != original_value);
}

void XmlStringEdit::apply(QDomDocument *doc) {
    const QString new_value = edit->text();

    set_xml_attribute(doc, attribute, new_value);
}
