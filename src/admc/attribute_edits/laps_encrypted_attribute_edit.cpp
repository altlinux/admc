/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
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

#include "attribute_edits/laps_encrypted_attribute_edit.h"

#include "adldap.h"
#include "globals.h"
#include "utils.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLineEdit>
#include <QTextCodec>

#include <cng-dpapi/cng-dpapi_client.h>

typedef struct laps_header {
    uint64_t password_update_timestamp; // 8 bytes
    uint32_t encrypted_password_size;   // 4 bytes
    uint32_t reserved;                  // 4 bytes
} laps_header_t;

const uint32_t DATA_OFFSET = 16;

LAPSEncryptedAttributeEdit::LAPSEncryptedAttributeEdit(QLineEdit *edit_arg, const QString &attribute_arg, const QString &json_field_arg, QObject *parent)
: AttributeEdit(parent) {
    attribute = attribute_arg;
    json_field = json_field_arg;
    edit = edit_arg;

    if (g_adconfig->get_attribute_is_number(attribute)) {
        set_line_edit_to_decimal_numbers_only(edit);
    }

    limit_edit(edit, attribute);

    connect(
        edit, &QLineEdit::textChanged,
        this, &AttributeEdit::edited);
}

void LAPSEncryptedAttributeEdit::load(AdInterface &ad, const AdObject &object) {
    UNUSED_ARG(ad);

    QJsonDocument json_document = get_jsondocument_from_attribute_value(ad, object, attribute);

    if (json_document.isEmpty() || json_document.isNull())
    {
        return;
    }

    QJsonObject json_object = json_document.object();

    if (json_object.isEmpty())
    {
        return;
    }

    QJsonValue field_value =  json_object.value(json_field);

    if (field_value.isNull() || field_value.isUndefined())
    {
        return;
    }

    edit->setText(field_value.toString());
}

bool LAPSEncryptedAttributeEdit::apply(AdInterface &ad, const QString &dn) const {
    return true;
}

void LAPSEncryptedAttributeEdit::set_enabled(const bool enabled) {
    edit->setEnabled(enabled);
}

QJsonDocument LAPSEncryptedAttributeEdit::get_jsondocument_from_attribute_value(AdInterface &ad, const AdObject &object, const QString &attribute_name) const
{
    const QByteArray encrypted_value = object.get_value(attribute_name);

    QString sam_account_name;
    const QStringList possible_names = ad.client_user().split('@');

    if (possible_names.size() > 0)
    {
        sam_account_name = possible_names[0];
    }

    uint8_t* value = NULL;
    uint32_t value_size = 0;

    if (ncrypt_unprotect_secret(reinterpret_cast<const uint8_t*>(encrypted_value.data() + DATA_OFFSET),
                                encrypted_value.size() - DATA_OFFSET,
                                &value,
                                &value_size,
                                ad.get_dc().toStdString().c_str(),
                                ad.get_domain().toStdString().c_str(),
                                sam_account_name.toStdString().c_str()) != 0) {
        qWarning() << "Unable to decode secret!\n";

        return QJsonDocument();
    }

    QString value_string = QString::fromUtf16(reinterpret_cast<const ushort*>(value));

    // TODO: free(value);

    return QJsonDocument::fromJson(value_string.toUtf8());
}

QByteArray LAPSEncryptedAttributeEdit::create_attribute_value_from_jsondocument(AdInterface &ad, const QJsonDocument *document) const
{
    if (!document)
    {
        return "";
    }

    QString value_string = document->toJson(QJsonDocument::Compact);

    const ushort *utf16_str = value_string.utf16();

    QString sam_account_name;
    const QStringList possible_names = ad.client_user().split('@');

    if (possible_names.size() > 0)
    {
        sam_account_name = possible_names[0];
    }

    uint8_t* security_descriptor = NULL;

    uint32_t value_size = value_string.size() * sizeof(ushort);
    uint8_t* value =static_cast<uint8_t*>(malloc(value_size));

    if (!value)
    {
        return "";
    }

    memcpy(value, utf16_str, value_size);

    uint8_t* encrypted_value = NULL;
    uint32_t encrypted_value_size = 0;

    if (ncrypt_protect_secret(security_descriptor,
                              value,
                              value_size,
                              &encrypted_value,
                              &encrypted_value_size,
                              ad.get_dc().toStdString().c_str(),
                              ad.get_domain().toStdString().c_str(),
                              sam_account_name.toStdString().c_str()) != 0) {
        qWarning() << "Unable to encode secret!\n";

        free(value);

        return "";
    }

    free(value);

    value_size = encrypted_value_size + DATA_OFFSET;
    value = static_cast<uint8_t*>(malloc(value_size));
    if (!value)
    {
        return "";
    }

    uint8_t* header = create_header(encrypted_value_size);
    if (!header)
    {
        return "";
    }

    memcpy(value, header, DATA_OFFSET);
    free(header);

    memcpy(value + DATA_OFFSET, encrypted_value, encrypted_value_size);
    // TODO: free(encrypted_value);

    QByteArray result = QByteArray::fromRawData(reinterpret_cast<const char*>(value), static_cast<int>(value_size));

    free(value);

    return result;
}

uint8_t *LAPSEncryptedAttributeEdit::create_header(uint32_t size) const
{
    laps_header_t header = {};

    header.encrypted_password_size = size;

    header.reserved = 0;

    uint8_t *result = static_cast<uint8_t*>(malloc(sizeof(laps_header_t)));
    if (!result)
    {
        return nullptr;
    }

    memcpy(result, &header, sizeof(laps_header_t));

    return result;
}
