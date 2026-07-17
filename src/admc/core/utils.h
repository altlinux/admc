/*
 * ADMC - AD Management Center
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

#ifndef CORE_UTILS_H
#define CORE_UTILS_H

#include <QList>

QList<QVariant> string_list_to_variant_list(const QList<QString> &string_list);
QList<QString> variant_list_to_string_list(const QList<QVariant> &variant_list);

QString generate_new_name(const QList<QString> &existing_name_list,
                          const QString &name_base);
bool string_contains_bad_chars(const QString &string, const QString &bad_chars);

#endif
