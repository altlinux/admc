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

#include "tabs/general_computer_tab.h"
#include "tabs/ui_general_computer_tab.h"

#include "adldap.h"
#include "edits/sam_name_edit.h"
#include "edits/string_edit.h"
#include "tabs/general_other_tab.h"

GeneralComputerTab::GeneralComputerTab(const AdObject &object) {
    ui = new Ui::GeneralComputerTab();
    ui->setupUi(this);

    load_name_label(ui->name_label, object);

    auto sam_name_edit = new SamNameEdit(ui->sam_name_edit, ui->sam_name_domain_edit, &edits, this);
    auto dns_edit = new StringEdit(ui->dns_host_name_edit, ATTRIBUTE_DNS_HOST_NAME, &edits, this);
    new StringEdit(ui->description_edit, ATTRIBUTE_DESCRIPTION, &edits, this);
    new StringEdit(ui->location_edit, ATTRIBUTE_LOCATION, &edits, this);

    sam_name_edit->set_read_only(true);
    dns_edit->set_read_only(true);

    edits_connect_to_tab(edits, this);
}

GeneralComputerTab::~GeneralComputerTab() {
    delete ui;
}
