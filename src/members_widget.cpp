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
#include "entry_context_menu.h"

#include <QTreeView>
#include <QLabel>
#include <QVBoxLayout>

MembersWidget::MembersWidget(EntryContextMenu *entry_context_menu, QWidget *parent)
: QWidget(parent)
{   
    model = new MembersModel(this);

    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setAcceptDrops(true);
    view->setModel(model);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setDragDropMode(QAbstractItemView::DragDrop);
    entry_context_menu->connect_view(view, MembersModel::Column::DN);

    const auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(view);
}

void MembersWidget::change_target(const QString &dn) {
    QModelIndex root_index = model->change_target(dn);

    // Set root to group index so that it is hidden
    view->setRootIndex(root_index);
}
