/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2022 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
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

#include "adldap.h"
#include "attribute_edits/general_name_edit.h"
#include "attribute_edits/string_edit.h"
#include "attribute_edits/string_other_edit.h"

GeneralUserTab::GeneralUserTab(QList<AttributeEdit *> *edit_list, QWidget *parent)
: QWidget(parent) {
    ui = new Ui::GeneralUserTab();
    ui->setupUi(this);

    edit_list->append(create_edits());
}

GeneralUserTab::GeneralUserTab(QWidget *parent)
: QWidget(parent) {
    ui = new Ui::GeneralUserTab();
    ui->setupUi(this);

    m_edit_list = create_edits();

    ui->name_label->setVisible(false);
    ui->description_edit->setReadOnly(true);
    ui->first_name_edit->setReadOnly(true);
    ui->last_name_edit->setReadOnly(true);
    ui->display_name_edit->setReadOnly(true);
    ui->initials_edit->setReadOnly(true);
    ui->email_edit->setReadOnly(true);
    ui->office_edit->setReadOnly(true);
    ui->web_page_edit->setReadOnly(true);
    ui->telephone_edit->setReadOnly(true);
    ui->web_page_button->setVisible(false);
    ui->telephone_button->setVisible(false);
    ui->middle_name_edit->setReadOnly(true);
}

void GeneralUserTab::update(AdInterface &ad, const AdObject &object) {
    AttributeEdit::load(m_edit_list, ad, object);
}

GeneralUserTab::~GeneralUserTab() {
    delete ui;
}

QList<AttributeEdit *> GeneralUserTab::create_edits() {
    auto name_edit = new GeneralNameEdit(ui->name_label, this);
    auto description_edit = new StringEdit(ui->description_edit, ATTRIBUTE_DESCRIPTION, this);
    auto first_name_edit = new StringEdit(ui->first_name_edit, ATTRIBUTE_FIRST_NAME, this);
    auto last_name_edit = new StringEdit(ui->last_name_edit, ATTRIBUTE_LAST_NAME, this);
    auto display_name_edit = new StringEdit(ui->display_name_edit, ATTRIBUTE_DISPLAY_NAME, this);
    auto initials_edit = new StringEdit(ui->initials_edit, ATTRIBUTE_INITIALS, this);
    auto mail_edit = new StringEdit(ui->email_edit, ATTRIBUTE_MAIL, this);
    auto office_edit = new StringEdit(ui->office_edit, ATTRIBUTE_OFFICE, this);
    auto middle_name_edit = new StringEdit(ui->middle_name_edit, ATTRIBUTE_MIDDLE_NAME, this);

    auto telephone_edit = new StringOtherEdit(ui->telephone_edit, ui->telephone_button, ATTRIBUTE_TELEPHONE_NUMBER, ATTRIBUTE_TELEPHONE_NUMBER_OTHER, this);
    auto web_page_edit = new StringOtherEdit(ui->web_page_edit, ui->web_page_button, ATTRIBUTE_WWW_HOMEPAGE, ATTRIBUTE_WWW_HOMEPAGE_OTHER, this);

    QList<AttributeEdit *> edits_out = {
        name_edit,
        description_edit,
        first_name_edit,
        last_name_edit,
        display_name_edit,
        initials_edit,
        mail_edit,
        office_edit,
        telephone_edit,
        web_page_edit,
        middle_name_edit,
    };

    return edits_out;
}
