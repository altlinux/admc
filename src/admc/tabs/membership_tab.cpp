
/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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
#include "tabs/ui_membership_tab.h"

#include "adldap.h"
#include "globals.h"
#include "properties_dialog.h"
#include "select_object_dialog.h"
#include "utils.h"
#include "settings.h"

#include <QDebug>
#include <QStandardItemModel>

// Store members in a set
// Generate model from current members list
// Add new members via select dialog
// Remove through context menu or select+remove button

enum MembersColumn {
    MembersColumn_Name,
    MembersColumn_Parent,
    MembersColumn_COUNT,
};

enum MembersRole {
    MembersRole_DN = Qt::UserRole + 1,
    MembersRole_Primary = Qt::UserRole + 2,
};

MembersTab::MembersTab()
: MembershipTab(MembershipTabType_Members) {
}

MemberOfTab::MemberOfTab()
: MembershipTab(MembershipTabType_MemberOf) {
}

MembershipTab::MembershipTab(const MembershipTabType type_arg) {
    ui = new Ui::MembershipTab();
    ui->setupUi(this);

    type = type_arg;

    model = new QStandardItemModel(0, MembersColumn_COUNT, this);
    set_horizontal_header_labels_from_map(model,
        {
            {MembersColumn_Name, tr("Name")},
            {MembersColumn_Parent, tr("Folder")},
        });

    ui->view->setModel(model);

    // Primary group widgets are visible only in Member of version
    if (type == MembershipTabType_Members) {
        ui->primary_button->hide();
        ui->primary_group_label->hide();
    }

    settings_restore_header_state(SETTING_membership_tab_header_state, ui->view->header());

    enable_widget_on_selection(ui->remove_button, ui->view);
    enable_widget_on_selection(ui->properties_button, ui->view);

    const QItemSelectionModel *selection_model = ui->view->selectionModel();
    connect(
        selection_model, &QItemSelectionModel::selectionChanged,
        this, &MembershipTab::enable_primary_button_on_valid_selection);
    enable_primary_button_on_valid_selection();

    connect(
        ui->remove_button, &QAbstractButton::clicked,
        this, &MembershipTab::on_remove_button);
    connect(
        ui->add_button, &QAbstractButton::clicked,
        this, &MembershipTab::on_add_button);
    connect(
        ui->properties_button, &QAbstractButton::clicked,
        this, &MembershipTab::on_properties_button);
    connect(
        ui->primary_button, &QAbstractButton::clicked,
        this, &MembershipTab::on_primary_button);

    PropertiesDialog::open_when_view_item_activated(ui->view, MembersRole_DN);
}

MembershipTab::~MembershipTab() {
    settings_save_header_state(SETTING_membership_tab_header_state, ui->view->header());   

    delete ui;
}

void MembershipTab::load(AdInterface &ad, const AdObject &object) {
    const QList<QString> values = object.get_strings(get_membership_attribute());
    original_values = values.toSet();
    current_values = original_values;

    // Add primary groups or primary members
    original_primary_values.clear();
    switch (type) {
        case MembershipTabType_Members: {
            // Get users who have this group as primary group
            const QByteArray group_sid = object.get_value(ATTRIBUTE_OBJECT_SID);
            const QString group_rid = extract_rid_from_sid(group_sid, g_adconfig);

            const QString base = g_adconfig->domain_head();
            const SearchScope scope = SearchScope_All;
            const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_PRIMARY_GROUP_ID, group_rid);
            const QList<QString> attributes = QList<QString>();
            const QHash<QString, AdObject> results = ad.search(base, scope, filter, attributes);

            for (const QString &user : results.keys()) {
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
            const QString user_sid_string = attribute_display_value(ATTRIBUTE_OBJECT_SID, user_sid, g_adconfig);
            const int cut_index = user_sid_string.lastIndexOf("-") + 1;
            const QString group_sid = user_sid_string.left(cut_index) + group_rid;

            const QString base = g_adconfig->domain_head();
            const SearchScope scope = SearchScope_All;
            const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_SID, group_sid);
            const QList<QString> attributes = QList<QString>();
            const QHash<QString, AdObject> results = ad.search(base, scope, filter, attributes);

            if (!results.isEmpty()) {
                const QString group_dn = results.values()[0].get_dn();
                original_primary_values.insert(group_dn);
            }

            break;
        }
    }

    current_primary_values = original_primary_values;

    reload_model();
}

