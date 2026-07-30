/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2026 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
 * Copyright (C) 2026 Artyom V. Poptsov
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

#include "attribute_edits/protect_deletion_edit.h"

#include "adldap.h"
#include "core/globals.h"
#include "utils.h"

#include <QCheckBox>

// Object is protected from deletion if it denies
// permissions for "delete" and "delete subtree" for
// "WORLD"(everyone) trustee

ProtectDeletionEdit::ProtectDeletionEdit(QCheckBox *check_arg, QObject *parent)
: AttributeEdit(parent) {
    check = check_arg;

    connect(
        check, &QCheckBox::stateChanged,
        this, &AttributeEdit::edited);
}

void ProtectDeletionEdit::set_enabled(const bool enabled) {
    check->setChecked(enabled);
}

void ProtectDeletionEdit::load(AdInterface &ad, const AdObject &object) {
    Q_UNUSED(ad);

    const bool enabled = ad_security_get_protected_against_deletion(object);

    check->setChecked(enabled);
}

bool ProtectDeletionEdit::apply(AdInterface &ad, const QString &dn) const {
    const bool enabled = check->isChecked();
    const bool apply_success = ad_security_set_protected_against_deletion(ad, dn, enabled);

    return apply_success;
}
