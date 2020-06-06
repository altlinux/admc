/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
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

#include "create_entry_dialog.h"
#include "ad_interface.h"

#include <QInputDialog>
#include <QString>

void create_entry_dialog(NewEntryType type, const QString &parent_dn) {
    // Open new user dialog and name of entry from it

    QString type_string = new_entry_type_to_string[type];
    QString dialog_title = "New " + type_string;
    QString input_label = type_string + " name";

    bool ok;
    QString name = QInputDialog::getText(nullptr, dialog_title, input_label, QLineEdit::Normal, "", &ok);

    // TODO: maybe expand tree to newly created entry?

    // Create user once dialog is complete
    if (ok && !name.isEmpty()) {
        // Attempt to create user in AD

        const QMap<NewEntryType, QString> new_entry_type_to_suffix = {
            {NewEntryType::User, "CN"},
            {NewEntryType::Computer, "CN"},
            {NewEntryType::OU, "OU"},
            {NewEntryType::Group, "CN"},
        };
        QString suffix = new_entry_type_to_suffix[type];

        const QString dn = suffix + "=" + name + "," + parent_dn;

        AD()->create_entry(name, dn, type);
    }
}