bool MembershipTab::apply(AdInterface &ad, const QString &target) {
    bool total_success = true;

    // NOTE: need temp copy because can't edit the set
    // during iteration
    QSet<QString> new_original_values = original_values;
    QSet<QString> new_original_primary_values = original_primary_values;

    // NOTE: logic is kinda duplicated but switching on behavior within iterations would be very confusing
    switch (type) {
        case MembershipTabType_Members: {
            const QString group = target;

            for (auto user : original_values) {
                const bool removed = !current_values.contains(user);
                if (removed) {
                    const bool success = ad.group_remove_member(group, user);
                    if (success) {
                        new_original_values.remove(user);
                    } else {
                        total_success = false;
                    }
                }
            }

            for (auto user : current_values) {
                const bool added = !original_values.contains(user);
                if (added) {
                    const bool success = ad.group_add_member(group, user);
                    if (success) {
                        new_original_values.insert(user);
                    } else {
                        total_success = false;
                    }
                }
            }

            break;
        }
        case MembershipTabType_MemberOf: {
            const QString user = target;

            // NOTE: must change primary group before
            // remove/add operations otherwise there will be
            // conflicts

            // Change primary group
            if (current_primary_values != original_primary_values) {
                const QString original_primary_group = original_primary_values.values()[0];
                const QString group_dn = current_primary_values.values()[0];

                const bool success = ad.user_set_primary_group(group_dn, target);
                if (success) {
                    new_original_primary_values = {group_dn};

                    // Server adds old primary group to
                    // normal membership
                    new_original_values.insert(original_primary_group);

                    // Server removes new primary group from
                    // normal membership
                    new_original_values.remove(group_dn);
                } else {
                    total_success = false;
                }
            }

            // When setting primary groups, the server
            // performs some membership modifications on
            // it's end. Therefore, don't need to do
            // anything with groups that were or are primary.
            auto group_is_or_was_primary = [this](const QString &group) {
                return original_primary_values.contains(group) || current_primary_values.contains(group);
            };

            // Remove user from groups that were removed
            for (auto group : original_values) {
                if (group_is_or_was_primary(group)) {
                    continue;
                }

                const bool removed = !current_values.contains(group);
                if (removed) {
                    const bool success = ad.group_remove_member(group, user);
                    if (success) {
                        new_original_values.remove(group);
                    } else {
                        total_success = false;
                    }
                }
            }

            // Add user to groups that were added
            for (auto group : current_values) {
                if (group_is_or_was_primary(group)) {
                    continue;
                }

                const bool added = !original_values.contains(group);
                if (added) {
                    const bool success = ad.group_add_member(group, user);
                    if (success) {
                        new_original_values.insert(group);
                    } else {
                        total_success = false;
                    }
                }
            }

            break;
        }
    }

    original_values = new_original_values;
    original_primary_values = new_original_primary_values;

    return total_success;
}

void MembershipTab::on_add_button() {
    // TODO: aduc has "other objects" section in class selection for adding members to groups. No idea what "other objects" are. No results come up when searching for other objects in current test domain.
    // TODO: there's also "service account", no idea what that is either.
    const QList<QString> classes = [this]() -> QList<QString> {
        switch (type) {
            case MembershipTabType_Members: return {
                CLASS_USER,
                CLASS_GROUP,
                CLASS_CONTACT,
                CLASS_COMPUTER,
            };
            case MembershipTabType_MemberOf: return {CLASS_GROUP};
        }
        return QList<QString>();
    }();

    auto dialog = new SelectObjectDialog(classes, SelectObjectDialogMultiSelection_Yes, this);

    const QString title = [&]() {
        switch (type) {
            case MembershipTabType_Members: return tr("Add Member");
            case MembershipTabType_MemberOf: return tr("Add to Group");
        }
        return QString();
    }();
    dialog->setWindowTitle(title);

    connect(
        dialog, &SelectObjectDialog::accepted,
        [this, dialog]() {
            const QList<QString> selected = dialog->get_selected();

            add_values(selected);
        });

    dialog->open();
}

