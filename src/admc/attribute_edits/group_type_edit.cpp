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

#include "attribute_edits/group_type_edit.h"

#include "adldap.h"
#include "utils.h"

#include <QComboBox>

GroupTypeEdit::GroupTypeEdit(QComboBox *combo_arg, QObject *parent)
: AttributeEdit(parent) {
    combo = combo_arg;

    for (int i = 0; i < GroupType_COUNT; i++) {
        const GroupType type = (GroupType) i;
        const QString type_string = group_type_string(type);

        combo->addItem(type_string, (int) type);
    }

    connect(
        combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &AttributeEdit::edited);
}

void GroupTypeEdit::load(AdInterface &ad, const AdObject &object) {
    UNUSED_ARG(ad);

    const GroupType type = object.get_group_type();

    combo->setCurrentIndex((int) type);

    const bool is_critical_system_object = object.get_bool(ATTRIBUTE_IS_CRITICAL_SYSTEM_OBJECT);

    if (is_critical_system_object) {
        combo->setDisabled(true);
    }
}

bool GroupTypeEdit::apply(AdInterface &ad, const QString &dn) const {
    const GroupType new_value = (GroupType) combo->currentData().toInt();
    const bool success = ad.group_set_type(dn, new_value);

    return success;
}
