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

#include "tabs/delegation_tab.h"
#include "tabs/ui_delegation_tab.h"

#include "adldap.h"
#include "attribute_edits/delegation_edit.h"

DelegationTab::DelegationTab(QList<AttributeEdit *> *edit_list, QWidget *parent)
: QWidget(parent) {
    ui = new Ui::DelegationTab();
    ui->setupUi(this);

    auto tab_edit = new DelegationEdit(ui->off_button, ui->on_button, this);

    edit_list->append({
        tab_edit,
    });
}

DelegationTab::~DelegationTab() {
    delete ui;
}
