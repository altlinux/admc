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

#include "attribute_edits/gpoptions_edit.h"

#include "adldap.h"
#include "utils.h"

#include <QCheckBox>

GpoptionsEdit::GpoptionsEdit(QCheckBox *check_arg, QObject *parent)
: AttributeEdit(parent) {
    check = check_arg;

    connect(
        check, &QCheckBox::stateChanged,
        this, &AttributeEdit::edited);
}

void GpoptionsEdit::load(AdInterface &ad, const AdObject &object) {
    UNUSED_ARG(ad);

    const QString value = object.get_string(ATTRIBUTE_GPOPTIONS);
    const bool checked = (value == GPOPTIONS_BLOCK_INHERITANCE);

    check->setChecked(checked);
}

bool GpoptionsEdit::apply(AdInterface &ad, const QString &dn) const {
    const QString new_value = [this]() {
        const bool checked = check->isChecked();
        if (checked) {
            return GPOPTIONS_BLOCK_INHERITANCE;
        } else {
            return GPOPTIONS_INHERIT;
        }
    }();
    const bool success = ad.attribute_replace_string(dn, ATTRIBUTE_GPOPTIONS, new_value);
    emit gp_options_changed(check->isChecked());

    return success;
}
