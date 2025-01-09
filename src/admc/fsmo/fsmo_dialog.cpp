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

#include "fsmo/fsmo_dialog.h"
#include "ui_fsmo_dialog.h"

#include "adldap.h"
#include "fsmo/fsmo_tab.h"
#include "fsmo/fsmo_utils.h"
#include "globals.h"
#include "settings.h"

#include <QMessageBox>


FSMODialog::FSMODialog(AdInterface &ad, QWidget *parent)
: QDialog(parent) {
    ui = new Ui::FSMODialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    for (int role_i = 0; role_i < FSMORole_COUNT; role_i++) {
        const FSMORole role = (FSMORole) role_i;

        const QString title = [&]() {
            switch (role) {
                case FSMORole_DomainDNS: return tr("Domain DNS");
                case FSMORole_ForestDNS: return tr("Forest DNS");
                case FSMORole_PDCEmulation: return tr("PDC Emulation");
                case FSMORole_Schema: return tr("Schema");
                case FSMORole_DomainNaming: return tr("Domain Naming");
                case FSMORole_Infrastructure: return tr("Infrastructure");
                case FSMORole_RidAllocation: return tr("Rid Allocation");

                case FSMORole_COUNT: break;
            };

            return QString();
        }();

        const QString role_dn = dn_from_role(role);

        auto tab = new FSMOTab(title, role_dn);
        ui->tab_widget->add_tab(tab, title);
        tab->load(ad);
        connect(tab, &FSMOTab::master_changed, this, &FSMODialog::master_changed);
    }

    ui->warning_widget->setVisible(false);
    ui->gpo_edit_PDC_check->setChecked(gpo_edit_without_PDC_disabled);
    connect(ui->gpo_edit_PDC_check, &QCheckBox::toggled, this, &FSMODialog::gpo_edit_PDC_check_toggled);

    settings_setup_dialog_geometry(SETTING_fsmo_dialog_geometry, this);
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
