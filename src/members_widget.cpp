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
#include "object_context_menu.h"
#include "utils.h"
#include "dn_column_proxy.h"

#include <QTreeView>
#include <QVBoxLayout>

enum MembersColumn {
    MembersColumn_Name,
    MembersColumn_DN,
    MembersColumn_COUNT,
};

MembersWidget::MembersWidget(ObjectContextMenu *object_context_menu, DetailsWidget *details_arg)
: DetailsTab(details_arg)
{   
    title = tr("Group members");

    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setAcceptDrops(true);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setDragDropMode(QAbstractItemView::DragDrop);
    view->setAllColumnsShowFocus(true);
    object_context_menu->connect_view(view, MembersColumn_DN);

    model = new MembersModel(this);
    const auto dn_column_proxy = new DnColumnProxy(MembersColumn_DN, this);

    setup_model_chain(view, model, {dn_column_proxy});
    
    const auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(view);
}

void MembersWidget::reload() {
    model->change_target(target());

    set_root_to_head(view);
}

bool MembersWidget::accepts_target() const {
    bool is_group = AdInterface::instance()->is_group(target());

    return is_group;
}

MembersModel::MembersModel(QObject *parent)
: ObjectModel(MembersColumn_COUNT, MembersColumn_DN, parent)
{
    setHorizontalHeaderItem(MembersColumn_Name, new QStandardItem(tr("Name")));
    setHorizontalHeaderItem(MembersColumn_DN, new QStandardItem(tr("DN")));
}

void MembersModel::change_target(const QString &dn) {
    removeRows(0, rowCount());

    auto create_row = [this](const QString &row_dn) {
        QList<QStandardItem *> row;
        for (int i = 0; i < MembersColumn_COUNT; i++) {
            row.append(new QStandardItem());
        }
        const QString name = AdInterface::instance()->attribute_get(row_dn, "name");
        row[MembersColumn_Name]->setText(name);
        row[MembersColumn_DN]->setText(row_dn);

        return row;
    };

    // Create root item to represent group itself
    QList<QStandardItem *> group_row = create_row(dn);
    appendRow(group_row);
    QStandardItem *group_item = group_row[0];

    // Populate model with members of new root
    const QList<QString> members = AdInterface::instance()->attribute_get_multi(dn, "member");
    for (auto member_dn : members) {
        QList<QStandardItem *> member_row = create_row(member_dn);
        group_item->appendRow(member_row);
    }
}
