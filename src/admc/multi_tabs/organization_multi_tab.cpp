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

#include "multi_tabs/organization_multi_tab.h"

#include "adldap.h"
#include "multi_edits/string_multi_edit.h"
#include "multi_edits/manager_multi_edit.h"

#include <QFormLayout>

OrganizationMultiTab::OrganizationMultiTab() {   
    new StringMultiEdit(ATTRIBUTE_TITLE, edit_list, this);
    new StringMultiEdit(ATTRIBUTE_DEPARTMENT, edit_list, this);
    new StringMultiEdit(ATTRIBUTE_COMPANY, edit_list, this);
    new ManagerMultiEdit(edit_list, this);

    auto edit_layout = new QFormLayout();

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addLayout(edit_layout);

    multi_edits_add_to_layout(edit_list, edit_layout);
    multi_edits_connect_to_tab(edit_list, this);
}
