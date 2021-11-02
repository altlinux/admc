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

#include "tabs/delegation_tab.h"
#include "tabs/ui_delegation_tab.h"

#include "adldap.h"
#include "edits/delegation_edit.h"
#include "globals.h"

DelegationTab::DelegationTab() {
    ui = new Ui::DelegationTab();
    ui->setupUi(this);

    new DelegationEdit(ui->off_button, ui->on_button, &edits, this);

    edits_connect_to_tab(edits, this);
}

DelegationTab::~DelegationTab() {
    delete ui;
}
