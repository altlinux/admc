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

#include <QList>
#include <QRegularExpression>
#include <QString>

#include "core/attribute.h"

int attribute_format_base(const OctetDisplayFormat format) {
    switch (format) {
    case OctetDisplayFormat_Hexadecimal: return 16;
    case OctetDisplayFormat_Binary: return 2;
    case OctetDisplayFormat_Decimal: return 10;
    case OctetDisplayFormat_Octal: return 8;
    }
    return 0;
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
