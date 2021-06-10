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

#include "adldap.h"
#include "ad_security.h"
#include "utils.h"
#include "globals.h"
#include "select_object_dialog.h"

#include "samba/ndr_security.h"

#include <QVBoxLayout>
#include <QDebug>
#include <QTreeView>
#include <QStandardItemModel>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QPersistentModelIndex>
#include <QMessageBox>

enum TrusteeItemRole {
    TrusteeItemRole_Sid = Qt::UserRole,
};

#define trS(x) QCoreApplication::translate("Security", x)
const QHash<AcePermission, QString> ace_permission_to_name_map = {
    {AcePermission_FullControl, trS("Full control")},
    {AcePermission_Read, trS("Read")},
    {AcePermission_Write, trS("Write")},
    {AcePermission_CreateChild, trS("Create child")},
    {AcePermission_DeleteChild, trS("Delete child")},
    {AcePermission_AllowedToAuthenticate, trS("Allowed to authenticate")},
    {AcePermission_ChangePassword, trS("Change password")},
    {AcePermission_ReceiveAs, trS("Receive as")},
    {AcePermission_ResetPassword, trS("Reset password")},
    {AcePermission_SendAs, trS("Send as")},
    {AcePermission_ReadAccountRestrictions, trS("Read Account restrictions")},
    {AcePermission_WriteAccountRestrictions, trS("Write Account restrictions")},
    {AcePermission_ReadGeneralInfo, trS("Read general info")},
    {AcePermission_WriteGeneralInfo, trS("Write general info")},
    {AcePermission_ReadGroupMembership, trS("Read group membership")},
    {AcePermission_ReadLogonInfo, trS("Read logon info")},
    {AcePermission_WriteLogonInfo, trS("Write logon info")},
    {AcePermission_ReadPersonalInfo, trS("Read personal info")},
    {AcePermission_WritePersonalInfo, trS("Write personal info")},
    {AcePermission_ReadPhoneAndMailOptions, trS("Read phone and mail options")},
    {AcePermission_WritePhoneAndMailOptions, trS("Write phone and mail options")},
    {AcePermission_ReadPrivateInfo, trS("Read private info")},
    {AcePermission_WritePrivateInfo, trS("Write private info")},
    {AcePermission_ReadPublicInfo, trS("Read public info")},
    {AcePermission_WritePublicInfo, trS("Write public info")},
    {AcePermission_ReadRemoteAccessInfo, trS("Read remote access info")},
    {AcePermission_WriteRemoteAccessInfo, trS("Write remote access info")},
    {AcePermission_ReadTerminalServerLicenseServer, trS("Read terminal server license server")},
    {AcePermission_WriteTerminalServerLicenseServer, trS("Write terminal server license server")},
    {AcePermission_ReadWebInfo, trS("Read web info")},
    {AcePermission_WriteWebInfo, trS("Write web info")},
};

