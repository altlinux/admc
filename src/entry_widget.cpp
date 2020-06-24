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

#include "entry_widget.h"
#include "ad_interface.h"
#include "entry_model.h"

#include <QTreeView>
#include <QVBoxLayout>

EntryWidget::EntryWidget(EntryModel *model, QWidget *parent)
: QWidget(parent)
{    
    entry_model = model;
    
    view = new QTreeView();
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setDragDropMode(QAbstractItemView::DragDrop);

    setLayout(new QVBoxLayout());
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->setSpacing(0);
    layout()->addWidget(view);

    connect(
        view, &QAbstractItemView::clicked,
        this, &EntryWidget::on_view_clicked);
    connect(
        AD(), &AdInterface::ad_interface_login_complete,
        this, &EntryWidget::on_ad_interface_login_complete);

    setEnabled(false);
}

void EntryWidget::on_ad_interface_login_complete(const QString &base, const QString &head) {
    setEnabled(true);
}

void EntryWidget::on_view_clicked(const QModelIndex &index) {
    const QString dn = entry_model->get_dn_from_index(index);

    emit clicked_dn(dn);
}
