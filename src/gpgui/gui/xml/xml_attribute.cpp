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

#include <QSet>
#include <QHash>

XmlAttribute::XmlAttribute(const QDomNode &node) {
    const QDomElement element = node.toElement();

    m_name = element.attribute("name");

    const QString type_string = element.attribute("type");
    m_type = string_to_attribute_type(type_string);

    auto get_parent_name =
    [node]() -> QString {
        QDomNode current = node.parentNode();
        while (!current.isNull()) {
            const bool is_schema_element = (current.nodeName() == "xs:element");

            if (is_schema_element) {
                const QDomElement current_element = current.toElement();
                const QString name = current_element.attribute("name");

                return name;
            }

            current = current.parentNode();
        }

        return QString();
    };
    m_parent_name = get_parent_name();

    if (m_parent_name.isEmpty()) {
        printf("Failed to find parent name for attribute %s!\n", qPrintable(name()));
    }
}

void XmlAttribute::print() const {
    printf("attribute\n");
    printf("    name=%s\n", qPrintable(m_name));
    printf("    type=%s\n", qPrintable(attribute_type_to_string(m_type)));
}


QString XmlAttribute::name() const {
    return m_name;
}

XmlAttributeType XmlAttribute::type() const {
    return m_type;
}

bool XmlAttribute::hidden() const {
    // TODO: there's gotta be some external resource that says which attributes are hidden
    static const QSet<QString> hidden_attributes = {
        "clsid",
        "disabled",
        "changed",
        "uid",
    };
    const bool is_hidden = hidden_attributes.contains(name());

    return is_hidden;
}

QString XmlAttribute::parent_name() const {
    return m_parent_name;
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

#define DISPLAY_STRING_MACRO(attribute) {attribute, QObject::tr(attribute)},
#define DISPLAY_STRING_MACRO_2(attribute) attribute "_test"

QString XmlAttribute::display_string() const {
    static const QHash<QString, QString> display_strings = {
        {"pidl", QObject::tr("pidl DISPLAY_STRING")},
        {"name", QObject::tr("name DISPLAY_STRING")},
        {"image", QObject::tr("image DISPLAY_STRING")},
        {"targetType", QObject::tr("targetType DISPLAY_STRING")},
        {"action", QObject::tr("action DISPLAY_STRING")},
        {"comment", QObject::tr("comment DISPLAY_STRING")},
        {"shortcutKey", QObject::tr("shortcutKey DISPLAY_STRING")},
        {"startIn", QObject::tr("startIn DISPLAY_STRING")},
        {"arguments", QObject::tr("arguments DISPLAY_STRING")},
        {"iconIndex", QObject::tr("iconIndex DISPLAY_STRING")},
        {"targetPath", QObject::tr("targetPath DISPLAY_STRING")},
        {"iconPath", QObject::tr("iconPath DISPLAY_STRING")},
        {"window", QObject::tr("window DISPLAY_STRING")},
        {"shortcutPath", QObject::tr("shortcutPath DISPLAY_STRING")},
        {"desc", QObject::tr("desc DISPLAY_STRING")},
        {"bypassErrors", QObject::tr("bypassErrors DISPLAY_STRING")},
        {"userContext", QObject::tr("userContext DISPLAY_STRING")},
        {"removePolicy", QObject::tr("removePolicy DISPLAY_STRING")},
    };
    static const QString default_value = QObject::tr("UNKNOWN XML ATTRIBUTE NAME");

    const QString display_string = display_strings.value(name(), default_value);

    return display_string;
}
