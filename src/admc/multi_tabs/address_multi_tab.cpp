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

#include "multi_tabs/address_multi_tab.h"
#include "ui_address_multi_tab.h"

#include "adldap.h"
#include "multi_edits/country_multi_edit.h"
#include "multi_edits/string_multi_edit.h"

AddressMultiTab::AddressMultiTab() {
    ui = new Ui::AddressMultiTab();
    ui->setupUi(this);

    new StringMultiEdit(ui->po_edit, ui->po_check, ATTRIBUTE_PO_BOX, edit_list, this);
    new StringMultiEdit(ui->city_edit, ui->city_check, ATTRIBUTE_CITY, edit_list, this);
    new StringMultiEdit(ui->state_edit, ui->state_check, ATTRIBUTE_STATE, edit_list, this);
    new StringMultiEdit(ui->postal_edit, ui->postal_check, ATTRIBUTE_POSTAL_CODE, edit_list, this);
    new CountryMultiEdit(ui->country_combo, ui->country_check, edit_list, this);

    multi_edits_connect_to_tab(edit_list, this);
}

AddressMultiTab::~AddressMultiTab() {
    delete ui;
}
