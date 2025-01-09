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

#include "attribute_edits/logon_computers_edit.h"

#include "attribute_edits/logon_computers_dialog.h"

#include "adldap.h"
#include "utils.h"

#include <QPushButton>

LogonComputersEdit::LogonComputersEdit(QPushButton *button_arg, QObject *parent)
: AttributeEdit(parent) {
    button = button_arg;

    connect(
        button, &QPushButton::clicked,
        this, &LogonComputersEdit::open_dialog);
}

void LogonComputersEdit::load(AdInterface &ad, const AdObject &object) {
    UNUSED_ARG(ad);

    current_value = object.get_value(ATTRIBUTE_USER_WORKSTATIONS);
}

bool LogonComputersEdit::apply(AdInterface &ad, const QString &dn) const {
    const bool success = ad.attribute_replace_string(dn, ATTRIBUTE_USER_WORKSTATIONS, current_value);

    return success;
}

void LogonComputersEdit::open_dialog() {
    auto dialog = new LogonComputersDialog(current_value, button);
    dialog->open();

    connect(
        dialog, &QDialog::accepted,
        this,
        [this, dialog]() {
            current_value = dialog->get();

            emit edited();
        });
}
