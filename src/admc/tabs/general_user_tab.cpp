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

#include "tabs/general_user_tab.h"
#include "tabs/ui_general_user_tab.h"

#include "tabs/general_other_tab.h"
#include "adldap.h"
#include "edits/string_edit.h"
#include "edits/string_other_edit.h"

GeneralUserTab::GeneralUserTab(const AdObject &object) {
    ui = new Ui::GeneralUserTab();
    ui->setupUi(this);

    load_name_label(ui->name_label, object);

    new StringEdit(ui->description_edit, ATTRIBUTE_DESCRIPTION, CLASS_USER, &edits, this);
    new StringEdit(ui->first_name_edit, ATTRIBUTE_FIRST_NAME, CLASS_USER, &edits, this);
    new StringEdit(ui->last_name_edit, ATTRIBUTE_LAST_NAME, CLASS_USER, &edits, this);
    new StringEdit(ui->display_name_edit, ATTRIBUTE_DISPLAY_NAME, CLASS_USER, &edits, this);
    new StringEdit(ui->initials_edit, ATTRIBUTE_INITIALS, CLASS_USER, &edits, this);
    new StringEdit(ui->email_edit, ATTRIBUTE_MAIL, CLASS_USER, &edits, this);
    new StringEdit(ui->office_edit, ATTRIBUTE_OFFICE, CLASS_USER, &edits, this);

    new StringOtherEdit(ui->telephone_edit, ui->telephone_button, ATTRIBUTE_TELEPHONE_NUMBER, ATTRIBUTE_TELEPHONE_NUMBER_OTHER, CLASS_USER, &edits, this);
    new StringOtherEdit(ui->web_page_edit, ui->web_page_button, ATTRIBUTE_WWW_HOMEPAGE, ATTRIBUTE_WWW_HOMEPAGE_OTHER, CLASS_USER, &edits, this);

    edits_connect_to_tab(edits, this);
}
