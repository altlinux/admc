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

#include "xml_editor.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QFile>
#include <QXmlStreamReader>
#include <QDomDocument>

const QHash<AttributeType, QString> attribute_type_to_string = {
    {AttributeType_String, "string"},
    {AttributeType_Boolean, "boolean"},
    {AttributeType_UnsignedByte, "unsignedByte"}
};

QList<GpoXmlAttribute> XmlEditor::schema_attributes;

AttributeType string_to_attribute_type(const QString string_raw) {
    auto generate_string_to_attribute_type_map =
    []() -> QHash<QString, AttributeType> {
        QHash<QString, AttributeType> result;

        for (auto type : attribute_type_to_string.keys()) {
            const QString string = attribute_type_to_string[type];
            result.insert(string, type);
        }

        return result;
    };
    static QHash<QString, AttributeType> string_to_attribute_type_map = generate_string_to_attribute_type_map();

    QString string = string_raw;
    if (string.contains("xs:")) {
        string.remove("xs:");
    }

    const AttributeType type = string_to_attribute_type_map.value(string, AttributeType_None);

    return type;
}

void XmlEditor::load_schema() {
    static bool loaded = false;
    if (loaded) {
        return;
    } else {
        loaded = true;
    }

    QFile file("../shortcuts_xml_schema.xml");
    const bool open_success = file.open(QIODevice::ReadOnly | QIODevice::Text);
    if (!open_success) {
        printf("Failed to open xml file\n");
        return;
    }

    QDomDocument doc("schema");
    doc.setContent(&file);

    file.close();

    QDomNode root = doc.namedItem("Properties");
    QDomNodeList node_list = root.childNodes();
    for (int i = 0; i < node_list.size(); i++) {
        printf("=%s\n", qPrintable(node_list.at(i).nodeValue()));
    }

    // NOTE: there are two things that are called "attributes" in this context, the nodes which are named "xs:attribute" and the attributes of those nodes
    QDomNodeList attributes = doc.elementsByTagName("xs:attribute");
    for (int i = 0; i < attributes.size(); i++) {
        const QDomNode node = attributes.at(i);

        GpoXmlAttribute gpo_attribute;

        // Recurse through all ancestors of node and check if any of them have an attribute "name" set to "Properties"
        auto is_child_of_properties =
        [node]() -> bool {
            QDomNode current = node.parentNode();

            while (!current.isNull()) {
                const QDomNamedNodeMap node_attributes = current.attributes();
                const QDomNode name_node = node_attributes.namedItem("name");
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

        gpo_attribute.properties = is_child_of_properties();

        const QDomNamedNodeMap xml_attribs = node.attributes();

        const QDomNode xml_name = xml_attribs.namedItem("name");
        gpo_attribute.name = xml_name.nodeValue();

        const QDomNode xml_use = xml_attribs.namedItem("use");
        gpo_attribute.required = (xml_use.nodeValue() == "required");

        QDomNode xml_type = xml_attribs.namedItem("type");
        gpo_attribute.type = string_to_attribute_type(xml_type.nodeValue());

        printf("attribute\n");
        printf("    name=%s\n", qPrintable(gpo_attribute.name));
        printf("    type=%s\n", qPrintable(attribute_type_to_string[gpo_attribute.type]));
        printf("    required=%d\n", gpo_attribute.required);
        printf("    properties=%d\n", gpo_attribute.properties);

        schema_attributes.append(gpo_attribute);
    }
}

XmlEditor::XmlEditor(const QString &path)
: QDialog()
{   
    load_schema();

    setAttribute(Qt::WA_DeleteOnClose);
    resize(300, 600);

    const QString title_label_text = tr("Editing xml file:") + path;
    auto title_label = new QLabel(title_label_text);

    auto button_box = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addWidget(title_label);
    top_layout->addWidget(button_box);

    connect(
        button_box, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
    connect(
        button_box, &QDialogButtonBox::rejected,
        this, &QDialog::reject);
    
    {
        QFile file(path);
        const bool open_success = file.open(QIODevice::ReadOnly | QIODevice::Text);
        if (!open_success) {
            printf("Failed to open xml file\n");
            return;
        }

        const QByteArray file_byte_array = file.readAll();
        const QString file_string(file_byte_array);

        QXmlStreamReader xml(file_string);

        while (!xml.atEnd()) {
            const QXmlStreamReader::TokenType token_type = xml.readNext();

            switch (token_type) {
                case QXmlStreamReader::StartElement: {
                    if (xml.name() == "Properties") {
                        const QXmlStreamAttributes attributes = xml.attributes();

                        for (const QXmlStreamAttribute attribute : attributes) {
                            printf("%s=%s\n", qPrintable(attribute.name().toString()), qPrintable(attribute.value().toString()));
                        }
                    }

                    break;
                }
                default: {
                    break;
                }
            }        
        }
    }
}
