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

#ifndef LINE_EDIT_UTILS
#define LINE_EDIT_UTILS

#include <QLineEdit>
#include <QString>

void line_edit_set_to_decimal_numbers_only(QLineEdit *edit);
void line_edit_set_to_hex_numbers_only(QLineEdit *edit);
void line_edit_set_to_time_span_format(QLineEdit *edit);
void line_edit_setup_autofill(QLineEdit *src, QLineEdit *dest);
void line_edit_setup_full_name_autofill(
    QLineEdit *first_name_edit,
    QLineEdit *last_name_edit,
    QLineEdit *middle_name_edit,
    QLineEdit *full_name_edit);
void line_edit_limit_edit(QLineEdit *edit, const QString &attribute);

#endif  /* ifndef LINE_EDIT_UTILS */
