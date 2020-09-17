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
#include "dn_column_proxy.h"
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
    model->setHorizontalHeaderItem(PoliciesColumn_Name, new QStandardItem(tr("Name")));
    model->setHorizontalHeaderItem(PoliciesColumn_DN, new QStandardItem(tr("DN")));

    const auto dn_column_proxy = new DnColumnProxy(PoliciesColumn_DN, this);

    setup_model_chain(view, model, {dn_column_proxy});

    const auto label = new QLabel(tr("Policies"), this);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(label);
    layout->addWidget(view);

    connect(
        AdInterface::instance(), &AdInterface::logged_in,
        this, &PoliciesWidget::on_logged_in);

    object_context_menu_connect(view, PoliciesColumn_DN);
}

void PoliciesWidget::on_logged_in() {
    model->removeRows(0, model->rowCount());

    const QList<QString> gpos = AdInterface::instance()->list_all_gpos();

    for (auto gpo : gpos) {
        QList<QStandardItem *> row;
        for (int i = 0; i < PoliciesColumn_COUNT; i++) {
            row.append(new QStandardItem());
        }

        const QString name = AdInterface::instance()->attribute_get(gpo, ATTRIBUTE_DISPLAY_NAME);
        row[PoliciesColumn_Name]->setText(name);

        row[PoliciesColumn_DN]->setText(gpo);

        model->appendRow(row);
    }
}
