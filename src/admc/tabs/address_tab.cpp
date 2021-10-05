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

#include "tabs/address_tab.h"
#include "tabs/ui_address_tab.h"

#include "adldap.h"
#include "edits/country_edit.h"
#include "edits/string_edit.h"
#include "edits/string_large_edit.h"

AddressTab::AddressTab() {
    ui = new Ui::AddressTab();
    ui->setupUi(this);

    new StringLargeEdit(ui->street_edit, ATTRIBUTE_STREET, CLASS_USER, &edits, this);

    new StringEdit(ui->po_box_edit, ATTRIBUTE_PO_BOX, CLASS_USER, &edits, this);
    new StringEdit(ui->city_edit, ATTRIBUTE_CITY, CLASS_USER, &edits, this);
    new StringEdit(ui->state_edit, ATTRIBUTE_STATE, CLASS_USER, &edits, this);
    new StringEdit(ui->postal_code_edit, ATTRIBUTE_POSTAL_CODE, CLASS_USER, &edits, this);

    new CountryEdit(ui->country_combo, &edits, this);

    edits_connect_to_tab(edits, this);
}
