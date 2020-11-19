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

#include "edit_dialogs/edit_dialog.h"
#include "edit_dialogs/string_edit_dialog.h"
#include "edit_dialogs/string_multi_edit_dialog.h"
#include "edit_dialogs/binary_edit_dialog.h"
#include "ad_config.h"

EditDialog *EditDialog::make(const QString attribute, const QList<QByteArray> values) {
    const bool single_valued = ADCONFIG()->get_attribute_is_single_valued(attribute);

    const bool is_binary =
    [attribute]() {
        static const QList<AttributeType> binary_types = {
            AttributeType_Octet,
            AttributeType_Sid,
        };
        const AttributeType type = ADCONFIG()->get_attribute_type(attribute);

        return binary_types.contains(type);
    }();

    if (single_valued) {
        if (is_binary) {
            return new BinaryEditDialog(attribute, values);
        } else {
            return new StringEditDialog(attribute, values);
        }
    } else {
        // TODO: are there multi-valued binary attributes?
        if (is_binary) {
            return nullptr;
        } else {
            return new StringMultiEditDialog(attribute, values);
        }
    }
}
