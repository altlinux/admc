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

#include "xml_attribute.h"

XmlAttribute::XmlAttribute(const QDomNode &node) {
    // NOTE: there are two "attributes", attributes of the object and xml attributes
    const QDomNamedNodeMap attributes = node.attributes();

    const QDomNode attribute_name = attributes.namedItem("name");
    m_name = attribute_name.nodeValue();

    const QDomNode attribute_type = attributes.namedItem("type");
    m_type = string_to_attribute_type(attribute_type.nodeValue());

    const QDomNode attribute_use = attributes.namedItem("use");
    m_required = (attribute_use.nodeValue() == "required");

    // Recurse through all ancestors of node and check if any of them have an attribute "name" set to "Properties"
    // TODO: might actually not be useful? Delete if not
    auto get_is_property =
    [node]() -> bool {
        QDomNode current = node.parentNode();

        while (!current.isNull()) {
            const QDomNamedNodeMap current_attributes = current.attributes();
            const QDomNode name_node = current_attributes.namedItem("name");
            if (!name_node.isNull()) {
                const QString this_name = name_node.nodeValue();

                if (this_name == "Properties") {
                    return true;
                }
            }

            const QDomNode new_current = current.parentNode();

            if (new_current == current) {
                // Reached top node
                return false;
            } else {
                current = new_current;
            }
        }

        return false;
    };
    m_is_property = get_is_property();
}

void XmlAttribute::print() const {
    printf("attribute\n");
    printf("    name=%s\n", qPrintable(m_name));
    printf("    type=%s\n", qPrintable(attribute_type_to_string(m_type)));
    printf("    required=%d\n", m_required);
    printf("    properties=%d\n", m_is_property);
}


QString XmlAttribute::name() const {
    return m_name;
}

XmlAttributeType XmlAttribute::type() const {
    return m_type;
}

bool XmlAttribute::required() const {
    return m_required;
}

bool XmlAttribute::is_property() const {
    return m_is_property;
}

const QHash<XmlAttributeType, QString> attribute_type_to_string_map = {
    {XmlAttributeType_String, "string"},
    {XmlAttributeType_Boolean, "boolean"},
    {XmlAttributeType_UnsignedByte, "unsignedByte"}
};

QString attribute_type_to_string(const XmlAttributeType type) {
    const QString string = attribute_type_to_string_map.value(type, "UNKNOWN_ATTRIBUTE_TYPE");
    return string;
}

XmlAttributeType string_to_attribute_type(const QString string_raw) {
    auto generate_string_to_attribute_type_map =
    []() -> QHash<QString, XmlAttributeType> {
        QHash<QString, XmlAttributeType> result;

        for (auto type : attribute_type_to_string_map.keys()) {
            const QString string = attribute_type_to_string_map[type];
            result.insert(string, type);
        }

        return result;
    };
    static QHash<QString, XmlAttributeType> string_to_attribute_type_map = generate_string_to_attribute_type_map();

    QString string = string_raw;
    if (string.contains("xs:")) {
        string.remove("xs:");
    }

    const XmlAttributeType type = string_to_attribute_type_map.value(string, XmlAttributeType_None);

    return type;
}