void MembershipTab::on_remove_button() {
    const QItemSelectionModel *selection_model = ui->view->selectionModel();
    const QList<QModelIndex> selected = selection_model->selectedRows();

    QList<QString> removed_values;
    for (auto index : selected) {
        const QString dn = index.data(MembersRole_DN).toString();

        removed_values.append(dn);
    }

    const bool any_selected_are_primary = [this, removed_values]() {
        for (const QString &dn : removed_values) {
            if (current_primary_values.contains(dn)) {
                return true;
            }
        }

        return false;
    }();

    if (any_selected_are_primary) {
        const QString error_text = [this]() {
            switch (type) {
                case MembershipTabType_Members: return tr("Can't remove because this group is a primary group to selected user.");
                case MembershipTabType_MemberOf: return tr("Can't remove because selected group is a primary group to this user.");
            }
            return QString();
        }();

        message_box_warning(this, tr("Error"), error_text);
    } else {
        remove_values(removed_values);
    }
}

void MembershipTab::on_primary_button() {
    // Make selected group primary
    const QItemSelectionModel *selection_model = ui->view->selectionModel();
    const QList<QModelIndex> selecteds = selection_model->selectedRows();
    const QModelIndex selected = selecteds[0];
    const QString group_dn = selected.data(MembersRole_DN).toString();

    // Old primary group becomes normal
    current_values.unite(current_primary_values);
    // New primary group stops being normal
    current_values.remove(group_dn);
    // and becomes primary
    current_primary_values = {group_dn};

    reload_model();

    emit edited();
}

void MembershipTab::on_properties_button() {
    const QModelIndex current = ui->view->selectionModel()->currentIndex();
    const QString dn = current.data(MembersRole_DN).toString();

    PropertiesDialog::open_for_target(dn);
}

void MembershipTab::enable_primary_button_on_valid_selection() {
    if (ui->primary_button == nullptr) {
        return;
    }

    const QItemSelectionModel *selection_model = ui->view->selectionModel();
    const QList<QModelIndex> selecteds = selection_model->selectedRows();

    // Enable "set primary group" button if
    // 1) there's a selection
    // 2) the selected group is NOT primary already
    const QSet<QString> selected_dns = [selecteds]() {
        QSet<QString> out;
        for (const QModelIndex selected : selecteds) {
            const QString dn = selected.data(MembersRole_DN).toString();
            out.insert(dn);
        }
        return out;
    }();

    if (selected_dns.size() == 1) {
        const QString dn = selected_dns.values()[0];
        const bool is_primary = current_primary_values.contains(dn);

        ui->primary_button->setEnabled(!is_primary);
    } else {
        ui->primary_button->setEnabled(false);
    }
}

void MembershipTab::reload_model() {
    // Load primary group name into label
    if (type == MembershipTabType_MemberOf) {
        const QString primary_group_label_text = [this]() {
            QString out = tr("Primary group: ");

            if (!current_primary_values.isEmpty()) {
                const QString primary_group_dn = current_primary_values.values()[0];
                const QString primary_group_name = dn_get_name(primary_group_dn);

                out += primary_group_name;
            }

            return out;
        }();

        ui->primary_group_label->setText(primary_group_label_text);
    }

    model->removeRows(0, model->rowCount());

    const QSet<QString> all_values = current_values + current_primary_values;

    for (auto dn : all_values) {
        const QString name = dn_get_name(dn);
        const QString parent = dn_get_parent_canonical(dn);

        const QList<QStandardItem *> row = make_item_row(MembersColumn_COUNT);
        row[MembersColumn_Name]->setText(name);
        row[MembersColumn_Parent]->setText(parent);

        set_data_for_row(row, dn, MembersRole_DN);

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
