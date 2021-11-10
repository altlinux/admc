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

#include "fsmo_dialog.h"
#include "ui_fsmo_dialog.h"

#include "fsmo_tab.h"
#include "settings.h"

FSMODialog::FSMODialog(QWidget *parent)
: QDialog(parent) {
    ui = new Ui::FSMODialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    ui->tab_widget->add_tab(new FSMOTab("test"), "test");

    settings_setup_dialog_geometry(SETTING_fsmo_dialog_geometry, this);
}

FSMODialog::~FSMODialog() {
    delete ui;
}