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

#include "tabs/general_ou_tab.h"
#include "tabs/ui_general_ou_tab.h"

#include "tabs/general_other_tab.h"
#include "adldap.h"
#include "edits/string_edit.h"
#include "edits/country_edit.h"

GeneralOUTab::GeneralOUTab(const AdObject &object) {
    ui = new Ui::GeneralOUTab();
    ui->setupUi(this);

    load_name_label(ui->name_label, object);

    new StringEdit(ui->description_edit, ATTRIBUTE_DESCRIPTION, &edits, this);
    new StringEdit(ui->street_edit, ATTRIBUTE_STREET, &edits, this);
    new StringEdit(ui->city_edit, ATTRIBUTE_CITY, &edits, this);
    new StringEdit(ui->state_edit, ATTRIBUTE_STATE, &edits, this);
    new StringEdit(ui->postal_code_edit, ATTRIBUTE_POSTAL_CODE, &edits, this);

    new CountryEdit(ui->country_combo, &edits, this);

    edits_connect_to_tab(edits, this);
}
