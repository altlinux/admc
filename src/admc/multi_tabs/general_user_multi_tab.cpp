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

#include "multi_tabs/general_user_multi_tab.h"
#include "multi_tabs/ui_general_user_multi_tab.h"

#include "adldap.h"
#include "attribute_multi_edits/string_multi_edit.h"

GeneralUserMultiTab::GeneralUserMultiTab() {
    ui = new Ui::GeneralUserMultiTab();
    ui->setupUi(this);

    new StringMultiEdit(ui->description_edit, ui->description_check, ATTRIBUTE_DESCRIPTION, edit_list, this);
    new StringMultiEdit(ui->office_edit, ui->office_check, ATTRIBUTE_OFFICE, edit_list, this);
    new StringMultiEdit(ui->mobile_edit, ui->mobile_check, ATTRIBUTE_MOBILE, edit_list, this);
    new StringMultiEdit(ui->fax_edit, ui->fax_check, ATTRIBUTE_FAX_NUMBER, edit_list, this);
    new StringMultiEdit(ui->web_edit, ui->web_check, ATTRIBUTE_WWW_HOMEPAGE, edit_list, this);
    new StringMultiEdit(ui->email_edit, ui->email_check, ATTRIBUTE_MAIL, edit_list, this);

    multi_edits_connect_to_tab(edit_list, this);
}

GeneralUserMultiTab::~GeneralUserMultiTab() {
    delete ui;
}
