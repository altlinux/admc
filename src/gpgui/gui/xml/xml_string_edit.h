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

#ifndef XML_STRING_EDIT_H
#define XML_STRING_EDIT_H

#include "xml_attribute.h"
#include "xml_edit.h"

#include <QString>

class QLineEdit;

class XmlStringEdit : public XmlEdit {
    Q_OBJECT
public:
    QLineEdit *edit;

    XmlStringEdit(const XmlAttribute &attribute_arg, QObject *parent);
    DECL_XML_EDIT_VIRTUALS();

protected:
    XmlAttribute attribute;

private:
    QString original_value;
};

#endif /* XML_STRING_EDIT_H */
