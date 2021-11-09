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

#include "multi_tabs/organization_multi_tab.h"
#include "multi_tabs/ui_organization_multi_tab.h"

#include "adldap.h"
#include "attribute_multi_edits/manager_multi_edit.h"
#include "attribute_multi_edits/string_multi_edit.h"

OrganizationMultiTab::OrganizationMultiTab() {
    ui = new Ui::OrganizationMultiTab();
    ui->setupUi(this);

    new StringMultiEdit(ui->title_edit, ui->title_check, ATTRIBUTE_TITLE, edit_list, this);
    new StringMultiEdit(ui->department_edit, ui->department_check, ATTRIBUTE_DEPARTMENT, edit_list, this);
    new StringMultiEdit(ui->company_edit, ui->company_check, ATTRIBUTE_COMPANY, edit_list, this);
    new ManagerMultiEdit(ui->manager_edit, ui->manager_check, edit_list, this);

    multi_edits_connect_to_tab(edit_list, this);
}

OrganizationMultiTab::~OrganizationMultiTab() {
    delete ui;
}
