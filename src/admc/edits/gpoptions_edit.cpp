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

#include "edits/gpoptions_edit.h"
#include "utils.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QHash>

GpoptionsEdit::GpoptionsEdit(QObject *parent)
: AttributeEdit(parent)
{
    check = new QCheckBox();

    QObject::connect(
        check, &QCheckBox::stateChanged,
        [this]() {
            emit edited();
        });
}

void GpoptionsEdit::load(const QString &dn) {
    const QString value = AdInterface::instance()->attribute_get(dn, ATTRIBUTE_GPOPTIONS);
    const bool checked = (value == GPOPTIONS_BLOCK_INHERITANCE);

    check->blockSignals(true);
    checkbox_set_checked(check, checked);
    check->blockSignals(false);

    original_checked_value = checked;

    emit edited();
}

void GpoptionsEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = tr("Block policy inheritance:");
    const auto label = new QLabel(label_text);

    connect_changed_marker(this, label);
    append_to_grid_layout_with_label(layout, label, check);
}

bool GpoptionsEdit::verify_input(QWidget *parent) {
    return true;
}

bool GpoptionsEdit::changed() const {
    const bool new_checked_value = checkbox_is_checked(check);
    return (new_checked_value != original_checked_value);
}

bool GpoptionsEdit::apply(const QString &dn) {
    const QString new_value =
    [this]() {
        const bool checked = checkbox_is_checked(check);
        if (checked) {
            return GPOPTIONS_BLOCK_INHERITANCE;
        } else {
            return GPOPTIONS_INHERIT;
        }
    }();
    const bool success = AdInterface::instance()->attribute_replace(dn, ATTRIBUTE_GPOPTIONS, new_value);

    return success;
}
