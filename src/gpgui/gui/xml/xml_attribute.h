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

#ifndef XML_ATTRIBUTE_H
#define XML_ATTRIBUTE_H

#include <QDialog>
#include <QString>
#include <QList>
#include <QDomDocument>

enum XmlAttributeType {
    XmlAttributeType_String,
    XmlAttributeType_Boolean,
    XmlAttributeType_UnsignedByte,
    XmlAttributeType_None
};

class XmlAttribute {
public:
    XmlAttribute(const QDomNode &node);

    void print() const;

    QString name() const;
    XmlAttributeType type() const;
    bool required() const;
    bool is_property() const;

private:
    QString m_name;
    XmlAttributeType m_type;
    bool m_required;
    bool m_is_property;
};

QString attribute_type_to_string(const XmlAttributeType type);
XmlAttributeType string_to_attribute_type(const QString string_raw);

#endif /* XML_ATTRIBUTE_H */
