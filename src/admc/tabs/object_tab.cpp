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

#include "tabs/object_tab.h"
#include "tabs/ui_object_tab.h"

#include "adldap.h"
#include "edits/datetime_edit.h"
#include "edits/string_edit.h"
#include "edits/protect_deletion_edit.h"

#include <QFormLayout>

ObjectTab::ObjectTab() {
    ui = new Ui::ObjectTab();
    ui->setupUi(this);

    new StringEdit(ui->dn_edit, ATTRIBUTE_DN, &edits, this);
    new StringEdit(ui->class_edit, ATTRIBUTE_OBJECT_CLASS, &edits, this);

    new DateTimeEdit(ui->created_edit, ATTRIBUTE_WHEN_CREATED, &edits, this);
    new DateTimeEdit(ui->changed_edit, ATTRIBUTE_WHEN_CHANGED, &edits, this);

    new StringEdit(ui->usn_created_edit, ATTRIBUTE_USN_CREATED, &edits, this);
    new StringEdit(ui->usn_changed_edit, ATTRIBUTE_USN_CHANGED, &edits, this);

    auto deletion_edit = new ProtectDeletionEdit(ui->deletion_check, &edits, this);

    edits_set_read_only(edits, true);
    deletion_edit->set_read_only(false);

    edits_connect_to_tab(edits, this);
}
