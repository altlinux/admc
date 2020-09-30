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

    connect(
        AdInterface::instance(), &AdInterface::logged_in,
        this, &PoliciesWidget::on_ad_modified);
    connect(
        AdInterface::instance(), &AdInterface::modified,
        this, &PoliciesWidget::on_ad_modified);

    ObjectContextMenu::connect_view(view, PoliciesColumn_DN);

    show_only_in_dev_mode(this);
}

void PoliciesWidget::on_ad_modified() {
    model->removeRows(0, model->rowCount());

    const QList<QString> gpos = AdInterface::instance()->list_all_gpos();

    for (auto gpo : gpos) {
        const QString name = AdInterface::instance()->get_name_for_display(gpo);
        
        const QList<QStandardItem *> row = make_item_row(PoliciesColumn_COUNT);
        row[PoliciesColumn_Name]->setText(name);
        row[PoliciesColumn_DN]->setText(gpo);

        model->appendRow(row);
    }
}
