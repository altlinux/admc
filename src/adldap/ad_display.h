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
#ifndef ATTRIBUTE_DISPLAY_H
#define ATTRIBUTE_DISPLAY_H

/**
 * Functions for converting raw attribute values (bytes)
 * into strings fit for displaying to user. If no adconfig
 * is given, then raw attribute values are returned.
 */

class AdConfig;
class QString;
class QByteArray;
template <typename T>
class QList;

QString attribute_display_value(const QString &attribute, const QByteArray &value, const AdConfig *adconfig);
QString attribute_display_values(const QString &attribute, const QList<QByteArray> &values, const AdConfig *adconfig);
QString object_sid_display_value(const QByteArray &sid_bytes);
bool attribute_value_is_hex_displayed(const QString &attribute);

#endif /* ATTRIBUTE_DISPLAY_H */
