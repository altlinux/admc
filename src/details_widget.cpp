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

#include "details_widget.h"
#include "attributes_model.h"
#include "ad_interface.h"
#include "members_widget.h"
#include "settings.h"

#include <QTreeView>
#include <QStandardItemModel>
#include <QAction>

DetailsWidget::DetailsWidget(MembersWidget *members_widget_, QWidget *parent)
: QTabWidget(parent)
{
    members_widget = members_widget_;

    attributes_model = new AttributesModel(this);

    attributes_view = new QTreeView();
    attributes_view->setEditTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed);
    attributes_view->setSelectionMode(QAbstractItemView::NoSelection);
    attributes_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    attributes_view->setModel(attributes_model);

    connect(
        AD(), &AdInterface::ad_interface_login_complete,
        this, &DetailsWidget::on_ad_interface_login_complete);
    connect(
        AD(), &AdInterface::delete_entry_complete,
        this, &DetailsWidget::on_delete_entry_complete);
    connect(
        AD(), &AdInterface::dn_changed,
        this, &DetailsWidget::on_dn_changed);
    connect(
        AD(), &AdInterface::attributes_changed,
        this, &DetailsWidget::on_attributes_changed);

    change_target("");
};

void DetailsWidget::change_target(const QString &dn) {
    // Save current tab/index to restore later
    QWidget *old_tab = widget(currentIndex());

    target_dn = dn;

    attributes_model->change_target(target_dn);

    members_widget->change_target(target_dn);

    // Setup tabs
    clear();

    addTab(attributes_view, "All Attributes");

    bool is_group = AD()->attribute_value_exists(target_dn, "objectClass", "group");
    if (is_group) {
        addTab(members_widget, "Group members");
    }

    // Restore current index if it is still shown
    // Otherwise current index is set to first tab by default
    const int old_tab_index_in_new_tabs = indexOf(old_tab);
    if (old_tab_index_in_new_tabs != -1) {
        setCurrentIndex(old_tab_index_in_new_tabs);
    }
}

void DetailsWidget::on_ad_interface_login_complete(const QString &search_base, const QString &head_dn) {
    // Clear data on new login
    change_target("");
}

void DetailsWidget::on_delete_entry_complete(const QString &dn) {
    // Clear data if current target was deleted
    if (target_dn == dn) {
        change_target("");
    }
}

void DetailsWidget::on_dn_changed(const QString &old_dn, const QString &new_dn) {
    // Switch to the entry at new dn (entry stays the same)
    if (target_dn == old_dn) {
        change_target(new_dn);
    }
}

void DetailsWidget::on_attributes_changed(const QString &dn) {
    // Reload entry since attributes were updated
    if (target_dn == dn) {
        change_target(dn);
    }
}

void DetailsWidget::on_containers_clicked_dn(const QString &dn) {
    if (SETTINGS()->details_on_containers_click->isChecked()) {
        change_target(dn);
    }
}

void DetailsWidget::on_contents_clicked_dn(const QString &dn) {
    if (SETTINGS()->details_on_contents_click->isChecked()) {
        change_target(dn);
    }
}

void DetailsWidget::on_context_menu_details(const QString &dn) {
    change_target(dn);
}
