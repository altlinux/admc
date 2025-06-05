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
#include "krb5client.h"

#include <cng-dpapi/cng-dpapi_client.h>

#include <krb5.h>

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
    Q_UNUSED(ad);
    Q_UNUSED(dn);
    return true;
}

void LAPSEncryptedAttributeEdit::set_enabled(const bool enabled) {
    edit->setEnabled(enabled);
}

QJsonDocument LAPSEncryptedAttributeEdit::get_jsondocument_from_attribute_value(AdInterface &ad, const AdObject &object, const QString &attribute_name) const
{
    const QByteArray encrypted_value = object.get_value(attribute_name);

    QJsonDocument document;
    QString value_string;

    QString domain_fqdn = ad.get_domain();

    uint8_t* value = NULL;
    uint32_t value_size = 0;

    const uint8_t *encrypted_value_p = reinterpret_cast<const uint8_t*>(encrypted_value.data() + DATA_OFFSET);
    uint32_t encrypted_value_s = encrypted_value.size() - DATA_OFFSET;

    char* dc = nullptr;
    char* user_name = nullptr;
    char* domain_name = nullptr;

    Krb5Client krb5_client;
    const QString principal_name = krb5_client.current_principal().section('@', 0, 0);
    QByteArray principal_name_bytes = principal_name.toUtf8();
    user_name = principal_name_bytes.data();

    dc = strdup(ad.get_dc().toLocal8Bit().constData());
    if (!dc)
    {
        goto out;
    }

    if (!user_name)
    {
        goto out;
    }

    domain_name = strdup(domain_fqdn.toLocal8Bit().constData());
    if (!domain_name)
    {
        goto out;
    }

    if (ncrypt_unprotect_secret(encrypted_value_p,
                                encrypted_value_s,
                                &value,
                                &value_size,
                                dc,
                                domain_name,
                                user_name) != 0) {
        emit show_error_dialog();

        goto out;
    }

    if (value_size == 0)
    {
        emit show_error_dialog();

        goto out;
    }

    value_string = QString::fromUtf16(reinterpret_cast<const ushort*>(value));

    document = QJsonDocument::fromJson(value_string.toUtf8());

out:
    if (dc) { free(dc); }
    if (user_name) { free(user_name); }
    if (domain_name) { free(domain_name); }

    return document;
}
