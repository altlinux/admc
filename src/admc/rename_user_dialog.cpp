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

#include "rename_user_dialog.h"
#include "ui_rename_user_dialog.h"

#include "adldap.h"
#include "utils.h"
#include "edits/string_edit.h"
#include "edits/sama_edit.h"
#include "edits/upn_edit.h"

RenameUserDialog::RenameUserDialog(QWidget *parent)
: RenameDialog(parent) {
    ui = new Ui::RenameUserDialog();
    ui->setupUi(this);

    QList<AttributeEdit *> edit_list;
    new StringEdit(ui->first_name_edit, ATTRIBUTE_FIRST_NAME, CLASS_USER, &edit_list, this);
    new StringEdit(ui->last_name_edit, ATTRIBUTE_LAST_NAME, CLASS_USER, &edit_list, this);
    new StringEdit(ui->full_name_edit, ATTRIBUTE_DISPLAY_NAME, CLASS_USER, &edit_list, this);
    upn_edit = new UpnEdit(ui->upn_prefix_edit, ui->upn_suffix_edit, &edit_list, this);
    new SamaEdit(ui->sama_edit, ui->sama_domain_edit, &edit_list, this);

    init(ui->name_edit, ui->button_box, edit_list);
}

void RenameUserDialog::open() {
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    upn_edit->init_suffixes(ad);

    RenameDialog::open();
}
