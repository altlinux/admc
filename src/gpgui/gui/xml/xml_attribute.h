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
    bool hidden() const;
    QString display_string() const;
    QString parent_name() const;

private:
    QString m_name;
    XmlAttributeType m_type;
    QString m_parent_name;
};

QString attribute_type_to_string(const XmlAttributeType type);
XmlAttributeType string_to_attribute_type(const QString string_raw);

#endif /* XML_ATTRIBUTE_H */
