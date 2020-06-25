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

#include "settings.h"

#include <QString>
#include <QMessageBox>
#include <QAction>

bool confirmation_dialog(const QString &text, QWidget *parent) {
    const QAction *confirm_actions = SETTINGS()->confirm_actions;
    const bool confirm_actions_checked = confirm_actions->isChecked();
    if (!confirm_actions_checked) {
        return true;
    }

    const QString title = "ADMC";
    const QMessageBox::StandardButton reply = QMessageBox::question(parent, title, text, QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        return true;
    } else {
        return false;
    }
}
