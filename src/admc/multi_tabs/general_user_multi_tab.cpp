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

#include "multi_tabs/general_user_multi_tab.h"
#include "multi_tabs/ui_general_user_multi_tab.h"

#include "ad_defines.h"
#include "attribute_edits/string_edit.h"

#include <QHash>

GeneralUserMultiTab::GeneralUserMultiTab(QList<AttributeEdit *> *edit_list, QHash<AttributeEdit *, QCheckBox *> *check_map, QWidget *parent)
: QWidget(parent) {
    ui = new Ui::GeneralUserMultiTab();
    ui->setupUi(this);

    auto description_edit = new StringEdit(ui->description_edit, ATTRIBUTE_DESCRIPTION, this);
    auto office_edit = new StringEdit(ui->office_edit, ATTRIBUTE_OFFICE, this);
    auto mobile_edit = new StringEdit(ui->mobile_edit, ATTRIBUTE_MOBILE, this);
    auto fax_edit = new StringEdit(ui->fax_edit, ATTRIBUTE_FAX_NUMBER, this);
    auto homepage_edit = new StringEdit(ui->web_edit, ATTRIBUTE_WWW_HOMEPAGE, this);
    auto mail_edit = new StringEdit(ui->email_edit, ATTRIBUTE_MAIL, this);

    edit_list->append({
        description_edit,
        office_edit,
        mobile_edit,
        fax_edit,
        homepage_edit,
        mail_edit,
    });

    check_map->insert(description_edit, ui->description_check);
    check_map->insert(office_edit, ui->office_check);
    check_map->insert(mobile_edit, ui->mobile_check);
    check_map->insert(fax_edit, ui->fax_check);
    check_map->insert(homepage_edit, ui->web_check);
    check_map->insert(mail_edit, ui->email_check);
}

GeneralUserMultiTab::~GeneralUserMultiTab() {
    delete ui;
}