SecurityTab::SecurityTab() {
    ignore_item_changed_signal = false;
    
    trustee_model = new QStandardItemModel(0, 1, this);
    
    trustee_view = new QTreeView(this);
    trustee_view->setHeaderHidden(true);
    trustee_view->setModel(trustee_model);
    trustee_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    trustee_view->sortByColumn(0, Qt::AscendingOrder);
    trustee_view->setSortingEnabled(true);
    trustee_view->setSelectionMode(QAbstractItemView::SingleSelection);

    auto trustee_buttonbox = new QDialogButtonBox();
    auto add_trustee_button = trustee_buttonbox->addButton(tr("Add"), QDialogButtonBox::ActionRole);
    auto remove_trustee_button = trustee_buttonbox->addButton(tr("Remove"), QDialogButtonBox::ActionRole);

    ace_model = new QStandardItemModel(0, AceColumn_COUNT, this);
    set_horizontal_header_labels_from_map(ace_model, {
        {AceColumn_Name, tr("Name")},
        {AceColumn_Allowed, tr("Allowed")},
        {AceColumn_Denied, tr("Denied")},
    });

    // Fill ace model
    for (const AcePermission &permission : all_permissions_list) {
        const QList<QStandardItem *> row = make_item_row(AceColumn_COUNT);

        const QString mask_string = ace_permission_to_name_map[permission];
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
    
    ace_view = new QTreeView(this);
    ace_view->setModel(ace_model);
    ace_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ace_view->setColumnWidth(AceColumn_Name, 400);

    trustee_label = new QLabel();

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(trustee_view);
    layout->addWidget(trustee_buttonbox);
    layout->addWidget(trustee_label);
    layout->addWidget(ace_view);

    connect(
        trustee_view->selectionModel(), &QItemSelectionModel::currentChanged,
        this, &SecurityTab::load_trustee_acl);
    connect(
        ace_model, &QStandardItemModel::itemChanged,
        this, &SecurityTab::on_item_changed);
    connect(
        add_trustee_button, &QAbstractButton::clicked,
        this, &SecurityTab::add_trustee);
    connect(
        remove_trustee_button, &QAbstractButton::clicked,
        this, &SecurityTab::remove_trustee);
}

void SecurityTab::load(AdInterface &ad, const AdObject &object) {
    trustee_model->removeRows(0, trustee_model->rowCount());
    
    // Add items to trustee model
    const QList<QByteArray> trustee_list = ad_security_get_trustee_list_from_object(&object);
    for (const QByteArray &trustee : trustee_list) {
        add_trustee_item(trustee, ad);
    }

    trustee_model->sort(0, Qt::AscendingOrder);

    permission_state_map = ad_security_get_state_from_object(&object, g_adconfig);

    original_permission_state_map = permission_state_map;

    // Select first index
    // NOTE: load_trustee_acl() is called because setCurrentIndex
    // emits "current change" signal
    trustee_view->selectionModel()->setCurrentIndex(trustee_model->index(0, 0), QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);

    PropertiesTab::load(ad, object);
}

void SecurityTab::load_trustee_acl() {
    const QString label_text = [&]() {
        const QModelIndex current_index = trustee_view->currentIndex();
        QStandardItem *current_item = trustee_model->itemFromIndex(current_index);
        const QString trustee_name = current_item->text();
        const QString text = QString("Permissions for %1").arg(trustee_name);

        return text;
    }();

    trustee_label->setText(label_text);

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
        const QModelIndex current_index = trustee_view->currentIndex();
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
    trustee_view->setCurrentIndex(item->index());

    return true;
}

bool SecurityTab::apply(AdInterface &ad, const QString &target) {
    const bool modified = (original_permission_state_map != permission_state_map);
    if (!modified) {
        return true;
    }

    const bool apply_success = attribute_replace_security_descriptor(&ad, target, permission_state_map);

    original_permission_state_map = permission_state_map;

    return apply_success;
}

void SecurityTab::add_trustee() {
    auto dialog = new SelectObjectDialog({CLASS_USER, CLASS_GROUP}, SelectObjectDialogMultiSelection_Yes, this);

    QObject::connect(
        dialog, &SelectObjectDialog::accepted,
        [=]() {
            AdInterface ad;
            if (ad_failed(ad)) {
                return;
            }

            const QList<QString> selected_list = dialog->get_selected();

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

            for (const QString &dn : selected_list) {
                const AdObject object = ad.search_object(dn, {ATTRIBUTE_OBJECT_SID});
                const QByteArray sid = object.get_value(ATTRIBUTE_OBJECT_SID);
                const QString sid_string = object_sid_display_value(sid);

                const bool trustee_already_in_list = (current_sid_string_list.contains(sid_string));
                if (trustee_already_in_list) {
                    continue;
                }

                add_trustee_item(sid, ad);
                added_anything = true;
            }

            if (added_anything) {
                trace();

                emit edited();
            }

            const bool failed_to_add_because_already_exists = (!added_anything && !selected_list.isEmpty());
            if (failed_to_add_because_already_exists) {
                trace();
                QMessageBox::warning(this, tr("Error"), tr("Failed to add some trustee's because they are already in the list."));
            }
        });

    dialog->open();
}

void SecurityTab::remove_trustee() {
    QItemSelectionModel *selection_model = trustee_view->selectionModel();
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

void SecurityTab::add_trustee_item(const QByteArray &sid, AdInterface &ad) {
    auto item = new QStandardItem();

    const QString name = ad_security_get_trustee_name(ad, sid);
    item->setText(name);

    item->setData(sid, TrusteeItemRole_Sid);

    trustee_model->appendRow(item);
}

// NOTE: a flag is set to avoid triggering on_item_changed()
// slot due to setCheckState() calls. Otherwise it would
// recurse and do all kinds of bad stuff.
void SecurityTab::apply_current_state_to_items() {
    ignore_item_changed_signal = true;
   
    const QByteArray trustee = [&]() {
        const QModelIndex current_index = trustee_view->currentIndex();
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
