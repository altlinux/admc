/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

#include "editors/attribute_editor.h"

#include "ad_config.h"
#include "ad_utils.h"
#include "globals.h"

#include <QLabel>

void AttributeEditor::set_attribute_label(QLabel *attribute_label) {
    m_attribute_label = attribute_label;
}

QString AttributeEditor::get_attribute() const {
    return m_attribute;
}

bool AttributeEditor::get_read_only() const {
    return m_read_only;
}

void AttributeEditor::set_attribute(const QString &attribute) {
    m_attribute = attribute;

    const QString text = QString(tr("Attribute: %1")).arg(m_attribute);
    if (m_attribute_label != nullptr) {
        m_attribute_label->setText(text);
    }

    const QString title = [&]() {
        const AttributeType type = g_adconfig->get_attribute_type(m_attribute);
        const bool single_valued = g_adconfig->get_attribute_is_single_valued(attribute);

        const QString title_action = [&]() {
            if (m_read_only) {
                return tr("Edit");
            } else {
                return tr("View");
            }
        }();

        const QString title_attribute = attribute_type_display_string(type);

        if (single_valued) {
            return QString("%1 %2").arg(title_action, title_attribute);
        } else {
            return QString(tr("%1 Multi-Valued %2", "This is a dialog title for attribute editors. Example: \"Edit Multi-Valued String\"")).arg(title_action, title_attribute);
        }
    }();

    setWindowTitle(title);
}

void AttributeEditor::set_read_only(const bool read_only) {
    m_read_only = read_only;
}
