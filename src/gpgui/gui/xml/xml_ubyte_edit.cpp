/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
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

#include <QMessageBox>
#include <QLineEdit>

XmlUByteEdit::XmlUByteEdit(const XmlAttribute &attribute_arg, QObject *parent)
: XmlStringEdit(attribute_arg, parent) {
    edit->setInputMask("000");
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
