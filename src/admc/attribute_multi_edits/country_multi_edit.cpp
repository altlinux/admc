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

#include "attribute_multi_edits/country_multi_edit.h"

#include "adldap.h"
#include "attribute_edits/country_combo.h"
#include "globals.h"

#include <QCheckBox>
#include <QComboBox>

CountryMultiEdit::CountryMultiEdit(QComboBox *country_combo_arg, QCheckBox *check, QList<AttributeMultiEdit *> *edit_list, QObject *parent)
: AttributeMultiEdit(check, edit_list, parent) {
    country_combo = country_combo_arg;
}

bool CountryMultiEdit::apply(AdInterface &ad, const QString &target) {
    return country_combo_apply(country_combo, ad, target);
}

void CountryMultiEdit::set_enabled(const bool enabled) {
    country_combo->setEnabled(enabled);
}
