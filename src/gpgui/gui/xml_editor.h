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

#ifndef XML_EDITOR_H
#define XML_EDITOR_H

#include <QDialog>
#include <QString>
#include <QList>

enum AttributeType {
    AttributeType_String,
    AttributeType_Boolean,
    AttributeType_UnsignedByte,
    AttributeType_None
};

class GpoXmlAttribute {
public:
    QString name;
    AttributeType type;
    bool required;
    bool properties;
};

class XmlEditor final : public QDialog {
Q_OBJECT

public:
    static QList<GpoXmlAttribute> schema_attributes;

    XmlEditor(const QString &path);

    static void load_schema();
};

#endif /* XML_EDITOR_H */
