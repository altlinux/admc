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
#include "edit_dialogs/octet_multi_edit_dialog.h"
#include "edit_dialogs/bool_edit_dialog.h"
#include "edit_dialogs/datetime_edit_dialog.h"
#include "ad_config.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QDialogButtonBox>

EditDialog *EditDialog::make(const QString attribute, const QList<QByteArray> values, QWidget *parent) {
    const bool single_valued = ADCONFIG()->get_attribute_is_single_valued(attribute);

    auto octet_dialog =
    [=]() -> EditDialog * {
        if (single_valued) {
            return new OctetEditDialog(attribute, values, parent);
        } else {
            return new OctetMultiEditDialog(attribute, values, parent);
        } 
    };

    auto string_dialog =
    [=]() -> EditDialog * {
        if (single_valued) {
            return new StringEditDialog(attribute, values, parent);
        } else {
            return new StringMultiEditDialog(attribute, values, parent);
        } 
    };

    auto bool_dialog =
    [=]() -> EditDialog * {
        if (single_valued) {
            return new BoolEditDialog(attribute, values, parent);
        } else {
            return nullptr;
        } 
    };

    auto datetime_dialog =
    [=]() -> EditDialog * {
        if (single_valued) {
            return new DateTimeEditDialog(attribute, values, parent);
        } else {
            return nullptr;
        } 
    };

    const AttributeType type = ADCONFIG()->get_attribute_type(attribute);
    switch (type) {
        case AttributeType_Octet: return octet_dialog();
        case AttributeType_Sid: return octet_dialog();

        case AttributeType_Boolean: return bool_dialog();

        case AttributeType_Unicode: return string_dialog();
        case AttributeType_StringCase: return string_dialog();
        case AttributeType_DSDN: return string_dialog();
        case AttributeType_IA5: return string_dialog();
        case AttributeType_Teletex: return string_dialog();
        case AttributeType_ObjectIdentifier: return string_dialog();
        case AttributeType_Integer: return string_dialog();
        case AttributeType_Enumeration: return string_dialog();
        case AttributeType_LargeInteger: return string_dialog();

        case AttributeType_UTCTime: return datetime_dialog();
        case AttributeType_GeneralizedTime: return datetime_dialog();

        // NOTE: putting these here as confirmed to be unsupported
        case AttributeType_DNBinary: return nullptr;

        default: return nullptr;
    }
}

QLabel *EditDialog::make_attribute_label(const QString &attribute) {
    const QString text = QString(tr("Attribute: %1")).arg(attribute);
    auto label = new QLabel(text);

    return label;
}

QDialogButtonBox *EditDialog::make_button_box(const QString attribute) {
    auto button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    const bool system_only = ADCONFIG()->get_attribute_is_system_only(attribute);
    if (system_only) {
        button_box->setEnabled(false);
    }

    connect(
        button_box, &QDialogButtonBox::accepted,
        this, &EditDialog::accept);
    connect(
        button_box, &QDialogButtonBox::rejected,
        this, &EditDialog::reject);

    return button_box;
}
