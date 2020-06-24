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
#include "entry_model.h"
#include "entry_context_menu.h"

#include <QTreeView>
#include <QLabel>
#include <QVBoxLayout>

MembersWidget::MembersWidget(EntryContextMenu *entry_context_menu, QWidget *parent)
: QWidget(parent)
{   
    model = new EntryModel(Column::COUNT, Column::DN, this);

    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setAcceptDrops(true);
    view->setModel(model);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setDragDropMode(QAbstractItemView::DragDrop);
    entry_context_menu->connect_view(view, Column::DN);

    const auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(view);
}

void MembersWidget::change_target(const QString &dn) {
    model->removeRows(0, model->rowCount());

    // Create root item to represent group itself
    const QString grp_name = AD()->get_attribute(dn, "name");
    const auto grp_name_item = new QStandardItem(grp_name);
    const auto grp_dn_item = new QStandardItem(dn);
    model->appendRow({grp_name_item, grp_dn_item});

    // Populate model with members of new root
    const QList<QString> members = AD()->get_attribute_multi(dn, "member");
    for (auto member_dn : members) {
        const QString name = AD()->get_attribute(member_dn, "name");

        const auto name_item = new QStandardItem(name);
        const auto dn_item = new QStandardItem(member_dn);

        grp_name_item->appendRow({name_item, dn_item});
    }

    QModelIndex grp_index = grp_name_item->index();

    // Set root to group index so that it is hidden
    view->setRootIndex(grp_index);
}
