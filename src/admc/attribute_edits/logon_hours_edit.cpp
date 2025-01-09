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

#include "attribute_edits/logon_hours_edit.h"

#include "adldap.h"
#include "attribute_edits/logon_hours_dialog.h"
#include "utils.h"

#include <QPushButton>

LogonHoursEdit::LogonHoursEdit(QPushButton *button_arg, QObject *parent)
: AttributeEdit(parent) {
    button = button_arg;

    connect(
        button, &QPushButton::clicked,
        this, &LogonHoursEdit::open_dialog);
}

void LogonHoursEdit::load(AdInterface &ad, const AdObject &object) {
    UNUSED_ARG(ad);

    current_value = object.get_value(ATTRIBUTE_LOGON_HOURS);
}

bool LogonHoursEdit::apply(AdInterface &ad, const QString &dn) const {
    const bool success = ad.attribute_replace_value(dn, ATTRIBUTE_LOGON_HOURS, current_value);

    return success;
}

void LogonHoursEdit::open_dialog() {
    auto dialog = new LogonHoursDialog(current_value, button);
    dialog->open();

    connect(
        dialog, &QDialog::accepted,
        this,
        [this, dialog]() {
            const QByteArray new_value = dialog->get();
            const bool value_changed = (new_value != current_value);

            if (value_changed) {
                current_value = dialog->get();

                emit edited();
            }
        });
}
