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
#include "main_window.h"
#include "ad_interface.h"
#include "entry_model.h"

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
#include <QAction>

QSet<EntryWidget *> EntryWidget::instances;

EntryWidget::EntryWidget(EntryModel *model)
: QWidget()
{    
    entry_model = model;
    
    instances.insert(this);

    view = new QTreeView();
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setDragDropMode(QAbstractItemView::DragDrop);

    setLayout(new QVBoxLayout());
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->setSpacing(0);
    layout()->addWidget(view);

    // Init column visibility
    for (int i = 0; i < entry_model->columnCount(); i++) {
        column_hidden.push_back(false);
    }
    update_column_visibility();

    connect(
        MainWindow::action_toggle_dn, &QAction::triggered,
        this, &EntryWidget::on_action_toggle_dn);

    connect(
        view, &QWidget::customContextMenuRequested,
        this, &EntryWidget::on_context_menu_requested);

    connect(
        view, &QAbstractItemView::clicked,
        this, &EntryWidget::on_view_clicked);
}

EntryWidget::~EntryWidget() {
    instances.remove(this);
}

void EntryWidget::on_context_menu_requested(const QPoint &pos) {
    // Open entry context menu
    QModelIndex index = view->indexAt(pos);

    if (!index.isValid()) {
        return;
    }
    
    QMenu menu;

    menu.addAction(MainWindow::action_details);
    menu.addAction(MainWindow::action_delete_entry);
    menu.addAction(MainWindow::action_rename);

    QMenu *submenu_new = menu.addMenu("New");
    submenu_new->addAction(MainWindow::action_new_user);
    submenu_new->addAction(MainWindow::action_new_computer);
    submenu_new->addAction(MainWindow::action_new_group);
    submenu_new->addAction(MainWindow::action_new_ou);

    const QString dn = entry_model->get_dn_from_index(index);
    const bool entry_is_policy = AD()->attribute_value_exists(dn, "objectClass", "groupPolicyContainer"); 
    if (entry_is_policy) {
        menu.addAction(MainWindow::action_edit_policy);
    }

    QPoint global_pos = view->mapToGlobal(pos);
    menu.exec(global_pos, MainWindow::action_details);
}

QString EntryWidget::get_selected_dn() {
    for (auto e : instances) {
        if (e->view->hasFocus()) {
            const auto selection_model = e->view->selectionModel();

            if (selection_model->hasSelection()) {
                const QList<QModelIndex> selected_indexes = selection_model->selectedIndexes();
                const QModelIndex selected = selected_indexes[0];
                const QString dn = e->entry_model->get_dn_from_index(selected);

                return dn;
            }
        }
    }
    
    return "";
}

void EntryWidget::on_action_toggle_dn(bool checked) {
    const bool dn_column_hidden = !checked;
    column_hidden[entry_model->dn_column] = dn_column_hidden;

    update_column_visibility();
}

void EntryWidget::update_column_visibility() {
    // Set column visiblity to current values in column_hidden
    for (int i = 0; i < column_hidden.size(); i++) {
        view->setColumnHidden(i, column_hidden[i]);
    }
}

void EntryWidget::on_view_clicked(const QModelIndex &index) {
    const QString dn = entry_model->get_dn_from_index(index);

    emit clicked_dn(dn);
}
