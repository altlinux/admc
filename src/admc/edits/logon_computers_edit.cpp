/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

#include "edits/logon_computers_edit.h"

#include "edits/logon_computers_dialog.h"

#include "adldap.h"
#include "utils.h"

#include <QPushButton>

LogonComputersEdit::LogonComputersEdit(QPushButton *button_arg, QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent) {
    button = button_arg;
    
    dialog = new LogonComputersDialog(button);

    connect(
        button, &QPushButton::clicked,
        dialog, &QDialog::open);
    connect(
        dialog, &LogonComputersDialog::accepted,
        this, &AttributeEdit::edited);
}

void LogonComputersEdit::load_internal(AdInterface &ad, const AdObject &object) {
    UNUSED_ARG(ad);

    QString value = object.get_value(ATTRIBUTE_USER_WORKSTATIONS);
    dialog->load(value);
}

void LogonComputersEdit::set_read_only(const bool read_only) {
    button->setEnabled(read_only);
}

bool LogonComputersEdit::apply(AdInterface &ad, const QString &dn) const {
    const QString new_value = dialog->get();
    const bool success = ad.attribute_replace_string(dn, ATTRIBUTE_USER_WORKSTATIONS, new_value);

    return success;
}
