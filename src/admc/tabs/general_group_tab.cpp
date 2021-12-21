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

#include "tabs/general_group_tab.h"
#include "tabs/ui_general_group_tab.h"

#include "adldap.h"
#include "attribute_edits/group_scope_edit.h"
#include "attribute_edits/group_type_edit.h"
#include "attribute_edits/sam_name_edit.h"
#include "attribute_edits/string_edit.h"
#include "tabs/general_other_tab.h"

GeneralGroupTab::GeneralGroupTab(const AdObject &object, QList<AttributeEdit *> *edit_list, QWidget *parent)
: QWidget(parent) {
    ui = new Ui::GeneralGroupTab();
    ui->setupUi(this);

    load_name_label(ui->name_label, object);

    new SamNameEdit(ui->sam_name_edit, ui->sam_name_domain_edit, this);
    new StringEdit(ui->description_edit, ATTRIBUTE_DESCRIPTION, this);
    new StringEdit(ui->email_edit, ATTRIBUTE_MAIL, this);
    new StringEdit(ui->notes_edit, ATTRIBUTE_INFO, this);

    auto scope_edit = new GroupScopeEdit(ui->scope_combo, this);
    auto type_edit = new GroupTypeEdit(ui->type_combo, this);

    const bool is_critical_system_object = object.get_bool(ATTRIBUTE_IS_CRITICAL_SYSTEM_OBJECT);
    if (is_critical_system_object) {
        scope_edit->set_read_only(true);
        type_edit->set_read_only(true);
    }
}

GeneralGroupTab::~GeneralGroupTab() {
    delete ui;
}
