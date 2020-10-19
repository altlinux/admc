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
#include <QPushButton>


UnlockEdit::UnlockEdit(QObject *parent)
: AttributeEdit(parent) {
    button = new QPushButton();
    button->setCheckable(true);
    button->setText(tr("Unlock account"));

    connect(
        button, &QAbstractButton::clicked,
        [this]() {
            emit edited();
        });
}

void UnlockEdit::load(const AdObject &object) {
    reset();
}

void UnlockEdit::reset() {
    button->setChecked(false);
}

void UnlockEdit::set_read_only(const bool read_only) {
    button->setDisabled(read_only);
}

void UnlockEdit::add_to_layout(QGridLayout *layout) {
    const int row = layout->rowCount();
    layout->addWidget(button, row, 0);
}

bool UnlockEdit::verify() const {
    return true;
}

bool UnlockEdit::changed() const {
    return button->isChecked();
}

bool UnlockEdit::apply(const QString &dn) const {
    if (button->isChecked()) {
        const bool result = AD()->user_unlock(dn);
        
        return result;
    } else {
        return true;
    }
}
