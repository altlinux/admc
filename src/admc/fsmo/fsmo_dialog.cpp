/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2026 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
 * Copyright (C) 2026 Artyom V. Poptsov
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

#include "adldap.h"
#include "core/fsmo.h"
#include "fsmo/fsmo_dialog.h"
#include "fsmo/fsmo_tab.h"
#include "fsmo/fsmo_utils.h"
#include "ui_fsmo_dialog.h"

FSMODialog::FSMODialog(AdInterface &ad, QWidget *parent) : QDialog(parent) {
    ui = new Ui::FSMODialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    fsmo_role_for_each(
        [&](FSMORole role, const QString& title, const QString &dn) {
            auto tab = new FSMOTab(title, dn);
            ui->tab_widget->add_tab(tab, title);
            tab->load(ad);
            connect(tab, &FSMOTab::master_changed,
                    this, &FSMODialog::master_changed);
        });

    ui->warning_widget->setVisible(false);
    ui->gpo_edit_PDC_check->setChecked(gpo_edit_without_PDC_disabled);
    connect(ui->gpo_edit_PDC_check, &QCheckBox::toggled,
            this, &FSMODialog::gpo_edit_PDC_check_toggled);

    fsmo_setup_dialog_geometry(this);
}

FSMODialog::~FSMODialog() {
    delete ui;
}

void FSMODialog::gpo_edit_PDC_check_toggled(bool is_checked)
{
    gpo_edit_without_PDC_disabled = is_checked;
    if (!is_checked)
        ui->warning_widget->setVisible(true);
    else
        ui->warning_widget->setVisible(false);
}
