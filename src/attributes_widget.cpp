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

#include "attributes_widget.h"
#include "attributes_model.h"
#include "ad_interface.h"

#include <QWidget>
#include <QTreeView>
#include <QLabel>
#include <QVBoxLayout>

AttributesWidget::AttributesWidget()
: QWidget()
{
    model = new AttributesModel(this);

    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed);
    view->setSelectionMode(QAbstractItemView::NoSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setModel(model);

    const auto label = new QLabel("Attributes");

    setLayout(new QVBoxLayout());
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->setSpacing(0);
    layout()->addWidget(label);
    layout()->addWidget(view);
};

void AttributesWidget::change_model_target(const QString &new_target_dn) {
    model->change_target(new_target_dn);
}
