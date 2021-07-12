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

#include "move_object_dialog.h"

#include "adldap.h"
#include "globals.h"
#include "status.h"
#include "utils.h"

MoveObjectDialog::MoveObjectDialog(const QList<QString> &targets_arg, QWidget *parent)
: SelectContainerDialog(parent) {
    targets = targets_arg;

    if (targets.size() == 0) {
        close();

        return;
    }
}

QList<QString> MoveObjectDialog::get_moved_objects() const {
    return moved_objects;
}

void MoveObjectDialog::accept() {
    AdInterface ad;
    if (ad_failed(ad)) {
        close();

        return;
    }

    show_busy_indicator();

    const QString new_parent_dn = get_selected();

    for (const QString &dn : targets) {
        const bool success = ad.object_move(dn, new_parent_dn);

        if (success) {
            moved_objects.append(dn);
        }
    }

    hide_busy_indicator();

    g_status()->display_ad_messages(ad, nullptr);

    QDialog::accept();
}
