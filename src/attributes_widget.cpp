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
    attributes_model = new AttributesModel(this);

    attributes_view = new QTreeView();
    attributes_view->setEditTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed);
    attributes_view->setSelectionMode(QAbstractItemView::NoSelection);
    attributes_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    attributes_view->setModel(attributes_model);

    members_view = new QTreeView();
    members_model = new MembersModel(this);
    members_view->setModel(members_model);
    members_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    members_view->setAcceptDrops(true);
    members_view->setDragDropMode(QAbstractItemView::DragDrop);

    connect(
        &ad_interface, &AdInterface::delete_entry_complete,
        this, &AttributesWidget::on_delete_entry_complete);
    connect(
        &ad_interface, &AdInterface::move_user_complete,
        this, &AttributesWidget::on_move_user_complete);
    connect(
        &ad_interface, &AdInterface::load_attributes_complete,
        this, &AttributesWidget::on_load_attributes_complete);

    change_target("");
};

void AttributesWidget::change_target(const QString &dn) {
    // Save current tab/index to restore later
    QWidget *old_tab = widget(currentIndex());

    target_dn = dn;

    attributes_model->change_target(target_dn);

    members_model->change_target(target_dn);
    members_view->setColumnHidden(MembersModel::Column::DN, true);

    // Setup tabs
    clear();

    addTab(attributes_view, "All Attributes");

    bool is_group = attribute_value_exists(target_dn, "objectClass", "group");
    if (is_group) {
        addTab(members_view, "Group members");
    }

    // Restore current index if it is still shown
    // Otherwise current index is set to first tab by default
    const int old_tab_index_in_new_tabs = indexOf(old_tab);
    if (old_tab_index_in_new_tabs != -1) {
        setCurrentIndex(old_tab_index_in_new_tabs);
    }
}

void AttributesWidget::on_delete_entry_complete(const QString &dn) {
    // Clear data if current target was deleted
    if (target_dn == dn) {
        change_target(QString(""));
    }
}

void AttributesWidget::on_move_user_complete(const QString &user_dn, const QString &container_dn, const QString &new_dn) {
    // Switch to the entry at new dn (entry stays the same)
    if (target_dn == user_dn) {
        change_target(new_dn);
    }
}

void AttributesWidget::on_load_attributes_complete(const QString &dn) {
    // Reload entry since attributes were updated
    if (target_dn == dn) {
        change_target(dn);
    }
}
