/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2022 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
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

#include "adldap.h"
#include "fsmo_tab.h"
#include "globals.h"
#include "settings.h"

enum FSMORole {
    FSMORole_DomainDNS,
    FSMORole_ForestDNS,
    FSMORole_PDCEmulation,
    FSMORole_Schema,
    FSMORole_DomainNaming,
    FSMORole_Infrastructure,
    FSMORole_RidAllocation,

    FSMORole_COUNT,
};

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

        // NOTE: this is the DN of the object that
        // store's role's master in it's attributes
        const QString role_dn = [&]() {
            const QString domain_dn = g_adconfig->domain_dn();

            switch (role) {
                case FSMORole_DomainDNS: return QString("CN=Infrastructure,DC=DomainDnsZones,%1").arg(domain_dn);
                case FSMORole_ForestDNS: return QString("CN=Infrastructure,DC=ForestDnsZones,%1").arg(domain_dn);
                case FSMORole_PDCEmulation: return domain_dn;
                case FSMORole_Schema: return g_adconfig->schema_dn();
                case FSMORole_DomainNaming: return g_adconfig->partitions_dn();
                case FSMORole_Infrastructure: return QString("CN=Infrastructure,%1").arg(domain_dn);
                case FSMORole_RidAllocation: return QString("CN=RID Manager$,CN=System,%1").arg(domain_dn);

                case FSMORole_COUNT: break;
            };

            return QString();
        }();

        auto tab = new FSMOTab(title, role_dn);
        ui->tab_widget->add_tab(tab, title);
        tab->load(ad);
    }

    settings_setup_dialog_geometry(SETTING_fsmo_dialog_geometry, this);
}

FSMODialog::~FSMODialog() {
    delete ui;
}
