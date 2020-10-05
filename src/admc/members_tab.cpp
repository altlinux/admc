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

#include "members_tab.h"
#include "object_context_menu.h"
#include "utils.h"
#include "select_dialog.h"

#include <QTreeView>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QMenu>
#include <QPushButton>

// Store members in a set
// Generate model from current members list
// Add new members via select dialog
// Remove through context menu or select+remove button

enum MembersColumn {
    MembersColumn_Name,
    MembersColumn_DN,
    MembersColumn_COUNT,
};

MembersTab::MembersTab()
: DetailsTab()
{   
    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setAllColumnsShowFocus(true);
    view->setSortingEnabled(true);

    model = new QStandardItemModel(0, MembersColumn_COUNT, this);
    set_horizontal_header_labels_from_map(model, {
        {MembersColumn_Name, tr("Name")},
        {MembersColumn_DN, tr("DN")}
    });

    view->setModel(model);

    setup_column_toggle_menu(view, model, {MembersColumn_Name});

    auto add_button = new QPushButton(tr("Add"));
    auto remove_button = new QPushButton(tr("Remove"));
    auto button_layout = new QHBoxLayout();
    button_layout->addWidget(add_button);
    button_layout->addWidget(remove_button);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(view);
    layout->addLayout(button_layout);

    connect(
        remove_button, &QAbstractButton::clicked,
        this, &MembersTab::on_remove_button);
    connect(
        add_button, &QAbstractButton::clicked,
        this, &MembersTab::on_add_button);
    QObject::connect(
        view, &QWidget::customContextMenuRequested,
        this, &MembersTab::on_context_menu);
}

bool MembersTab::changed() const {
    return (current_members != original_members);
}

bool MembersTab::verify() {
    return true;
}

// TODO: could do this with less requests by deleting all values of ATTRIBUTE_MEMBER and then setting to the list of values, but need to implement this in adldap
void MembersTab::apply(const QString &target) {
    for (auto member : original_members) {
        const bool removed = !current_members.contains(member);
        if (removed) {
            AdInterface::instance()->attribute_delete_string(target, ATTRIBUTE_MEMBER, member);
        }
    }

    for (auto member : current_members) {
        const bool added = !original_members.contains(member);
        if (added) {
            AdInterface::instance()->attribute_add_string(target, ATTRIBUTE_MEMBER, member);
        }
    }
}

void MembersTab::load(const QString &target, const AdObject &attributes) {
    const QList<QString> members = attributes.get_strings(ATTRIBUTE_MEMBER);

    original_members = members.toSet();
    current_members = original_members;

    reload_current_members_into_model();
}

bool MembersTab::accepts_target(const AdObject &attributes) const {
    bool is_group = attributes.is_group();

    return is_group;
}

void MembersTab::on_context_menu(const QPoint pos) {
    const QString dn = get_dn_from_pos(pos, view, MembersColumn_DN);

    QMenu menu(this);
    menu.addAction(tr("Remove from group"),
        [this, dn]() {
            const QList<QString> removed_members = {dn};
            remove_members(removed_members);
        });

    exec_menu_from_view(&menu, view, pos);
}

void MembersTab::on_add_button() {
    const QList<QString> classes = {CLASS_USER};
    const QList<QString> selected_objects = SelectDialog::open(classes, SelectDialogMultiSelection_Yes);

    if (selected_objects.size() > 0) {
        add_members(selected_objects);
    }
}

void MembersTab::on_remove_button() {
    const QItemSelectionModel *selection_model = view->selectionModel();
    const QList<QModelIndex> selected = selection_model->selectedIndexes();

    QList<QString> removed_members;
    for (auto index : selected) {
        const QString dn = get_dn_from_index(index, MembersColumn_DN);

        removed_members.append(dn);
    }

    remove_members(removed_members);    
}

void MembersTab::reload_current_members_into_model() {
    model->removeRows(0, model->rowCount());

    for (auto dn : current_members) {
        const QString name = extract_name_from_dn(dn);
        
        const QList<QStandardItem *> row = make_item_row(MembersColumn_COUNT);
        row[MembersColumn_Name]->setText(name);
        row[MembersColumn_DN]->setText(dn);

        model->appendRow(row);
    }
}

void MembersTab::add_members(QList<QString> members) {
    for (auto member : members) {
        current_members.insert(member);
    }

    reload_current_members_into_model();

    emit edited();
}

void MembersTab::remove_members(QList<QString> members) {
    for (auto member : members) {
        current_members.remove(member);
    }

    reload_current_members_into_model();

    emit edited();
}
