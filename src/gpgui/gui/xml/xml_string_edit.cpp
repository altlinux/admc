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

#include "xml_string_edit.h"

#include <QLineEdit>
#include <QGridLayout>
#include <QMessageBox>
#include <QLabel>

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

XmlStringEdit::XmlStringEdit(const XmlAttribute &attribute_arg)
: attribute(attribute_arg) {
    edit = new QLineEdit();

    QObject::connect(
        edit, &QLineEdit::textChanged,
        [this]() {
            emit edited();
        });
}

void XmlStringEdit::load(const QDomDocument &doc) {
    const QDomElement parent_element = get_element_by_tag_name(doc, attribute.parent_name());
    const QString value = parent_element.attribute(attribute.name(), QString());

    original_value = value;

    edit->blockSignals(true);
    edit->setText(original_value);
    edit->blockSignals(false);

    emit edited();
}

void XmlStringEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = attribute.display_string() + ":";
    const auto label = new QLabel(label_text);

    // TODO: connect_changed_marker(this, label);
    
    // TODO: shared usage via append_to_grid_layout_with_label(layout, label, edit);
    const int row = layout->rowCount();
    layout->addWidget(label, row, 0);
    layout->addWidget(edit, row, 1);
}

bool XmlStringEdit::verify_input(QWidget *parent) {
    return true;
}

bool XmlStringEdit::changed() const {
    const QString new_value = edit->text();
    return (new_value != original_value);
}

bool XmlStringEdit::apply(QDomDocument *doc) {
    if (!changed()) {
        printf("   not applying\n");
        return true;
    }

    const QString new_value = edit->text();

    QDomElement parent_element = get_element_by_tag_name(*doc, attribute.parent_name());
    parent_element.setAttribute(attribute.name(), new_value);

    printf("apply %s: [%s]=>[%s]\n", qPrintable(attribute.name()), qPrintable(original_value), qPrintable(new_value));

    return true;
}
