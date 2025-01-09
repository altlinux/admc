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

#include "tabs/os_tab.h"
#include "tabs/ui_os_tab.h"

#include "adldap.h"
#include "attribute_edits/string_edit.h"

OSTab::OSTab(QList<AttributeEdit *> *edit_list, QWidget *parent)
: QWidget(parent) {
    ui = new Ui::OSTab();
    ui->setupUi(this);

    auto os_edit = new StringEdit(ui->os_edit, ATTRIBUTE_OS, this);
    auto version_edit = new StringEdit(ui->version_edit, ATTRIBUTE_OS_VERSION, this);
    auto pack_edit = new StringEdit(ui->pack_edit, ATTRIBUTE_OS_SERVICE_PACK, this);

    edit_list->append({
        os_edit,
        version_edit,
        pack_edit,
    });
}

OSTab::~OSTab() {
    delete ui;
}
