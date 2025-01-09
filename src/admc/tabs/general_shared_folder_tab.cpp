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

#include "tabs/general_shared_folder_tab.h"
#include "tabs/ui_general_shared_folder_tab.h"

#include "adldap.h"
#include "attribute_edits/general_name_edit.h"
#include "attribute_edits/string_edit.h"
#include "attribute_edits/string_list_edit.h"

GeneralSharedFolderTab::GeneralSharedFolderTab(QList<AttributeEdit *> *edit_list, QWidget *parent)
: QWidget(parent) {
    ui = new Ui::GeneralSharedFolderTab();
    ui->setupUi(this);

    auto name_edit = new GeneralNameEdit(ui->name_label, this);
    auto description_edit = new StringEdit(ui->description_edit, ATTRIBUTE_DESCRIPTION, this);
    auto keywords_edit = new StringListEdit(ui->keywords_button, ATTRIBUTE_KEYWORDS, this);

    edit_list->append({
        name_edit,
        description_edit,
        keywords_edit,
    });
}

GeneralSharedFolderTab::~GeneralSharedFolderTab() {
    delete ui;
}
