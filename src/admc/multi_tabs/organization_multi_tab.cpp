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

#include "multi_tabs/organization_multi_tab.h"
#include "multi_tabs/ui_organization_multi_tab.h"

#include "ad_defines.h"
#include "attribute_edits/manager_edit.h"
#include "attribute_edits/string_edit.h"

OrganizationMultiTab::OrganizationMultiTab(QList<AttributeEdit *> *edit_list, QHash<AttributeEdit *, QCheckBox *> *check_map, QWidget *parent)
: QWidget(parent) {
    ui = new Ui::OrganizationMultiTab();
    ui->setupUi(this);

    auto title_edit = new StringEdit(ui->title_edit, ATTRIBUTE_TITLE, this);
    auto department_edit = new StringEdit(ui->department_edit, ATTRIBUTE_DEPARTMENT, this);
    auto company_edit = new StringEdit(ui->company_edit, ATTRIBUTE_COMPANY, this);
    auto manager_edit = new ManagerEdit(ui->manager_edit, ATTRIBUTE_MANAGER, this);

    edit_list->append({
        title_edit,
        department_edit,
        company_edit,
        manager_edit,
    });

    check_map->insert(title_edit, ui->title_check);
    check_map->insert(department_edit, ui->department_check);
    check_map->insert(company_edit, ui->company_check);
    check_map->insert(manager_edit, ui->manager_check);
}

OrganizationMultiTab::~OrganizationMultiTab() {
    delete ui;
}
