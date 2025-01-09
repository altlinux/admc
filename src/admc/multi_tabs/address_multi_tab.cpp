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

#include "multi_tabs/address_multi_tab.h"
#include "ui_address_multi_tab.h"

#include "ad_defines.h"
#include "attribute_edits/country_edit.h"
#include "attribute_edits/string_edit.h"

#include <QHash>

AddressMultiTab::AddressMultiTab(QList<AttributeEdit *> *edit_list, QHash<AttributeEdit *, QCheckBox *> *check_map, QWidget *parent)
: QWidget(parent) {
    ui = new Ui::AddressMultiTab();
    ui->setupUi(this);

    auto po_edit = new StringEdit(ui->po_edit, ATTRIBUTE_PO_BOX, this);
    auto city_edit = new StringEdit(ui->city_edit, ATTRIBUTE_CITY, this);
    auto state_edit = new StringEdit(ui->state_edit, ATTRIBUTE_STATE, this);
    auto postal_edit = new StringEdit(ui->postal_edit, ATTRIBUTE_POSTAL_CODE, this);
    auto country_edit = new CountryEdit(ui->country_combo, this);

    edit_list->append({
        po_edit,
        city_edit,
        state_edit,
        postal_edit,
        country_edit,
    });

    check_map->insert(po_edit, ui->po_check);
    check_map->insert(city_edit, ui->city_check);
    check_map->insert(state_edit, ui->state_check);
    check_map->insert(postal_edit, ui->postal_check);
    check_map->insert(country_edit, ui->country_check);
}

AddressMultiTab::~AddressMultiTab() {
    delete ui;
}
