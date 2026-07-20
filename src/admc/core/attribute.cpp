/*
 * ADMC - AD Management Center
 *
 * This file contains procedures for attribute handling and validation.
 *
 * Copyright (C) 2020-2026 BaseALT Ltd.
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

#include <QByteArray>
#include <QList>
#include <QRegularExpression>
#include <QString>

#include "core/attribute.h"
#include "core/utils.h"

int attribute_format_base(const OctetDisplayFormat format) {
    switch (format) {
    case OctetDisplayFormat_Hexadecimal: return 16;
    case OctetDisplayFormat_Binary: return 2;
    case OctetDisplayFormat_Decimal: return 10;
    case OctetDisplayFormat_Octal: return 8;
    }
    return 0;
}

/* Helper procedures. */

QString octet_bytes_to_string(const QByteArray bytes,
                              const OctetDisplayFormat format) {
    QString out;

    for (int i = 0; i < bytes.size(); i++) {
        if (i > 0) {
            out += " ";
        }

        const char byte_char = bytes[i];
        uint8_t byte = (uint8_t) byte_char;

        char buffer[100];

        const int base = attribute_format_base(format);

        itoa((int) byte, buffer, base);

        const QString byte_string_unpadded(buffer);

        int string_length = 0;
        switch (format) {
        case OctetDisplayFormat_Hexadecimal:
            string_length = 2;
            break;
        case OctetDisplayFormat_Binary:
            string_length = 8;
            break;
        case OctetDisplayFormat_Decimal:
        case OctetDisplayFormat_Octal:
            string_length = 3;
            break;
        default:
            // XXX: Can it ever happen?
            string_length = 0;
        }

        // "5" => "005"
        // "f" => "0f"
        const QString byte_string =
            byte_string_unpadded.rightJustified(string_length, '0');

        out += byte_string;
    }

    return out;
}

QByteArray octet_string_to_bytes(const QString string,
                                 const OctetDisplayFormat format) {
    if (string.isEmpty()) {
        return QByteArray();
    }

    const QList<QString> string_split = string.split(" ");

    QByteArray out;

    for (const QString &byte_string_padded : string_split) {
        // NOTE: remove padding because strtol doesn't understand it
        // "005" => "5"
        QString byte_string = byte_string_padded;
        while ((byte_string[0] == '0') && (byte_string.size() > 0)) {
            byte_string.remove(0, 1);
        }

        const QByteArray byte_bytes = byte_string.toLocal8Bit();
        const char *byte_cstr = byte_bytes.constData();
        const int base = attribute_format_base(format);
        const long int byte_li = strtol(byte_cstr, NULL, base);
        const char byte = (char) byte_li;

        out.append(byte);
    }

    return out;
}

/* Value validation procedures. */

/**
 * Check if a value is a proper hexadecimal value.
 */
bool attribute_validate_hexadecimal(const QString& value) {
    const QRegularExpression rx("^([0-9a-f]{2})$");
    return rx.match(value).hasMatch();
}

/**
 * Check if a value is a proper binary value.
 */
bool attribute_validate_binary(const QString& value) {
    const QRegularExpression rx("^([0-1]{8})$");
    return rx.match(value).hasMatch();
}

/**
 * Check if a value is a proper decimal value in the 0..255 range.
 */
bool attribute_validate_decimal(const QString& value) {
    const QRegularExpression rx("^([0-9]{3})$");
    if (! rx.match(value).hasMatch()) {
        return false;
    }

    const int number = value.toInt();
    if ((number < 0) || (number > 255)) {
        return false;
    }

    return true;
}

/**
 * Check if a value is a proper octal value in the 0..377 range.
 */
bool attribute_validate_octal(const QString& value) {
    const QRegularExpression rx("^([0-7]{3})$");

    if (! rx.match(value).hasMatch()) {
        return false;
    }

    const int number = value.toInt();
    if ((number < 0) || (number > 377)) {
        return false;
    }

    return true;
}

/* Attribute validation.  */

typedef bool(*predicate_t)(const QString& value);

/**
 * A hash table that maps display formats and predicates to check their
 * validity.
 */
static QHash<OctetDisplayFormat, predicate_t> validators = {
    {OctetDisplayFormat_Binary,      attribute_validate_binary},
    {OctetDisplayFormat_Decimal,     attribute_validate_decimal},
    {OctetDisplayFormat_Hexadecimal, attribute_validate_hexadecimal},
    {OctetDisplayFormat_Octal,       attribute_validate_octal}
};

/**
 * Validate an input string of values according to the specified format.
 *
 * @param format A format to check the values against.
 * @param input An input string to check.
 * @return True if all values are valid, false if one or more values
 * are not valid.
 */
bool attribute_validate_input(const OctetDisplayFormat format,
                              const QString input) {
    const QList<QString> text_split = input.split(" ");
    predicate_t is_valid = validators[format];
    for (const QString &element : text_split) {
        if (! is_valid(element)) {
            return false;
        }
    }
    return true;
}

/**
 * Check if a value list contain empty values.
 *
 * @param value_list A list of QByteArray values.
 * @return True if the list contains any empty values, false if it is not.
 */
bool does_list_contain_empty_values(const QList<QByteArray> value_list) {
    for (const QByteArray &value : value_list) {
        const QString value_string = QString(value);
        const bool value_is_all_spaces = (value.count(' ') == value.length());
        const bool value_is_empty = value.isEmpty() || value_is_all_spaces;
        if (value_is_empty) {
            return true;
        }
    }
    return false;
}
