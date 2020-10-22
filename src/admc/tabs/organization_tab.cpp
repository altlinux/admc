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
#include "filter.h"
#include "utils.h"

#include <QGridLayout>
#include <QTextEdit>
#include <QDebug>

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

    reports_text_edit = new QTextEdit();
    reports_text_edit->setReadOnly(true);

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addLayout(edits_layout);
    top_layout->addWidget(reports_text_edit);
}

void OrganizationTab::load(const AdObject &object) {
    const QList<QString> reports = object.get_strings(ATTRIBUTE_DIRECT_REPORTS);
    
    for (auto dn : reports) {
        const QString rdn = dn_get_rdn(dn);
        reports_text_edit->append(rdn);
    }

    DetailsTab::load(object);
}
