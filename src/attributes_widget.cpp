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
#include "members_model.h"

#include <QTreeView>
#include <QStandardItemModel>

AttributesWidget::AttributesWidget()
: QTabWidget()
{
    model = new AttributesModel(this);

    view = new QTreeView();
    view->setEditTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed);
    view->setSelectionMode(QAbstractItemView::NoSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setModel(model);

    members_view = new QTreeView();
    members_model = new MembersModel(this);
    members_view->setModel(members_model);
    members_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    change_model_target("");
};

void AttributesWidget::change_model_target(const QString &dn) {
    model->change_target(dn);

    members_model->change_target(dn);
    members_view->setColumnHidden(MembersModel::Column::DN, true);

    // Setup tabs
    clear();

    addTab(view, "All Attributes");

    bool is_group = attribute_value_exists(dn, "objectClass", "group");
    if (is_group) {
        addTab(members_view, "Group members");
    }
}
