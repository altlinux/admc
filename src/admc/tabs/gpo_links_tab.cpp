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

#include "tabs/gpo_links_tab.h"
#include "details_dialog.h"
#include "utils.h"
#include "ad_interface.h"
#include "filter.h"

#include <algorithm>

#include <QTreeView>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QMenu>

enum GpoLinksColumn {
    GpoLinksColumn_Name,
    GpoLinksColumn_DN,
    GpoLinksColumn_COUNT
};

GpoLinksTab::GpoLinksTab() {   
    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setAllColumnsShowFocus(true);
    view->setSortingEnabled(true);

    model = new QStandardItemModel(0, GpoLinksColumn_COUNT, this);
    set_horizontal_header_labels_from_map(model, {
        {GpoLinksColumn_Name, tr("Name")},
        {GpoLinksColumn_DN, tr("DN")}
    });

    view->setModel(model);

    setup_column_toggle_menu(view, model, {GpoLinksColumn_Name});

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(view);

    DetailsDialog::connect_to_open_by_double_click(view, GpoLinksColumn_DN);
}

void GpoLinksTab::load(const AdObject &policy) {
    const QList<QString> search_attributes = {ATTRIBUTE_NAME};
    const QString filter = filter_CONDITION(Condition_Contains, ATTRIBUTE_GPLINK, policy.get_dn());
    const QHash<QString, AdObject> search_results = AD()->search(filter, search_attributes, SearchScope_All);

    // Sort objects by dn(identical to sorting by name)
    QList<QString> dns = search_results.keys();
    std::sort(dns.begin(), dns.end());

    for (auto dn : dns) {
        const AdObject object = search_results[dn];
        const QString name = object.get_string(ATTRIBUTE_NAME);

        const QList<QStandardItem *> row = make_item_row(GpoLinksColumn_COUNT);
        row[GpoLinksColumn_Name]->setText(name);
        row[GpoLinksColumn_DN]->setText(dn);

        model->appendRow(row);
    }

    model->sort(GpoLinksColumn_Name);
}

void GpoLinksTab::showEvent(QShowEvent *event) {
    resize_columns(view,
    {
        {GpoLinksColumn_Name, 0.5},
    });
}
