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

#include "tabs/object_tab.h"
#include "tabs/ui_object_tab.h"

#include "adldap.h"
#include "attribute_edits/datetime_edit.h"
#include "attribute_edits/dn_edit.h"
#include "attribute_edits/gpoptions_edit.h"
#include "attribute_edits/protect_deletion_edit.h"
#include "attribute_edits/string_edit.h"

#include <QFormLayout>

ObjectTab::ObjectTab(QList<AttributeEdit *> *edit_list, QWidget *parent)
: QWidget(parent) {
    ui = new Ui::ObjectTab();
    ui->setupUi(this);

    auto dn_edit = new DNEdit(ui->dn_edit, this);
    auto class_edit = new StringEdit(ui->class_edit, ATTRIBUTE_OBJECT_CLASS, this);

    auto when_created_edit = new DateTimeEdit(ui->created_edit, ATTRIBUTE_WHEN_CREATED, this);
    auto when_changed_edit = new DateTimeEdit(ui->changed_edit, ATTRIBUTE_WHEN_CHANGED, this);

    auto usn_created_edit = new StringEdit(ui->usn_created_edit, ATTRIBUTE_USN_CREATED, this);
    auto usn_changed_edit = new StringEdit(ui->usn_changed_edit, ATTRIBUTE_USN_CHANGED, this);

    auto deletion_edit = new ProtectDeletionEdit(ui->deletion_check, this);

    edit_list->append({
        dn_edit,
        class_edit,
        when_created_edit,
        when_changed_edit,
        usn_created_edit,
        usn_changed_edit,
        deletion_edit,
    });
}

ObjectTab::~ObjectTab() {
    delete ui;
}
