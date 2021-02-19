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
#include "ad_object.h"
#include <QFormLayout>
#include <QCheckBox>

UnlockEdit::UnlockEdit(QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent) {
    check = new QCheckBox();

    connect(
        check, &QCheckBox::stateChanged,
        [this]() {
            emit edited();
        });
}

void UnlockEdit::load_internal(const AdObject &object) {
    check->setChecked(false);
}

void UnlockEdit::set_read_only(const bool read_only) {
    check->setDisabled(read_only);
}

void UnlockEdit::add_to_layout(QFormLayout *layout) {
    layout->addRow(tr("Unlock account"), check);
}

bool UnlockEdit::apply(AdInterface &ad, const QString &dn) const {


    if (check->isChecked()) {
        const bool result = ad.user_unlock(dn);
        check->setChecked(false);
        
        return result;
    } else {
        return true;
    }
}
