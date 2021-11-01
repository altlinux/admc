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
#include "globals.h"

#include <QLabel>

QString AttributeEditor::get_attribute() const {
    return m_attribute;
}

void AttributeEditor::accept() {
    const bool is_read_only = g_adconfig->get_attribute_is_system_only(m_attribute);

    if (is_read_only) {
        QDialog::reject();
    } else {
        QDialog::accept();
    }
}

void AttributeEditor::set_attribute_internal(const QString &attribute, QLabel *attribute_label) {
    m_attribute = attribute;

    const QString text = QString(tr("Attribute: %1")).arg(m_attribute);
    attribute_label->setText(text);
}
