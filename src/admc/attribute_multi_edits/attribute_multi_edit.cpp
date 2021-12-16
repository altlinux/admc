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

#include "attribute_multi_edits/attribute_multi_edit.h"

#include <QCheckBox>
#include <QDebug>

AttributeMultiEdit::AttributeMultiEdit(QCheckBox *apply_check_arg, QList<AttributeMultiEdit *> *edit_list, QObject *parent)
: QObject(parent) {
    if (edit_list->contains(this)) {
        qDebug() << "ERROR: attribute edit added twice to list!";
    } else {
        edit_list->append(this);
    }

    apply_check = apply_check_arg;

    connect(
        apply_check, &QAbstractButton::toggled,
        this, &AttributeMultiEdit::on_apply_check);
}

bool AttributeMultiEdit::need_to_apply() const {
    const bool out = apply_check->isChecked();

    return out;
}

void AttributeMultiEdit::uncheck() {
    apply_check->setChecked(false);
}

void AttributeMultiEdit::on_apply_check() {
    if (apply_check->isChecked()) {
        emit edited();
    }

    const bool enabled = apply_check->isChecked();
    set_enabled(enabled);
}
