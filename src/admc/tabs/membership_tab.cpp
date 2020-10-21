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
#include "details_dialog.h"
#include "ad_interface.h"
#include "utils.h"
#include "select_dialog.h"
#include "filter.h"
#include "attribute_display.h"

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
    MembersColumn_Primary,
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
        {MembersColumn_Primary, tr("Primary")},
        {MembersColumn_DN, tr("DN")}
    });

    view->setModel(model);

    setup_column_toggle_menu(view, model, {MembersColumn_Name, MembersColumn_Parent});

    auto add_button = new QPushButton(tr("Add"));
    remove_button = new QPushButton(tr("Remove"));
    auto button_layout = new QHBoxLayout();
    button_layout->addWidget(add_button);
    button_layout->addWidget(remove_button);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(view);
    layout->addLayout(button_layout);

    const QItemSelectionModel *selection_model = view->selectionModel();
    connect(
        selection_model, &QItemSelectionModel::selectionChanged,
        this, &MembershipTab::on_selection_changed);
    on_selection_changed();

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

    // Add primary groups or primary members
    original_primary_values.clear();
    switch (type) {
        case MembershipTabType_Members: {
            // Get users who have this group as primary group
            const QByteArray group_sid = object.get_value(ATTRIBUTE_OBJECT_SID);
            const QString group_sid_string = object_sid_to_display_value(group_sid);
            const int cut_index = group_sid_string.lastIndexOf("-") + 1;
            const QString group_rid = group_sid_string.mid(cut_index);

            const QString filter = filter_EQUALS(ATTRIBUTE_PRIMARY_GROUP_ID, group_rid);
            const QHash<QString, AdObject> result = AD()->search(filter, QList<QString>(), SearchScope_All);

            for (const QString user : result.keys()) {
                original_primary_values.insert(user);
            }

            break;
        }
        case MembershipTabType_MemberOf: {
            // Get primary group's dn
            // Need to first construct group sid from ATTRIBUTE_PRIMARY_GROUP_ID
            // and then search for object with that sid to get dn

            // Construct group sid from group rid + user sid
            // user sid  = "S-foo-bar-baz-abc"
            // group rid = "xyz" 
            // group sid = "S-foo-bar-baz-xyz"
            const QString group_rid = object.get_string(ATTRIBUTE_PRIMARY_GROUP_ID);
            const QByteArray user_sid = object.get_value(ATTRIBUTE_OBJECT_SID);
            const QString user_sid_string = object_sid_to_display_value(user_sid);
            const int cut_index = user_sid_string.lastIndexOf("-") + 1;
            const QString group_sid = user_sid_string.left(cut_index) + group_rid;

            const QString filter = filter_EQUALS(ATTRIBUTE_OBJECT_SID, group_sid);
            const QHash<QString, AdObject> result = AD()->search(filter, QList<QString>(), SearchScope_All);

            if (!result.isEmpty()) {
                const QString group_dn = result.values()[0].get_dn();
                original_primary_values.insert(group_dn);
            }

            break;
        }
    } 

    current_primary_values = original_primary_values;

    reload_model();
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
    menu.addAction(tr("Details"), [this, dn]() {
        DetailsDialog::open_for_target(dn);
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

void MembershipTab::on_selection_changed() {
    const QItemSelectionModel *selection_model = view->selectionModel();
    const QList<QModelIndex> selected = selection_model->selectedIndexes();
    const bool any_selected = !selected.isEmpty();

    remove_button->setEnabled(any_selected);
}

void MembershipTab::reload_model() {
    model->removeRows(0, model->rowCount());

    const QSet<QString> all_values = current_values + current_primary_values;

    for (auto dn : all_values) {
        const QString name = dn_get_rdn(dn);
        const QString parent = dn_get_parent(dn);
        const bool primary = current_primary_values.contains(dn);
        const Qt::CheckState check_state =
        [primary]() {
            if (primary) {
                return Qt::Checked;
            } else {
                return Qt::Unchecked;
            }
        }();
        
        const QList<QStandardItem *> row = make_item_row(MembersColumn_COUNT);
        row[MembersColumn_Name]->setText(name);
        row[MembersColumn_Parent]->setText(parent);
        row[MembersColumn_Primary]->setCheckState(check_state);
        row[MembersColumn_DN]->setText(dn);

        // Make primary non selectable so you cant remove them
        for (int i = 0; i < MembersColumn_COUNT; i++) {
            row[i]->setSelectable(!primary);
        }

        model->appendRow(row);
    }

    model->sort(MembersColumn_Name);
}

void MembershipTab::add_values(QList<QString> values) {
    for (auto value : values) {
        current_values.insert(value);
    }

    reload_model();

    emit edited();
}

void MembershipTab::remove_values(QList<QString> values) {
    for (auto value : values) {
        current_values.remove(value);
    }

    reload_model();

    emit edited();
}

QString MembershipTab::get_membership_attribute() {
    switch (type) {
        case MembershipTabType_Members: return ATTRIBUTE_MEMBER;
        case MembershipTabType_MemberOf: return ATTRIBUTE_MEMBER_OF;
    }
    return "";
}
