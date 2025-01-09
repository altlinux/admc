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

#include "attribute_edits/upn_multi_edit.h"

#include "adldap.h"
#include "attribute_edits/upn_suffix_combo.h"
#include "globals.h"

#include <QComboBox>

UpnMultiEdit::UpnMultiEdit(QComboBox *upn_suffix_combo_arg, AdInterface &ad, QObject *parent)
: AttributeEdit(parent) {
    upn_suffix_combo = upn_suffix_combo_arg;
    upn_suffix_combo_init(upn_suffix_combo, ad);
}

bool UpnMultiEdit::apply(AdInterface &ad, const QString &target) const {
    const QString new_value = [&]() {
        const AdObject current_object = ad.search_object(target);
        const QString current_prefix = current_object.get_upn_prefix();
        const QString new_suffix = upn_suffix_combo->currentText();

        return QString("%1@%2").arg(current_prefix, new_suffix);
    }();

    return ad.attribute_replace_string(target, ATTRIBUTE_USER_PRINCIPAL_NAME, new_value);
}

void UpnMultiEdit::set_enabled(const bool enabled) {
    upn_suffix_combo->setEnabled(enabled);
}
