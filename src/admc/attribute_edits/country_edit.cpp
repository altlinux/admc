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

#include "attribute_edits/country_edit.h"

#include "adldap.h"
#include "attribute_edits/country_combo.h"
#include "globals.h"
#include "utils.h"

#include <QComboBox>

CountryEdit::CountryEdit(QComboBox *combo_arg, QObject *parent)
: AttributeEdit(parent) {
    combo = combo_arg;

    country_combo_init(combo);

    connect(
        combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &AttributeEdit::edited);
}

void CountryEdit::load(AdInterface &ad, const AdObject &object) {
    UNUSED_ARG(ad);

    country_combo_load(combo, object);
}

bool CountryEdit::apply(AdInterface &ad, const QString &dn) const {
    return country_combo_apply(combo, ad, dn);
}

void CountryEdit::set_enabled(const bool enabled) {
    combo->setEnabled(enabled);
}
