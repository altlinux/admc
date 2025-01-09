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

#include "tabs/general_group_tab.h"
#include "tabs/ui_general_group_tab.h"

#include "adldap.h"
#include "attribute_edits/general_name_edit.h"
#include "attribute_edits/group_scope_edit.h"
#include "attribute_edits/group_type_edit.h"
#include "attribute_edits/sam_name_edit.h"
#include "attribute_edits/string_edit.h"


GeneralGroupTab::GeneralGroupTab(QList<AttributeEdit *> *edit_list, QWidget *parent)
: QWidget(parent) {
    ui = new Ui::GeneralGroupTab();
    ui->setupUi(this);

    edit_list->append(create_edits());
}

GeneralGroupTab::GeneralGroupTab(QWidget *parent)
: QWidget(parent) {
    ui = new Ui::GeneralGroupTab();
    ui->setupUi(this);

    m_edit_list.append(create_edits());

    ui->name_label->setVisible(false);

    ui->description_edit->setReadOnly(true);
    ui->email_edit->setReadOnly(true);
    ui->notes_edit->setReadOnly(true);
    ui->sam_name_edit->setReadOnly(true);

    ui->scope_combo->setEditable(false);
    ui->type_combo->setEditable(false);
}

GeneralGroupTab::~GeneralGroupTab() {
    delete ui;
}

void GeneralGroupTab::update(AdInterface &ad, const AdObject &object) {
    AttributeEdit::load(m_edit_list, ad, object);
}

QList<AttributeEdit *> GeneralGroupTab::create_edits() {
    auto name_edit = new GeneralNameEdit(ui->name_label, this);
    auto sam_name_edit = new SamNameEdit(ui->sam_name_edit, ui->sam_name_domain_edit, this);
    auto description_edit = new StringEdit(ui->description_edit, ATTRIBUTE_DESCRIPTION, this);
    auto email_edit = new StringEdit(ui->email_edit, ATTRIBUTE_MAIL, this);
    auto notes_edit = new StringEdit(ui->notes_edit, ATTRIBUTE_INFO, this);

    auto scope_edit = new GroupScopeEdit(ui->scope_combo, this);
    auto type_edit = new GroupTypeEdit(ui->type_combo, this);

    QList<AttributeEdit *> edits_out = {
        name_edit,
        sam_name_edit,
        description_edit,
        email_edit,
        notes_edit,
        scope_edit,
        type_edit,
    };

    return edits_out;
}
