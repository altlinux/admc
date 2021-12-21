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

#include "tabs/os_tab.h"
#include "tabs/ui_os_tab.h"

#include "adldap.h"
#include "attribute_edits/string_edit.h"

OSTab::OSTab(QList<AttributeEdit *> *edit_list, QWidget *parent)
: QWidget(parent) {
    ui = new Ui::OSTab();
    ui->setupUi(this);

    QList<AttributeEdit *> my_edit_list;

    new StringEdit(ui->os_edit, ATTRIBUTE_OS, &my_edit_list, this);
    new StringEdit(ui->version_edit, ATTRIBUTE_OS_VERSION, &my_edit_list, this);
    new StringEdit(ui->pack_edit, ATTRIBUTE_OS_SERVICE_PACK, &my_edit_list, this);

    AttributeEdit::set_read_only(my_edit_list, true);

    edit_list->append(my_edit_list);
}

OSTab::~OSTab() {
    delete ui;
}
