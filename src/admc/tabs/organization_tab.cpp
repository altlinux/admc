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

#include "tabs/organization_tab.h"
#include "tabs/ui_organization_tab.h"

#include "adldap.h"
#include "edits/manager_edit.h"
#include "edits/string_edit.h"
#include "properties_dialog.h"
#include "utils.h"
#include "settings.h"
#include "globals.h"

#include <QStandardItemModel>

enum ReportsColumn {
    ReportsColumn_Name,
    ReportsColumn_Folder,
    ReportsColumn_COUNT,
};

enum ReportsRole {
    ReportsRole_DN = Qt::UserRole + 1,
};

OrganizationTab::OrganizationTab() {
    ui = new Ui::OrganizationTab();
    ui->setupUi(this);

    new StringEdit(ui->job_title_edit, ATTRIBUTE_TITLE, &edits, this);
    new StringEdit(ui->department_edit, ATTRIBUTE_DEPARTMENT, &edits, this);
    new StringEdit(ui->company_edit, ATTRIBUTE_COMPANY, &edits, this);

    new ManagerEdit(ui->manager_widget, ATTRIBUTE_MANAGER, &edits, this);

    edits_connect_to_tab(edits, this);

    reports_model = new QStandardItemModel(0, ReportsColumn_COUNT, this);
    set_horizontal_header_labels_from_map(reports_model,
        {
            {ReportsColumn_Name, tr("Name")},
            {ReportsColumn_Folder, tr("Folder")},
        });

    ui->reports_view->setModel(reports_model);

    PropertiesDialog::open_when_view_item_activated(ui->reports_view, ReportsRole_DN);

    settings_restore_header_state(SETTING_organization_tab_header_state, ui->reports_view->header());
}

OrganizationTab::~OrganizationTab() {
    settings_save_header_state(SETTING_organization_tab_header_state, ui->reports_view->header());   
}

void OrganizationTab::load(AdInterface &ad, const AdObject &object) {
    const QList<QString> reports = object.get_strings(ATTRIBUTE_DIRECT_REPORTS);

    reports_model->removeRows(0, reports_model->rowCount());
    for (auto dn : reports) {
        const QString name = dn_get_name(dn);
        const QString parent = dn_get_parent_canonical(dn);

        const QList<QStandardItem *> row = make_item_row(ReportsColumn_COUNT);
        row[ReportsColumn_Name]->setText(name);
        row[ReportsColumn_Folder]->setText(parent);

        set_data_for_row(row, dn, ReportsRole_DN);

        reports_model->appendRow(row);
    }

    PropertiesTab::load(ad, object);
}
