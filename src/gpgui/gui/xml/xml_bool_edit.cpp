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

#include "xml_bool_edit.h"

#include <QGridLayout>
#include <QMessageBox>
#include <QLabel>
#include <QCheckBox>

XmlBoolEdit::XmlBoolEdit(const XmlAttribute &attribute_arg)
: attribute(attribute_arg) {
    check = new QCheckBox();

    QObject::connect(
        check, &QCheckBox::stateChanged,
        [this]() {
            emit edited();
        });
}

void XmlBoolEdit::load(const QDomDocument &doc) {
    const QDomElement parent_element = get_element_by_tag_name(doc, attribute.parent_name());
    const QString value_string = parent_element.attribute(attribute.name(), QString());
    const bool value = (value_string == "1");

    original_value = value;

    Qt::CheckState check_state;
    if (value) {
        check_state = Qt::Checked;
    } else {
        check_state = Qt::Unchecked;
    }

    check->blockSignals(true);
    check->setCheckState(check_state);
    check->blockSignals(false);

    emit edited();
}

void XmlBoolEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = attribute.display_string() + ":";
    const auto label = new QLabel(label_text);

    const int row = layout->rowCount();
    layout->addWidget(label, row, 0);
    layout->addWidget(check, row, 1);
}

bool XmlBoolEdit::verify_input(QWidget *parent) {
    return true;
}

bool XmlBoolEdit::changed() const {
    // TODO: use checkbox_is_checked()
    const bool new_value = (check->checkState() == Qt::Checked);
    return (new_value != original_value);
}

bool XmlBoolEdit::apply(QDomDocument *doc) {
    const bool new_value_bool = (check->checkState() == Qt::Checked);
    const QString new_value = (new_value_bool ? "1" : "0");

    QDomElement parent_element = get_element_by_tag_name(*doc, attribute.parent_name());
    parent_element.setAttribute(attribute.name(), new_value);

    return true;
}
