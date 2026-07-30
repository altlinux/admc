/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
 * Copyright (C) 2026 Artyom V. Poptsov
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

#include "attribute_edits/general_name_edit.h"

#include "adldap.h"
#include "utils.h"

#include <QLabel>

GeneralNameEdit::GeneralNameEdit(QLabel *label_arg, QObject *parent)
: AttributeEdit(parent) {
    label = label_arg;
}

void GeneralNameEdit::load(AdInterface &ad, const AdObject &object) {
    Q_UNUSED(ad);

    const bool is_gpc = object.is_class(CLASS_GP_CONTAINER);
    QString name_attribute;
    if (is_gpc) {
        name_attribute = ATTRIBUTE_DISPLAY_NAME;
    } else {
        name_attribute = ATTRIBUTE_NAME;
    }
    QString label_text = object.get_string(name_attribute);

    label->setText(label_text);
}
