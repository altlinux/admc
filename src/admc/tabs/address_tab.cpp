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

#include "tabs/address_tab.h"
#include "edits/string_edit.h"
#include "edits/country_edit.h"

#include <QVBoxLayout>
#include <QFormLayout>

AddressTab::AddressTab() {
    const QList<QString> attributes = {
        ATTRIBUTE_STREET,
        ATTRIBUTE_PO_BOX,
        ATTRIBUTE_CITY,
        ATTRIBUTE_STATE,
        ATTRIBUTE_POSTAL_CODE
    };

    make_string_edits(attributes, CLASS_USER, this, &edits);

    new CountryEdit(this, &edits);

    edits_connect_to_tab(edits, this);

    const auto layout = new QFormLayout();
    setLayout(layout);
    edits_add_to_layout(edits, layout);
}
