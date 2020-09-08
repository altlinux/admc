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

#include "xml_edit.h"

#include <QStack>

// Look for attribute's node in the document by iterating
// through all nodes and looking for and attribute with same name
QDomNode find_attribute_node(const QDomDocument &doc, const QString &attribute_name) {
    QStack<QDomElement> elements_to_explore;
    const QDomElement top_element = doc.documentElement();
    elements_to_explore.push(top_element);

    while (!elements_to_explore.isEmpty()) {
        const QDomElement element = elements_to_explore.pop();

        QDomNode child = element.firstChild();
        while (!child.isNull()) {
            QDomElement child_as_element = child.toElement();
            const bool is_element = !child_as_element.isNull();

            if (is_element) {
                elements_to_explore.push(child_as_element);
            }

                // Check node's attributes
            const QDomNamedNodeMap attributes = child.attributes();
            const QDomNode attribute_node = attributes.namedItem(attribute_name);
            if (!attribute_node.isNull()) {
                return attribute_node;
            }

            child = child.nextSibling();
        }
    }

    return QDomNode();
}
