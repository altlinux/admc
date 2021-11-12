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
#include "adldap.h"
#include "globals.h"

FSMODialog::FSMODialog(AdInterface &ad, QWidget *parent)
: QDialog(parent) {
    ui = new Ui::FSMODialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    const QString domain_dn = g_adconfig->domain_dn();
    const QString schema_dn = g_adconfig->schema_dn();
    const QString naming_dn = g_adconfig->partitions_dn();
    const QString infrastructure_dn = QString("CN=Infrastructure,%1").arg(domain_dn);
    const QString rid_dn = QString("CN=RID Manager$,CN=System,%1").arg(domain_dn);

    const AdObject rootDSE = ad.search_object("");
    const QString current_service_name = rootDSE.get_string(ATTRIBUTE_DS_SERVICE_NAME);

    const QString server_name = rootDSE.get_string(ATTRIBUTE_SERVER_NAME);
    const AdObject server = ad.search_object(server_name);
    const QString current_host = server.get_string(ATTRIBUTE_DNS_HOST_NAME);
    
    auto add_tab = [&](const QString &dn, const QString &name, const QString &explanation) {
        const AdObject object = ad.search_object(dn);
        const QString master_settings_dn = object.get_string(ATTRIBUTE_FSMO_ROLE_OWNER);
        const QString master_dn = dn_get_parent(master_settings_dn);

        const AdObject master_object = ad.search_object(master_dn);
        const QString host_name = master_object.get_string(ATTRIBUTE_DNS_HOST_NAME);

        ui->tab_widget->add_tab(new FSMOTab(explanation, host_name, current_host), name);
    };

    add_tab(domain_dn, tr("Pdc Emulation"), tr("domain"));
    add_tab(schema_dn, tr("Schema"), tr("schema"));
    add_tab(naming_dn, tr("Domain Naming"), tr("partitions"));
    add_tab(infrastructure_dn, tr("Infrastructure"), tr("infrastructure"));
    add_tab(rid_dn, tr("Rid Allocation"), tr("rid"));

    settings_setup_dialog_geometry(SETTING_fsmo_dialog_geometry, this);
}

FSMODialog::~FSMODialog() {
    delete ui;
}
