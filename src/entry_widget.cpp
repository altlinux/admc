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
#include "ad_model.h"
#include "ad_proxy_model.h"
#include "actions.h"

#include <QApplication>
#include <QItemSelection>
#include <QSortFilterProxyModel>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QTreeView>
#include <QHeaderView>
#include <QLabel>
#include <QVBoxLayout>
#include <QMenu>

EntryWidget::EntryWidget(AdModel* model)
: QWidget()
{
    proxy = new AdProxyModel(model, this);
    
    view = new QTreeView();
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setModel(proxy);

    label = new QLabel("LABEL");

    setLayout(new QVBoxLayout());
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->setSpacing(0);
    layout()->addWidget(label);
    layout()->addWidget(view);

    // Init column visibility
    for (int column_i = AdModel::Column::Name; column_i < AdModel::Column::COUNT; column_i++) {
        auto column = static_cast<AdModel::Column>(column_i);

        column_hidden[column] = false;
    }
    update_column_visibility();

    QObject::connect(
        &action_toggle_dn, &QAction::triggered,
        this, &EntryWidget::on_action_toggle_dn);

    QObject::connect(
        view, &QWidget::customContextMenuRequested,
        this, &EntryWidget::on_context_menu_requested);
}

void EntryWidget::on_context_menu_requested(const QPoint &pos) {
    // Open entry context menu
    QModelIndex index = view->indexAt(pos);

    if (!index.isValid()) {
        return;
    }
    
    QMenu menu;

    menu.addAction(&action_attributes);
    menu.addAction(&action_delete_entry);

    QMenu *submenu_new = menu.addMenu("New");
    submenu_new->addAction(&action_new_user);
    submenu_new->addAction(&action_new_computer);
    submenu_new->addAction(&action_new_group);
    submenu_new->addAction(&action_new_ou);

    QPoint global_pos = view->mapToGlobal(pos);
    menu.exec(global_pos, &action_attributes);
}

QString EntryWidget::get_selected_dn() const {
    // Return dn of selected entry, if any is selected and view
    // has focus
    const auto selection_model = view->selectionModel();

    if (view->hasFocus() && selection_model->hasSelection()) {
        auto selected_indexes = selection_model->selectedIndexes();
        auto selected = selected_indexes[0];
        QModelIndex dn_index = selected.siblingAtColumn(AdModel::Column::DN);

        return dn_index.data().toString();
    } else {
        return "";
    }
}

void EntryWidget::on_action_toggle_dn(bool checked) {
    const bool dn_column_hidden = !checked;
    column_hidden[AdModel::Column::DN] = dn_column_hidden;

    update_column_visibility();
}

void EntryWidget::update_column_visibility() {
    // Set column visiblity to current values in column_hidden
    for (int column_i = AdModel::Column::Name; column_i < AdModel::Column::COUNT; column_i++) {
        auto column = static_cast<AdModel::Column>(column_i);

        view->setColumnHidden(column, column_hidden[column]);
    }
}
