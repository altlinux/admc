/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2026 BaseALT Ltd.
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

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <QByteArray>
#include <QList>
#include <QString>

enum OctetDisplayFormat {
    OctetDisplayFormat_Hexadecimal = 0,
    OctetDisplayFormat_Binary,
    OctetDisplayFormat_Decimal,
    OctetDisplayFormat_Octal,
};

enum ListAttributeDialogType {
    ListAttributeDialogType_String,
    ListAttributeDialogType_Octet,
    ListAttributeDialogType_Datetime,
};

int attribute_format_base(const OctetDisplayFormat format);

/* Value validation procedures. */
bool attribute_validate_hexadecimal(const QString& value);
bool attribute_validate_binary(const QString& value);
bool attribute_validate_decimal(const QString& value);
bool attribute_validate_octal(const QString& value);

bool attribute_validate_input(const OctetDisplayFormat format,
                              const QString input);

/* Helper procedures. */
QString octet_bytes_to_string(const QByteArray bytes,
                              const OctetDisplayFormat format);
QByteArray octet_string_to_bytes(const QString string,
                                 const OctetDisplayFormat format);
bool does_list_contain_empty_values(const QList<QByteArray> value_list);
QString bytes_to_string(const QByteArray& bytes,
                        ListAttributeDialogType editor_type);
QByteArray string_to_bytes(const QString& string,
                           ListAttributeDialogType editor_type);

#endif /* ifndef ATTRIBUTE_H */
