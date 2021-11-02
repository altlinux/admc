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

#include "tabs/security_tab.h"
#include "tabs/ui_security_tab.h"

#include "select_well_known_trustee_dialog.h"
#include "ad_security.h"
#include "adldap.h"
#include "globals.h"
#include "select_object_dialog.h"
#include "utils.h"
#include "settings.h"

#include "samba/ndr_security.h"

#include <QDebug>
#include <QLabel>
#include <QPersistentModelIndex>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>

enum TrusteeItemRole {
    TrusteeItemRole_Sid = Qt::UserRole,
};

enum AcePermissionItemRole {
    AcePermissionItemRole_Permission = Qt::UserRole,
};

QHash<AcePermission, QString> SecurityTab::ace_permission_to_name_map() {
    return {
        {AcePermission_FullControl, tr("Full control")},
        {AcePermission_Read, tr("Read")},
        {AcePermission_Write, tr("Write")},
        {AcePermission_Delete, tr("Delete")},
        {AcePermission_DeleteSubtree, tr("Delete subtree")},
        {AcePermission_CreateChild, tr("Create child")},
        {AcePermission_DeleteChild, tr("Delete child")},
        {AcePermission_AllowedToAuthenticate, tr("Allowed to authenticate")},
        {AcePermission_ChangePassword, tr("Change password")},
        {AcePermission_ReceiveAs, tr("Receive as")},
        {AcePermission_ResetPassword, tr("Reset password")},
        {AcePermission_SendAs, tr("Send as")},
        {AcePermission_ReadAccountRestrictions, tr("Read Account restrictions")},
        {AcePermission_WriteAccountRestrictions, tr("Write Account restrictions")},
        {AcePermission_ReadGeneralInfo, tr("Read general info")},
        {AcePermission_WriteGeneralInfo, tr("Write general info")},
        {AcePermission_ReadGroupMembership, tr("Read group membership")},
        {AcePermission_ReadLogonInfo, tr("Read logon info")},
        {AcePermission_WriteLogonInfo, tr("Write logon info")},
        {AcePermission_ReadPersonalInfo, tr("Read personal info")},
        {AcePermission_WritePersonalInfo, tr("Write personal info")},
        {AcePermission_ReadPhoneAndMailOptions, tr("Read phone and mail options")},
        {AcePermission_WritePhoneAndMailOptions, tr("Write phone and mail options")},
        {AcePermission_ReadPrivateInfo, tr("Read private info")},
        {AcePermission_WritePrivateInfo, tr("Write private info")},
        {AcePermission_ReadPublicInfo, tr("Read public info")},
        {AcePermission_WritePublicInfo, tr("Write public info")},
        {AcePermission_ReadRemoteAccessInfo, tr("Read remote access info")},
        {AcePermission_WriteRemoteAccessInfo, tr("Write remote access info")},
        {AcePermission_ReadTerminalServerLicenseServer, tr("Read terminal server license server")},
        {AcePermission_WriteTerminalServerLicenseServer, tr("Write terminal server license server")},
        {AcePermission_ReadWebInfo, tr("Read web info")},
        {AcePermission_WriteWebInfo, tr("Write web info")},
    };
}

SecurityTab::SecurityTab() {
    ui = new Ui::SecurityTab();
    ui->setupUi(this);

    select_well_known_trustee_dialog = new SelectWellKnownTrusteeDialog(this);

    ignore_item_changed_signal = false;

    trustee_model = new QStandardItemModel(0, 1, this);

    ui->trustee_view->setModel(trustee_model);
    ui->trustee_view->sortByColumn(0, Qt::AscendingOrder);

    ace_model = new QStandardItemModel(0, AceColumn_COUNT, this);
    set_horizontal_header_labels_from_map(ace_model,
        {
            {AceColumn_Name, tr("Name")},
            {AceColumn_Allowed, tr("Allowed")},
            {AceColumn_Denied, tr("Denied")},
        });

    // Fill ace model
    for (const AcePermission &permission : all_permissions_list) {
        const QList<QStandardItem *> row = make_item_row(AceColumn_COUNT);

        const QString mask_string = ace_permission_to_name_map()[permission];
        row[AceColumn_Name]->setText(mask_string);

        row[AceColumn_Allowed]->setCheckable(true);
        row[AceColumn_Denied]->setCheckable(true);

        row[0]->setData(permission, AcePermissionItemRole_Permission);

        ace_model->appendRow(row);
    }

    permission_item_map = [&]() {
        QHash<AcePermission, QHash<AceColumn, QStandardItem *>> out;

        for (int row = 0; row < ace_model->rowCount(); row++) {
            QStandardItem *main_item = ace_model->item(row, 0);
            const AcePermission permission = (AcePermission) main_item->data(AcePermissionItemRole_Permission).toInt();

            const QList<AceColumn> column_list = {
                AceColumn_Allowed,
                AceColumn_Denied,
            };

            for (const AceColumn &column : column_list) {
                QStandardItem *item = ace_model->item(row, column);
                out[permission][column] = item;
            }
        }

        return out;
    }();

    ui->ace_view->setModel(ace_model);
    ui->ace_view->setColumnWidth(AceColumn_Name, 400);

    settings_restore_header_state(SETTING_security_tab_header_state, ui->ace_view->header());

    connect(
        ui->trustee_view->selectionModel(), &QItemSelectionModel::currentChanged,
        this, &SecurityTab::load_trustee_acl);
    connect(
        ace_model, &QStandardItemModel::itemChanged,
        this, &SecurityTab::on_item_changed);
    connect(
        ui->add_trustee_button, &QAbstractButton::clicked,
        this, &SecurityTab::on_add_trustee_button);
    connect(
        ui->add_well_known_trustee_button, &QAbstractButton::clicked,
        select_well_known_trustee_dialog, &QDialog::open);
    connect(
        ui->remove_trustee_button, &QAbstractButton::clicked,
        this, &SecurityTab::on_remove_trustee_button);
    connect(
        select_well_known_trustee_dialog, &QDialog::accepted,
        this, &SecurityTab::on_select_well_known_trustee_dialog_accepted);
}

