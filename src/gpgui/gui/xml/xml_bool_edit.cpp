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

#include "xml_bool_edit.h"
#include "utils.h"

#include <QMessageBox>
#include <QLabel>
#include <QCheckBox>

XmlBoolEdit::XmlBoolEdit(const XmlAttribute &attribute_arg, QObject *parent)
: XmlEdit(parent)
, attribute(attribute_arg)
{
    check = new QCheckBox();

    QObject::connect(
        check, &QCheckBox::stateChanged,
        [this]() {
            emit edited();
        });
}

void XmlBoolEdit::load(const QDomDocument &doc) {
    const QString value_string = get_xml_attribute(doc, attribute);
    original_value = (value_string == "1");

    check->blockSignals(true);
    check->setChecked(original_value);
    check->blockSignals(false);

    emit edited();
}

void XmlBoolEdit::add_to_layout(QGridLayout *layout) {
    add_xml_edit_to_layout(layout, attribute, check, this);
}

bool XmlBoolEdit::verify_input(QWidget *parent) {
    return true;
}

bool XmlBoolEdit::modified() const {
    const bool new_value = check->isChecked();
    return (new_value != original_value);
}

void XmlBoolEdit::apply(QDomDocument *doc) {
    const bool new_value_bool = check->isChecked();
    const QString new_value = (new_value_bool ? "1" : "0");

    set_xml_attribute(doc, attribute, new_value);
}
