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

#include "tabs/address_tab.h"
#include "tabs/ui_address_tab.h"

#include "adldap.h"
#include "attribute_edits/country_edit.h"
#include "attribute_edits/string_edit.h"
#include "attribute_edits/string_large_edit.h"

AddressTab::AddressTab(QList<AttributeEdit *> *edit_list, QWidget *parent)
: QWidget(parent) {
    ui = new Ui::AddressTab();
    ui->setupUi(this);

    auto street_edit = new StringLargeEdit(ui->street_edit, ATTRIBUTE_STREET, this);

    auto po_box_edit = new StringEdit(ui->po_box_edit, ATTRIBUTE_PO_BOX, this);
    auto city_edit = new StringEdit(ui->city_edit, ATTRIBUTE_CITY, this);
    auto state_edit = new StringEdit(ui->state_edit, ATTRIBUTE_STATE, this);
    auto postal_code_edit = new StringEdit(ui->postal_code_edit, ATTRIBUTE_POSTAL_CODE, this);

    auto country_edit = new CountryEdit(ui->country_combo, this);

    edit_list->append({
        street_edit,
        po_box_edit,
        city_edit,
        state_edit,
        street_edit,
        postal_code_edit,
        country_edit,
    });
}

AddressTab::~AddressTab() {
    delete ui;
}