SecurityTab::~SecurityTab() {
    settings_save_header_state(SETTING_security_tab_header_state, ui->ace_view->header());   

    delete ui;
}

void SecurityTab::load(AdInterface &ad, const AdObject &object) {
    trustee_model->removeRows(0, trustee_model->rowCount());

    // Add items to trustee model
    const QList<QByteArray> trustee_list = ad_security_get_trustee_list_from_object(object);
    add_trustees(trustee_list, ad);

    trustee_model->sort(0, Qt::AscendingOrder);

    permission_state_map = object.get_security_state(g_adconfig);

    original_permission_state_map = permission_state_map;

    // Select first index
    // NOTE: load_trustee_acl() is called because setCurrentIndex
    // emits "current change" signal
    ui->trustee_view->selectionModel()->setCurrentIndex(trustee_model->index(0, 0), QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);

    is_policy = object.is_class(CLASS_GP_CONTAINER);

    PropertiesTab::load(ad, object);
}

void SecurityTab::load_trustee_acl() {
    const QModelIndex current_index = ui->trustee_view->currentIndex();
    if (!current_index.isValid()) {
        return;
    }
    
    apply_current_state_to_items();
}

void SecurityTab::on_item_changed(QStandardItem *item) {
    // NOTE: in some cases we need to ignore this signal
    if (ignore_item_changed_signal) {
        return;
    }

    const AceColumn column = (AceColumn) item->column();

    const bool incorrect_column = (column != AceColumn_Allowed && column != AceColumn_Denied);
    if (incorrect_column) {
        return;
    }

    const AcePermission permission = [&]() {
        QStandardItem *main_item = ace_model->item(item->row(), 0);
        const AcePermission out = (AcePermission) main_item->data(AcePermissionItemRole_Permission).toInt();

        return out;
    }();

    const PermissionState new_state = [&]() {
        const bool checked = (item->checkState() == Qt::Checked);
        const bool allowed = (column == AceColumn_Allowed);

        if (checked) {
            if (allowed) {
                return PermissionState_Allowed;
            } else {
                return PermissionState_Denied;
            }
        } else {
            // NOTE: the case of opposite column being
            // checked while this one becomes unchecked is
            // impossible, so ignore it
            return PermissionState_None;
        }
    }();

    const QByteArray trustee = [&]() {
        const QModelIndex current_index = ui->trustee_view->currentIndex();
        QStandardItem *current_item = trustee_model->itemFromIndex(current_index);
        const QByteArray out = current_item->data(TrusteeItemRole_Sid).toByteArray();

        return out;
    }();

    permission_state_map = ad_security_modify(permission_state_map, trustee, permission, new_state);
    apply_current_state_to_items();

    emit edited();
}

QStandardItem *SecurityTab::get_item(const AcePermission permission, const AceColumn column) {
    return permission_item_map[permission][column];
}

bool SecurityTab::set_trustee(const QString &trustee_name) {
    const QList<QStandardItem *> item_list = trustee_model->findItems(trustee_name);

    if (item_list.isEmpty()) {
        return false;
    }

    const QStandardItem *item = item_list[0];
    ui->trustee_view->setCurrentIndex(item->index());

    return true;
}

bool SecurityTab::verify(AdInterface &ad, const QString &target) const {
    UNUSED_ARG(target);

    if (is_policy) {
        // To apply security tab for policies we need user
        // to have admin rights to be able to sync perms of
        // GPT
        const bool have_sufficient_rights = ad.logged_in_as_admin();

        return have_sufficient_rights;
    } else {
        return true;
    }
}

