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

#include "edits/group_scope_edit.h"
#include "utils.h"
#include "ad_interface.h"

#include <QComboBox>
#include <QGridLayout>
#include <QMessageBox>
#include <QLabel>

GroupScopeEdit::GroupScopeEdit(QObject *parent, QList<AttributeEdit *> *edits_out)
: AttributeEdit(parent)
{
    combo = new QComboBox();

    for (int i = 0; i < GroupScope_COUNT; i++) {
        const GroupScope type = (GroupScope) i;
        const QString type_string = group_scope_string(type);

        combo->addItem(type_string, (int)type);
    }

    QObject::connect(
        combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this]() {
            emit edited();
        });

    AttributeEdit::append_to_list(edits_out);
}

void GroupScopeEdit::load(const AdObject &object) {
    original_value = object.get_group_scope();
}

void GroupScopeEdit::reset() {
    combo->setCurrentIndex((int) original_value);
}

void GroupScopeEdit::set_read_only(const bool read_only) {
    combo->setDisabled(read_only);
}

void GroupScopeEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = tr("Group scope") + ":";
    const auto label = new QLabel(label_text);

    connect_changed_marker(label);
    append_to_grid_layout_with_label(layout, label, combo);
}

bool GroupScopeEdit::verify() const {
    return true;
}

bool GroupScopeEdit::changed() const {
    const int new_value = combo->currentData().toInt();
    return (new_value != original_value);
}

bool GroupScopeEdit::apply(const QString &dn) const {
    const GroupScope new_value = (GroupScope)combo->currentData().toInt();
    const bool success = AD()->group_set_scope(dn, new_value);

    return success;
}
