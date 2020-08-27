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

#include "edits/unlock_edit.h"
#include "ad_interface.h"
#include "utils.h"

#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>

// This edit works differently from other account option edits. The checkbox starts out as unchecked, if it's checked, then applying will unlock the user. Unchecking only disables the unlock action, it DOES NOT lock the user. Can't lock the account manually. Also finding out whether user is locked is convoluted, so can't show any status about that.

UnlockEdit::UnlockEdit() {
    check = new QCheckBox();

    connect(
        check, &QCheckBox::stateChanged,
        [this]() {
            emit edited();
        });
}

void UnlockEdit::load(const QString &dn) {
    check->blockSignals(true);
    check->setChecked(false);
    check->blockSignals(false);

    emit edited();
}

void UnlockEdit::add_to_layout(QGridLayout *layout) {
    const auto label = new QLabel(tr("Unlock account:"));

    label->setToolTip(tr("Can only unlock the account, impossible to lock it manually."));

    connect_changed_marker(this, label);
    append_to_grid_layout_with_label(layout, label, check);
}

bool UnlockEdit::verify_input(QWidget *parent) {
    return true;
}

bool UnlockEdit::changed() const {
    return checkbox_is_checked(check);
}

bool UnlockEdit::apply(const QString &dn) {
    if (checkbox_is_checked(check)) {
        const bool result = AdInterface::instance()->user_unlock(dn);
        return result;
    } else {
        return true;
    }
}