bool SecurityTab::apply(AdInterface &ad, const QString &target) {
    const bool modified = (original_permission_state_map != permission_state_map);
    if (!modified) {
        return true;
    }

    bool total_success = true;
 
    total_success &= attribute_replace_security_descriptor(&ad, target, permission_state_map);

    original_permission_state_map = permission_state_map;

    if (is_policy) {
        total_success &= ad.gpo_sync_perms(target);
    }

    return total_success;
}

void SecurityTab::on_add_trustee_button() {
    auto dialog = new SelectObjectDialog({CLASS_USER, CLASS_GROUP}, SelectObjectDialogMultiSelection_Yes, this);
    dialog->select_all_classes();
    dialog->setWindowTitle(tr("Add Trustee"));

    QObject::connect(
        dialog, &SelectObjectDialog::accepted,
        [=]() {
            AdInterface ad;
            if (ad_failed(ad)) {
                return;
            }

            // Get sid's of selected objects
            const QList<QByteArray> sid_list = [&, dialog]() {
                QList<QByteArray> out;

                const QList<QString> selected_list = dialog->get_selected();
                for (const QString &dn : selected_list) {
                    const AdObject object = ad.search_object(dn, {ATTRIBUTE_OBJECT_SID});
                    const QByteArray sid = object.get_value(ATTRIBUTE_OBJECT_SID);

                    out.append(sid);
                }

                return out;
            }();

            add_trustees(sid_list, ad);
        });

    dialog->open();
}

void SecurityTab::on_remove_trustee_button() {
    QItemSelectionModel *selection_model = ui->trustee_view->selectionModel();
    const QList<QPersistentModelIndex> selected_list = persistent_index_list(selection_model->selectedRows());

    for (const QPersistentModelIndex &index : selected_list) {
        const QByteArray sid = index.data(TrusteeItemRole_Sid).toByteArray();
        permission_state_map.remove(sid);

        trustee_model->removeRow(index.row());
    }

    if (!selected_list.isEmpty()) {
        emit edited();
    }
}

// NOTE: a flag is set to avoid triggering on_item_changed()
// slot due to setCheckState() calls. Otherwise it would
// recurse and do all kinds of bad stuff.
void SecurityTab::apply_current_state_to_items() {
    ignore_item_changed_signal = true;

    const QByteArray trustee = [&]() {
        const QModelIndex current_index = ui->trustee_view->currentIndex();
        QStandardItem *current_item = trustee_model->itemFromIndex(current_index);
        const QByteArray out = current_item->data(TrusteeItemRole_Sid).toByteArray();

        return out;
    }();

    for (const AcePermission &permission : all_permissions) {
        QStandardItem *allowed = permission_item_map[permission][AceColumn_Allowed];
        QStandardItem *denied = permission_item_map[permission][AceColumn_Denied];
        const PermissionState state = permission_state_map[trustee][permission];

        switch (state) {
            case PermissionState_None: {
                allowed->setCheckState(Qt::Unchecked);
                denied->setCheckState(Qt::Unchecked);
                break;
            };
            case PermissionState_Allowed: {
                allowed->setCheckState(Qt::Checked);
                denied->setCheckState(Qt::Unchecked);
                break;
            }
            case PermissionState_Denied: {
                allowed->setCheckState(Qt::Unchecked);
                denied->setCheckState(Qt::Checked);
                break;
            }
        }
    }

    ignore_item_changed_signal = false;
}

void SecurityTab::add_trustees(const QList<QByteArray> &sid_list, AdInterface &ad) {
    const QList<QString> current_sid_string_list = [&]() {
        QList<QString> out;

        for (int row = 0; row < trustee_model->rowCount(); row++) {
            QStandardItem *item = trustee_model->item(row, 0);
            const QByteArray sid = item->data(TrusteeItemRole_Sid).toByteArray();
            const QString sid_string = object_sid_display_value(sid);

            out.append(sid_string);
        }

        return out;
    }();

    bool added_anything = false;
    bool failed_to_add_because_already_exists = false;

    for (const QByteArray &sid : sid_list) {
        const QString sid_string = object_sid_display_value(sid);
        const bool trustee_already_in_list = (current_sid_string_list.contains(sid_string));
        if (trustee_already_in_list) {
            failed_to_add_because_already_exists = true;

            continue;
        }

        auto item = new QStandardItem();
        const QString name = ad_security_get_trustee_name(ad, sid);
        item->setText(name);
        item->setData(sid, TrusteeItemRole_Sid);
        trustee_model->appendRow(item);
        
        added_anything = true;
    }

    if (added_anything) {
        emit edited();
    }

    if (failed_to_add_because_already_exists) {
        message_box_warning(this, tr("Error"), tr("Failed to add some trustee's because they are already in the list."));
    }
}

void SecurityTab::on_select_well_known_trustee_dialog_accepted() {
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    const QList<QByteArray> trustee_list = select_well_known_trustee_dialog->get_selected();

    add_trustees(trustee_list, ad);
}
