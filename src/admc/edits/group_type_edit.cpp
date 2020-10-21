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

#include "edits/group_type_edit.h"
#include "utils.h"
#include "ad_interface.h"

#include <QComboBox>
#include <QGridLayout>
#include <QLabel>

GroupTypeEdit::GroupTypeEdit(QObject *parent, QList<AttributeEdit *> *edits_out)
: AttributeEdit(parent)
{
    combo = new QComboBox();

    for (int i = 0; i < GroupType_COUNT; i++) {
        const GroupType type = (GroupType) i;
        const QString type_string = group_type_string(type);

        combo->addItem(type_string, (int)type);
    }

    QObject::connect(
        combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this]() {
            emit edited();
        });

    AttributeEdit::append_to_list(edits_out);
}

void GroupTypeEdit::load(const AdObject &object) {
    original_value = object.get_group_type();

    combo->setCurrentIndex((int) original_value);

    emit edited();
}

void GroupTypeEdit::set_read_only(const bool read_only) {
    combo->setDisabled(read_only);
}

void GroupTypeEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = tr("Group type") + ":";
    const auto label = new QLabel(label_text);

    connect_changed_marker(label);
    append_to_grid_layout_with_label(layout, label, combo);
}

bool GroupTypeEdit::verify() const {
    return true;
}

bool GroupTypeEdit::changed() const {
    const int new_value = combo->currentData().toInt();
    return (new_value != original_value);
}

bool GroupTypeEdit::apply(const QString &dn) const {
    const GroupType new_value = (GroupType)combo->currentData().toInt();
    const bool success = AD()->group_set_type(dn, new_value);

    return success;
}
