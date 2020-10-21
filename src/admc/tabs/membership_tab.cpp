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

#include "tabs/membership_tab.h"
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
    MembersColumn_Parent,
    MembersColumn_DN,
    MembersColumn_COUNT,
};

MembersTab::MembersTab()
: MembershipTab(MembershipTabType_Members) {

}

MemberOfTab::MemberOfTab()
: MembershipTab(MembershipTabType_MemberOf) {

}

MembershipTab::MembershipTab(const MembershipTabType type_arg) {
    type = type_arg;

    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setAllColumnsShowFocus(true);
    view->setSortingEnabled(true);

    model = new QStandardItemModel(0, MembersColumn_COUNT, this);
    set_horizontal_header_labels_from_map(model, {
        {MembersColumn_Name, tr("Name")},
        {MembersColumn_Parent, tr("Folder")},
        {MembersColumn_DN, tr("DN")}
    });

    view->setModel(model);

    setup_column_toggle_menu(view, model, {MembersColumn_Name, MembersColumn_Parent});

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
        this, &MembershipTab::on_remove_button);
    connect(
        add_button, &QAbstractButton::clicked,
        this, &MembershipTab::on_add_button);
    QObject::connect(
        view, &QWidget::customContextMenuRequested,
        this, &MembershipTab::on_context_menu);
}

void MembershipTab::load(const AdObject &object) {
    const QList<QString> values = object.get_strings(get_membership_attribute());
    original_values = values.toSet();
    current_values = original_values;

    reload_model(original_values);
}

bool MembershipTab::changed() const {
    return (current_values != original_values);
}

void MembershipTab::apply(const QString &target) const {
    // NOTE: logic is kinda duplicated but switching on behavior within iterations would be very confusing
    switch (type) {
        case MembershipTabType_Members: {
            const QString group = target;

            for (auto user : original_values) {
                const bool removed = !current_values.contains(user);
                if (removed) {
                    AD()->group_remove_user(group, user);
                }
            }

            for (auto user : current_values) {
                const bool added = !original_values.contains(user);
                if (added) {
                    AD()->group_add_user(group, user);
                }
            }

            break;
        }
        case MembershipTabType_MemberOf: {
            const QString user = target;

            for (auto group : original_values) {
                const bool removed = !current_values.contains(group);
                if (removed) {
                    AD()->group_remove_user(group, user);
                }
            }

            for (auto group : current_values) {
                const bool added = !original_values.contains(group);
                if (added) {
                    AD()->group_add_user(group, user);
                }
            }

            break;
        }
    } 
}

void MembershipTab::on_context_menu(const QPoint pos) {
    const QString dn = get_dn_from_pos(pos, view, MembersColumn_DN);
    if (dn.isEmpty()) {
        return;
    }

    QMenu menu(this);
    menu.addAction(tr("Remove"),
        [this, dn]() {
            const QList<QString> removed_values = {dn};
            remove_values(removed_values);
        });

    exec_menu_from_view(&menu, view, pos);
}

void MembershipTab::on_add_button() {
    const QList<QString> classes =
    [this]() -> QList<QString> {
        switch (type) {
            case MembershipTabType_Members: return {CLASS_USER};
            case MembershipTabType_MemberOf: return {CLASS_GROUP};
        }
        return QList<QString>();
    }();
    ;
    const QList<QString> selected_objects = SelectDialog::open(classes, SelectDialogMultiSelection_Yes);

    if (selected_objects.size() > 0) {
        add_values(selected_objects);
    }
}

void MembershipTab::on_remove_button() {
    const QItemSelectionModel *selection_model = view->selectionModel();
    const QList<QModelIndex> selected = selection_model->selectedIndexes();

    QList<QString> removed_values;
    for (auto index : selected) {
        const QString dn = get_dn_from_index(index, MembersColumn_DN);

        removed_values.append(dn);
    }

    remove_values(removed_values);    
}

void MembershipTab::reload_model(const QSet<QString> &values) {
    model->removeRows(0, model->rowCount());

    for (auto dn : values) {
        const QString name = dn_get_rdn(dn);
        const QString parent = dn_get_parent(dn);
        
        const QList<QStandardItem *> row = make_item_row(MembersColumn_COUNT);
        row[MembersColumn_Name]->setText(name);
        row[MembersColumn_Parent]->setText(parent);
        row[MembersColumn_DN]->setText(dn);

        model->appendRow(row);
    }

    model->sort(MembersColumn_Name);
}

void MembershipTab::add_values(QList<QString> values) {
    for (auto value : values) {
        current_values.insert(value);
    }

    reload_model(current_values);

    emit edited();
}

void MembershipTab::remove_values(QList<QString> values) {
    for (auto value : values) {
        current_values.remove(value);
    }

    reload_model(current_values);

    emit edited();
}

QString MembershipTab::get_membership_attribute() {
    switch (type) {
        case MembershipTabType_Members: return ATTRIBUTE_MEMBER;
        case MembershipTabType_MemberOf: return ATTRIBUTE_MEMBER_OF;
    }
    return "";
}
