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
#include "edit_dialogs/octet_edit_dialog.h"
#include "edit_dialogs/bool_edit_dialog.h"
#include "ad_config.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>

// TODO: implement missing types

EditDialog *EditDialog::make(const QString attribute, const QList<QByteArray> values, QWidget *parent) {
    const bool single_valued = ADCONFIG()->get_attribute_is_single_valued(attribute);

    const AttributeType type = ADCONFIG()->get_attribute_type(attribute);

    const bool is_octet = (type == AttributeType_Octet || type == AttributeType_Sid);

    if (type == AttributeType_Boolean) {
        return new BoolEditDialog(attribute, values, parent);
    }

    // TODO: split by type first, then single/multi
    if (single_valued) {
        if (is_octet) {
            return new OctetEditDialog(attribute, values, parent);
        } else {
            return new StringEditDialog(attribute, values, parent);
        }
    } else {
        // TODO: there are multi-valued octet attributes!
        if (is_octet) {
            return nullptr;
        } else {
            return new StringMultiEditDialog(attribute, values, parent);
        }
    }
}

void EditDialog::add_attribute_label(QVBoxLayout *layout, const QString &attribute) {
    auto form = new QFormLayout();
    form->addRow(tr("Attribute:"), new QLabel(attribute));

    layout->addLayout(form);
}
