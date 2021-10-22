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

#include "tabs/profile_tab.h"
#include "tabs/ui_profile_tab.h"

#include "adldap.h"
#include "edits/string_edit.h"

ProfileTab::ProfileTab() {
    ui = new Ui::ProfileTab();
    ui->setupUi(this);

    new StringEdit(ui->profile_path_edit, ATTRIBUTE_PROFILE_PATH, &edits, this);
    new StringEdit(ui->script_path_edit, ATTRIBUTE_SCRIPT_PATH, &edits, this);

    // TODO: verify that local path exists. Also add alternate input method named "Connect"? Has drop-down of disk letters and path input.
    new StringEdit(ui->home_dir_edit, ATTRIBUTE_HOME_DIRECTORY, &edits, this);

    edits_connect_to_tab(edits, this);
}

ProfileTab::~ProfileTab() {
    delete ui;
}
