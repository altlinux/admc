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
#include "utils.h"

#include <QDomDocument>
#include <QLabel>

void add_xml_edit_to_layout(QGridLayout *layout, const XmlAttribute &attribute, QWidget *widget, XmlEdit *edit) {
    const QString label_text = attribute.display_string() + ":";
    const auto label = new QLabel(label_text);

    append_to_grid_layout_with_label(layout, label, widget);

    QObject::connect(edit, &XmlEdit::edited,
        [=]() {
            const QString current_text = label->text();
            const QString new_text = set_changed_marker(current_text, edit->changed());
            label->setText(new_text);
        });
}

// TODO: there's probably a better place for this
QDomElement get_element_by_tag_name(const QDomDocument &doc, const QString &tag_name) {
    const QDomNodeList parents = doc.elementsByTagName(tag_name);
    const QDomNode parent_node = parents.at(0);
    const QDomElement parent_element = parent_node.toElement();

    // NOTE: should never happen? as long as xml files are validated on load at least
    if (parent_element.isNull()) {
        printf("get_element_by_tag_name() failed to find element \"%s\"\n", qPrintable(tag_name));
    }

    return parent_element;
}

void set_xml_attribute(QDomDocument *doc, const XmlAttribute &attribute, const QString &value) {
    QDomElement parent_element = get_element_by_tag_name(*doc, attribute.parent_name());
    parent_element.setAttribute(attribute.name(), value);
}

QString get_xml_attribute(const QDomDocument &doc, const XmlAttribute &attribute) {
    const QDomElement parent_element = get_element_by_tag_name(doc, attribute.parent_name());
    const QString value = parent_element.attribute(attribute.name(), QString());

    return value;
}
