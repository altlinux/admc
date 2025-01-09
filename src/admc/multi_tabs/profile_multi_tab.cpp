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

#include "multi_tabs/profile_multi_tab.h"
#include "ui_profile_multi_tab.h"

#include "ad_defines.h"
#include "attribute_edits/string_edit.h"

#include <QHash>

ProfileMultiTab::ProfileMultiTab(QList<AttributeEdit *> *edit_list, QHash<AttributeEdit *, QCheckBox *> *check_map, QWidget *parent)
: QWidget(parent) {
    ui = new Ui::ProfileMultiTab();
    ui->setupUi(this);

    auto profile_edit = new StringEdit(ui->profile_edit, ATTRIBUTE_PROFILE_PATH, this);
    auto script_edit = new StringEdit(ui->script_edit, ATTRIBUTE_SCRIPT_PATH, this);
    auto home_edit = new StringEdit(ui->home_edit, ATTRIBUTE_HOME_DIRECTORY, this);

    edit_list->append({
        profile_edit,
        script_edit,
        home_edit,
    });

    check_map->insert(profile_edit, ui->profile_check);
    check_map->insert(script_edit, ui->script_check);
    check_map->insert(home_edit, ui->home_check);
}

ProfileMultiTab::~ProfileMultiTab() {
    delete ui;
}
