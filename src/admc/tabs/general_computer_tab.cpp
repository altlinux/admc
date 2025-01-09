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

#include "tabs/general_computer_tab.h"
#include "tabs/ui_general_computer_tab.h"

#include "adldap.h"
#include "attribute_edits/computer_sam_name_edit.h"
#include "attribute_edits/general_name_edit.h"
#include "attribute_edits/string_edit.h"

GeneralComputerTab::GeneralComputerTab(QList<AttributeEdit *> *edit_list, QWidget *parent)
: QWidget(parent) {
    ui = new Ui::GeneralComputerTab();
    ui->setupUi(this);

    auto name_edit = new GeneralNameEdit(ui->name_label, this);
    auto sam_name_edit = new ComputerSamNameEdit(ui->sam_name_edit, ui->sam_name_domain_edit, this);
    auto dns_edit = new StringEdit(ui->dns_host_name_edit, ATTRIBUTE_DNS_HOST_NAME, this);
    auto description_edit = new StringEdit(ui->description_edit, ATTRIBUTE_DESCRIPTION, this);
    auto location_edit = new StringEdit(ui->location_edit, ATTRIBUTE_LOCATION, this);

    sam_name_edit->set_enabled(false);
    dns_edit->set_enabled(false);

    edit_list->append({
        name_edit,
        sam_name_edit,
        dns_edit,
        description_edit,
        location_edit,
    });
}

GeneralComputerTab::~GeneralComputerTab() {
    delete ui;
}
