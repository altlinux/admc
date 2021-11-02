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

#include "edits/logon_hours_edit.h"

#include "edits/logon_hours_dialog.h"
#include "adldap.h"
#include "utils.h"

#include <QPushButton>

LogonHoursEdit::LogonHoursEdit(QPushButton *button_arg, QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent) {
    button = button_arg;
    dialog = new LogonHoursDialog(button);

    connect(
        button, &QPushButton::clicked,
        dialog, &QDialog::open);
    connect(
        dialog, &LogonHoursDialog::accepted,
        this, &AttributeEdit::edited);
}

void LogonHoursEdit::load_internal(AdInterface &ad, const AdObject &object) {
    UNUSED_ARG(ad);

    const QByteArray value = object.get_value(ATTRIBUTE_LOGON_HOURS);
    dialog->load(value);
}

void LogonHoursEdit::set_read_only(const bool read_only) {
    button->setEnabled(read_only);
}

bool LogonHoursEdit::apply(AdInterface &ad, const QString &dn) const {
    const QByteArray new_value = dialog->get();
    const bool success = ad.attribute_replace_value(dn, ATTRIBUTE_LOGON_HOURS, new_value);

    return success;
}
