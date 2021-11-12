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

#include "fsmo_tab.h"
#include "ui_fsmo_tab.h"

#include "adldap.h"
#include "utils.h"
#include "globals.h"
#include "status.h"

FSMOTab::FSMOTab(const QString &role_dn_arg) {
    ui = new Ui::FSMOTab();
    ui->setupUi(this);

    role_dn = role_dn_arg;
}

FSMOTab::~FSMOTab() {
    delete ui;
}

void FSMOTab::load(AdInterface &ad) {
    const QString current_master = [&]() {
        const AdObject role_object = ad.search_object(role_dn);
        const QString master_settings_dn = role_object.get_string(ATTRIBUTE_FSMO_ROLE_OWNER);
        const QString master_dn = dn_get_parent(master_settings_dn);
        const AdObject master_object = ad.search_object(master_dn);
        const QString out = master_object.get_string(ATTRIBUTE_DNS_HOST_NAME);

        return out;
    }();

    const QString new_master = [&]() {
        const AdObject rootDSE = ad.search_object("");
        const QString server_name = rootDSE.get_string(ATTRIBUTE_SERVER_NAME);
        const AdObject server = ad.search_object(server_name);
        const QString out = server.get_string(ATTRIBUTE_DNS_HOST_NAME);

        return out;
    }();

    ui->current_edit->setText(current_master);
    ui->new_edit->setText(new_master);
}
