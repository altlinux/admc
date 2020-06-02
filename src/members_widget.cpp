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

#include "members_widget.h"
#include "members_model.h"

#include <QTreeView>
#include <QLabel>

MembersWidget::MembersWidget()
: EntryWidget(MembersModel::Column::COUNT, MembersModel::Column::DN)
{   
    model = new MembersModel(this);

    view->setModel(model);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setDragDropMode(QAbstractItemView::DragDrop);
    view->setAcceptDrops(true);
    view->setModel(model);

    column_hidden[MembersModel::Column::Name] = false;
    column_hidden[MembersModel::Column::DN] = true;
    update_column_visibility();
}

void MembersWidget::change_target(const QString &dn) {
    model->change_target(dn);
    update_column_visibility();
}
