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

#include "tabs/profile_tab.h"
#include "tabs/ui_profile_tab.h"

#include "adldap.h"
#include "attribute_edits/string_edit.h"

ProfileTab::ProfileTab(QList<AttributeEdit *> *edit_list, QWidget *parent)
: QWidget(parent) {
    ui = new Ui::ProfileTab();
    ui->setupUi(this);

    auto profile_path_edit = new StringEdit(ui->profile_path_edit, ATTRIBUTE_PROFILE_PATH, this);
    auto script_path_edit = new StringEdit(ui->script_path_edit, ATTRIBUTE_SCRIPT_PATH, this);
    auto home_dir_edit = new StringEdit(ui->home_dir_edit, ATTRIBUTE_HOME_DIRECTORY, this);

    edit_list->append({
        profile_path_edit,
        script_path_edit,
        home_dir_edit,
    });
}

ProfileTab::~ProfileTab() {
    delete ui;
}
