/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
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
#include "edits/string_edit.h"
#include "edits/manager_edit.h"
#include "ad_interface.h"
#include "details_dialog.h"
#include "utils.h"

#include <QGridLayout>
#include <QTreeView>
#include <QStandardItemModel>

enum ReportsColumn {
    ReportsColumn_Name,
    ReportsColumn_Folder,
    ReportsColumn_DN,
    ReportsColumn_COUNT,
};

OrganizationTab::OrganizationTab() {   
    auto edits_layout = new QGridLayout();

    const QList<QString> attributes = {
        ATTRIBUTE_TITLE,
        ATTRIBUTE_DEPARTMENT,
        ATTRIBUTE_COMPANY,
    };
    make_string_edits(attributes, CLASS_USER, this, &edits);

    new ManagerEdit(this, &edits);

    edits_add_to_layout(edits, edits_layout);
    edits_connect_to_tab(edits, this);

    reports_model = new QStandardItemModel(0, ReportsColumn_COUNT, this);
    set_horizontal_header_labels_from_map(reports_model, {
        {ReportsColumn_Name, tr("Name")},
        {ReportsColumn_Folder, tr("Folder")},
        {ReportsColumn_DN, tr("DN")}
    });

    auto reports_view = new QTreeView(this);
    reports_view->setModel(reports_model);
    reports_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    reports_view->setAllColumnsShowFocus(true);
    reports_view->setSortingEnabled(true);
    DetailsDialog::connect_to_open_by_double_click(reports_view, ReportsColumn_DN);

    setup_column_toggle_menu(reports_view, reports_model, {ReportsColumn_Name, ReportsColumn_Folder});

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addLayout(edits_layout);
    top_layout->addWidget(reports_view);
}

void OrganizationTab::load(const AdObject &object) {
    const QList<QString> reports = object.get_strings(ATTRIBUTE_DIRECT_REPORTS);
    
    reports_model->removeRows(0, reports_model->rowCount());
    for (auto dn : reports) {
        const QString name = dn_get_rdn(dn);
        const QString parent = dn_get_parent(dn);
        
        const QList<QStandardItem *> row = make_item_row(ReportsColumn_COUNT);
        row[ReportsColumn_Name]->setText(name);
        row[ReportsColumn_Folder]->setText(parent);
        row[ReportsColumn_DN]->setText(dn);

        reports_model->appendRow(row);
    }

    DetailsTab::load(object);
}
