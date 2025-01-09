/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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
    UNUSED_ARG(ad);

    const QString label_text = [&]() {
        const QString name_attribute = [&]() {
            const bool is_gpc = object.is_class(CLASS_GP_CONTAINER);

            if (is_gpc) {
                return ATTRIBUTE_DISPLAY_NAME;
            } else {
                return ATTRIBUTE_NAME;
            }
        }();

        const QString name = object.get_string(name_attribute);

        return name;
    }();

    label->setText(label_text);
}
