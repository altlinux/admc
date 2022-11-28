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

#include "tabs/group_policy_tab.h"
#include "tabs/ui_group_policy_tab.h"

#include "adldap.h"
#include "attribute_edits/gpoptions_edit.h"
#include "globals.h"
#include "select_policy_dialog.h"
#include "settings.h"
#include "utils.h"

#include <QDebug>
#include <QFormLayout>
#include <QMenu>
#include <QPushButton>
#include <QStandardItemModel>
#include <QVBoxLayout>


GroupPolicyTab::GroupPolicyTab(QList<AttributeEdit *> *edit_list, QWidget *parent)
: QWidget(parent) {
    ui = new Ui::GroupPolicyTab();
    ui->setupUi(this);

    auto options_edit = new GpoptionsEdit(ui->gpo_options_check, this);

    edit_list->append({
        options_edit
    });
}

GroupPolicyTab::~GroupPolicyTab() {

    delete ui;
}

