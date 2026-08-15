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

#include <QLineEdit>
#include <QObject>
#include <QRegularExpressionValidator>
#include <QString>
#include <QStringList>

#include "adldap.h"
#include "core/ad.h"
#include "core/settings.h"
#include "core/utils.h"

/**
 * Prohibits leading zeroes.
 */
void line_edit_set_to_decimal_numbers_only(QLineEdit *edit) {
    edit->setValidator(make_decimal_numbers_validator(edit));
}

void line_edit_set_to_hex_numbers_only(QLineEdit *edit) {
    edit->setValidator(
        new QRegularExpressionValidator(QRegularExpression("[0-9a-f]*"), edit));
}

void line_edit_set_to_time_span_format(QLineEdit *edit) {
    QRegularExpression time_span_reg_exp(
        "([0-9]{1,4}:[0-2][0-3]:[0-5][0-9]:[0-5][0-9])|^\\(never\\)&|^\\(none\\)&");
    edit->setValidator(new QRegularExpressionValidator(time_span_reg_exp));
}

/**
 * Setup an auto-fill from one line edit into another, so that when source line
 * edit is edited, input is copied into destination line edit.
 */
void line_edit_setup_autofill(QLineEdit *src, QLineEdit *dest) {
    QObject::connect(
        src, &QLineEdit::textChanged,
        [src, dest]() {
            const QString src_input = src->text();
            dest->setText(src_input);
        });
}

/**
 * (first name + last name) -> full name
 */
void line_edit_setup_full_name_autofill(
    QLineEdit *first_name_edit,
    QLineEdit *last_name_edit,
    QLineEdit *middle_name_edit,
    QLineEdit *full_name_edit)
{
    auto autofill_full_name = [=]() {
        const QString first_name = first_name_edit->text().trimmed();
        const QString last_name = last_name_edit->text().trimmed();
        const QString middle_name = middle_name_edit->text().trimmed();
        const bool last_name_first =
            settings_get_bool(SETTING_last_name_before_first_name);

        QStringList names{first_name, middle_name};
        if (last_name_first) {
            names.push_front(last_name);
        } else {
            names.push_back(last_name);
        }
        names.removeAll(QString(""));
        const QString full_name_value =
            QStringList(names.begin(), names.end()).join(" ");

        full_name_edit->setText(full_name_value);
    };

    QObject::connect(
        first_name_edit, &QLineEdit::textChanged,
        first_name_edit, autofill_full_name);
    QObject::connect(
        last_name_edit, &QLineEdit::textChanged,
        last_name_edit, autofill_full_name);
    QObject::connect(
        middle_name_edit, &QLineEdit::textChanged,
        middle_name_edit, autofill_full_name);
}

void line_edit_limit_edit(QLineEdit *edit, const QString &attribute) {
    const int range_upper = ad_get_range_upper(attribute);

    if (range_upper > 0) {
        edit->setMaxLength(range_upper);
    }
}

