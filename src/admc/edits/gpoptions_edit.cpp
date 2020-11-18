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
#include "ad_interface.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QHash>

GpoptionsEdit::GpoptionsEdit(QObject *parent, QList<AttributeEdit *> *edits_out)
: AttributeEdit(edits_out, parent)
{
    check = new QCheckBox();

    QObject::connect(
        check, &QCheckBox::stateChanged,
        [this]() {
            emit edited();
        });
}

void GpoptionsEdit::load_internal(const AdObject &object) {
    const QString value = object.get_string(ATTRIBUTE_GPOPTIONS);
    const bool checked = (value == GPOPTIONS_BLOCK_INHERITANCE);

    check->setChecked(checked);
}

void GpoptionsEdit::set_read_only(const bool read_only) {
    check->setDisabled(read_only);
}

void GpoptionsEdit::add_to_layout(QFormLayout *layout) {
    const QString label_text = tr("Block policy inheritance:");
    layout->addRow(label_text, check);
}

bool GpoptionsEdit::apply(const QString &dn) const {
    const QString new_value =
    [this]() {
        const bool checked = check->isChecked();
        if (checked) {
            return GPOPTIONS_BLOCK_INHERITANCE;
        } else {
            return GPOPTIONS_INHERIT;
        }
    }();
    const bool success = AD()->attribute_replace_string(dn, ATTRIBUTE_GPOPTIONS, new_value);

    return success;
}
