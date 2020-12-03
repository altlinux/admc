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

#include "editors/attribute_editor.h"
#include "editors/string_editor.h"
#include "editors/string_multi_editor.h"
#include "editors/octet_editor.h"
#include "editors/octet_multi_editor.h"
#include "editors/bool_editor.h"
#include "editors/datetime_editor.h"
#include "ad_config.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QDialogButtonBox>

AttributeEditor *AttributeEditor::make(const QString attribute, const QList<QByteArray> values, QWidget *parent) {
    const bool single_valued = ADCONFIG()->get_attribute_is_single_valued(attribute);

    auto octet_dialog =
    [=]() -> AttributeEditor * {
        if (single_valued) {
            return new OctetEditor(attribute, values, parent);
        } else {
            return new OctetMultiEditor(attribute, values, parent);
        } 
    };

    auto string_dialog =
    [=]() -> AttributeEditor * {
        if (single_valued) {
            return new StringEditor(attribute, values, parent);
        } else {
            return new StringMultiEditor(attribute, values, parent);
        } 
    };

    auto bool_dialog =
    [=]() -> AttributeEditor * {
        if (single_valued) {
            return new BoolEditor(attribute, values, parent);
        } else {
            return nullptr;
        } 
    };

    auto datetime_dialog =
    [=]() -> AttributeEditor * {
        if (single_valued) {
            return new DateTimeEditor(attribute, values, parent);
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

QLabel *AttributeEditor::make_attribute_label(const QString &attribute) {
    const QString text = QString(tr("Attribute: %1")).arg(attribute);
    auto label = new QLabel(text);

    return label;
}

QDialogButtonBox *AttributeEditor::make_button_box(const QString attribute) {
    auto button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    const bool system_only = ADCONFIG()->get_attribute_is_system_only(attribute);
    if (system_only) {
        button_box->setEnabled(false);
    }

    connect(
        button_box, &QDialogButtonBox::accepted,
        this, &AttributeEditor::accept);
    connect(
        button_box, &QDialogButtonBox::rejected,
        this, &AttributeEditor::reject);

    return button_box;
}
