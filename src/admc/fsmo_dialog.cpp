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

    auto add_tab = [&](const QString &title, const QString &dn) {
        auto tab = new FSMOTab(dn);
        ui->tab_widget->add_tab(tab, title);
        tab->load(ad);
    };

    const QString domain_dn = g_adconfig->domain_dn();
    const QString schema_dn = g_adconfig->schema_dn();
    const QString naming_dn = g_adconfig->partitions_dn();
    const QString infrastructure_dn = QString("CN=Infrastructure,%1").arg(domain_dn);
    const QString rid_dn = QString("CN=RID Manager$,CN=System,%1").arg(domain_dn);

    add_tab(tr("Pdc Emulation"), domain_dn);
    add_tab(tr("Schema"), schema_dn);
    add_tab(tr("Domain Naming"), naming_dn);
    add_tab(tr("Infrastructure"), infrastructure_dn);
    add_tab(tr("Rid Allocation"), rid_dn);

    settings_setup_dialog_geometry(SETTING_fsmo_dialog_geometry, this);
}

FSMODialog::~FSMODialog() {
    delete ui;
}
