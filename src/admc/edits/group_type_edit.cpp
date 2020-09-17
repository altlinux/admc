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
#include "attribute_display_strings.h"
#include "utils.h"
#include "ad_interface.h"

#include <QComboBox>
#include <QGridLayout>
#include <QLabel>

GroupTypeEdit::GroupTypeEdit(QObject *parent)
: AttributeEdit(parent)
{
    combo = new QComboBox();

    for (int i = 0; i < GroupType_COUNT; i++) {
        const GroupType type = (GroupType) i;
        const QString type_string = group_type_to_string(type);

        combo->addItem(type_string, (int)type);
    }

    QObject::connect(
        combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this]() {
            emit edited();
        });
}

void GroupTypeEdit::set_read_only(EditReadOnly read_only_arg) {
    read_only = read_only_arg;

    combo->setEnabled(read_only == EditReadOnly_No);
}

void GroupTypeEdit::load(const QString &dn) {
    const GroupType type = AdInterface::instance()->group_get_type(dn);
    const int type_int = (int)type;

    combo->blockSignals(true);
    combo->setCurrentIndex(type_int);
    combo->blockSignals(false);

    original_value = type_int;

    emit edited();
}

void GroupTypeEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = QObject::tr("Group type") + ":";
    const auto label = new QLabel(label_text);

    connect_changed_marker(this, label);
    append_to_grid_layout_with_label(layout, label, combo);
}

bool GroupTypeEdit::verify_input(QWidget *parent) {
    return true;
}

bool GroupTypeEdit::changed() const {
    const int new_value = combo->currentData().toInt();
    return (new_value != original_value);
}

bool GroupTypeEdit::apply(const QString &dn) {
    const GroupType new_value = (GroupType)combo->currentData().toInt();
    const bool success = AdInterface::instance()->group_set_type(dn, new_value);

    return success;
}
