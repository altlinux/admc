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

#include "policies_widget.h"
#include "ad_interface.h"
#include "details_widget.h"
#include "object_context_menu.h"
#include "utils.h"

#include <QTreeView>
#include <QLabel>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QMenu>

enum PoliciesColumn {
    PoliciesColumn_Name,
    PoliciesColumn_DN,
    PoliciesColumn_COUNT,
};

PoliciesWidget::PoliciesWidget()
: QWidget()
{   
    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setContextMenuPolicy(Qt::CustomContextMenu);

    model = new QStandardItemModel(0, PoliciesColumn_COUNT, this);
    set_horizontal_header_labels_from_map(model, {
        {PoliciesColumn_Name, tr("Name")},
        {PoliciesColumn_DN, tr("DN")}
    });

    view->setModel(model);

    setup_column_toggle_menu(view, model, {PoliciesColumn_Name});

    const auto label = new QLabel(tr("Policies"), this);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(label);
    layout->addWidget(view);

    ObjectContextMenu::connect_view(view, PoliciesColumn_DN);

    show_only_in_dev_mode(this);

    connect(
        AdInterface::instance(), &AdInterface::modified,
        this, &PoliciesWidget::reload);
    reload();
}

void PoliciesWidget::reload() {
    model->removeRows(0, model->rowCount());

    const QList<QString> search_attributes = {ATTRIBUTE_DISPLAY_NAME};
    const QString filter = filter_EQUALS(ATTRIBUTE_OBJECT_CLASS, CLASS_GP_CONTAINER);
    const QHash<QString, AdObject> search_results = AdInterface::instance()->search(filter, search_attributes, SearchScope_All);

    for (auto dn : search_results.keys()) {
        const AdObject attributes = search_results[dn];
        const QString display_name = attributes.get_value(ATTRIBUTE_DISPLAY_NAME);
        
        const QList<QStandardItem *> row = make_item_row(PoliciesColumn_COUNT);
        row[PoliciesColumn_Name]->setText(display_name);
        row[PoliciesColumn_DN]->setText(dn);

        model->appendRow(row);
    }
}
