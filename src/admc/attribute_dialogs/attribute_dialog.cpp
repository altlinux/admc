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

#include "attribute_dialogs/attribute_dialog.h"

#include "ad_config.h"
#include "ad_utils.h"

#include "ad_display.h"
#include "globals.h"
#include "attribute_dialogs/bool_attribute_dialog.h"
#include "attribute_dialogs/datetime_attribute_dialog.h"
#include "attribute_dialogs/list_attribute_dialog.h"
#include "attribute_dialogs/number_attribute_dialog.h"
#include "attribute_dialogs/octet_attribute_dialog.h"
#include "attribute_dialogs/string_attribute_dialog.h"
#include "attribute_dialogs/number_attribute_dialog.h"
#include "attribute_dialogs/hex_number_attribute_dialog.h"
#include "attribute_dialogs/time_span_attribute_dialog.h"


#include <QLabel>
#include <QDebug>

AttributeDialog *AttributeDialog::make(const QString &attribute, const QList<QByteArray> &value_list, const bool read_only, const bool single_valued, QWidget *parent) {
    // Single/multi valued logic is separated out of the
    // switch statement for better flow
    auto octet_attribute_dialog = [&]() -> AttributeDialog * {
        if (single_valued) {
            return new OctetAttributeDialog(value_list, attribute, read_only, parent);
        } else {
            return new ListAttributeDialog(value_list, attribute, read_only, parent);
        }
    };

    auto string_attribute_dialog = [&]() -> AttributeDialog * {
        if (single_valued) {
            const bool attribute_is_number = g_adconfig->get_attribute_is_number(attribute);

            if (attribute_is_number) {
                if (attribute_value_is_hex_displayed(attribute))
                    return new HexNumberAttributeDialog(value_list, attribute, read_only, parent);
                else
                    return new NumberAttributeDialog(value_list, attribute, read_only, parent);
            } else {
                return new StringAttributeDialog(value_list, attribute, read_only, parent);
            }
        } else {
            return new ListAttributeDialog(value_list, attribute, read_only, parent);
        }
    };

    auto bool_attribute_dialog = [&]() -> AttributeDialog * {
        if (single_valued) {
            return new BoolAttributeDialog(value_list, attribute, read_only, parent);
        } else {
            return new ListAttributeDialog(value_list, attribute, read_only, parent);
        }
    };

    auto datetime_attribute_dialog = [&]() -> AttributeDialog * {
        if (single_valued) {
            return new DatetimeAttributeDialog(value_list, attribute, read_only, parent);
        } else {
            return nullptr;
        }
    };

    auto time_span_attribute_dialog = [&]() -> AttributeDialog * {
        if (single_valued) {
            return new TimeSpanAttributeDialog(value_list, attribute, read_only, parent);
        } else {
            return nullptr;
        }
    };

    AttributeType type = g_adconfig->get_attribute_type(attribute);
    const LargeIntegerSubtype large_int_subtype = g_adconfig->get_attribute_large_integer_subtype(attribute);
    if (type == AttributeType_LargeInteger && large_int_subtype == LargeIntegerSubtype_Datetime)
        type = AttributeType_UTCTime;

    AttributeDialog *dialog = [&]() -> AttributeDialog * {
        if (type == AttributeType_LargeInteger && large_int_subtype == LargeIntegerSubtype_Timespan) {
            return time_span_attribute_dialog();
        }

        switch (type) {
            case AttributeType_Octet: return octet_attribute_dialog();
            case AttributeType_Sid: return octet_attribute_dialog();

            case AttributeType_Boolean: return bool_attribute_dialog();

            case AttributeType_Unicode: return string_attribute_dialog();
            case AttributeType_StringCase: return string_attribute_dialog();
            case AttributeType_DSDN: return string_attribute_dialog();
            case AttributeType_IA5: return string_attribute_dialog();
            case AttributeType_Teletex: return string_attribute_dialog();
            case AttributeType_ObjectIdentifier: return string_attribute_dialog();
            case AttributeType_Integer: return string_attribute_dialog();
            case AttributeType_Enumeration: return string_attribute_dialog();
            case AttributeType_LargeInteger: return string_attribute_dialog();
            case AttributeType_UTCTime: return datetime_attribute_dialog();
            case AttributeType_GeneralizedTime: return datetime_attribute_dialog();
            case AttributeType_NTSecDesc: return octet_attribute_dialog();
            case AttributeType_Numeric: return string_attribute_dialog();
            case AttributeType_Printable: return string_attribute_dialog();
            case AttributeType_DNString: return string_attribute_dialog();

            // NOTE: putting these here as confirmed to be unsupported
            case AttributeType_ReplicaLink: return nullptr;
            case AttributeType_DNBinary: return nullptr;
        }

        return nullptr;
    }();

    const QString title = [&]() {
        const QString title_action = [&]() {
            if (read_only) {
                return tr("View");
            } else {
                return tr("Edit");
            }
        }();

        const QString title_attribute = attribute_type_display_string(type);

        if (single_valued) {
            return QString("%1 %2").arg(title_action, title_attribute);
        } else {
            return QString(tr("%1 Multi-Valued %2", "This is a dialog title for attribute editors. Example: \"Edit Multi-Valued String\"")).arg(title_action, title_attribute);
        }
    }();

    if (dialog != nullptr) {
        dialog->setWindowTitle(title);
    }

    return dialog;
}

AttributeDialog::AttributeDialog(const QString &attribute, const bool read_only, QWidget *parent)
: QDialog(parent) {
    m_attribute = attribute;
    m_read_only = read_only;
}

QString AttributeDialog::get_attribute() const {
    return m_attribute;
}

bool AttributeDialog::get_read_only() const {
    return m_read_only;
}

void AttributeDialog::load_attribute_label(QLabel *attribute_label) {
    const QString text = QString(tr("Attribute: %1")).arg(m_attribute);
    attribute_label->setText(text);
}
