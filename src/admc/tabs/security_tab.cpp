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
#include "utils.h"
#include "globals.h"

#include "samba/ndr_security.h"

#include <QVBoxLayout>
#include <QDebug>
#include <QTreeView>
#include <QStandardItemModel>
#include <QLabel>

enum TrusteeItemRole {
    TrusteeItemRole_Sid = Qt::UserRole,
    TrusteeItemRole_SidRaw = Qt::UserRole + 1,
};

enum AcePermissionItemRole {
    AcePermissionItemRole_Permission = Qt::UserRole,
};

enum AceColumn {
    AceColumn_Name,
    AceColumn_Allowed,
    AceColumn_Denied,

    AceColumn_COUNT,
};

enum PermissionState {
    PermissionState_None,
    PermissionState_Allowed,
    PermissionState_Denied,
};

enum AcePermission {
    AcePermission_FullControl,
    AcePermission_Read,
    AcePermission_Write,
    AcePermission_CreateChild,
    AcePermission_DeleteChild,
    AcePermission_AllowedToAuthenticate,
    AcePermission_ChangePassword,
    AcePermission_ReceiveAs,
    AcePermission_ResetPassword,
    AcePermission_SendAs,
    AcePermission_ReadAccountRestrictions,
    AcePermission_WriteAccountRestrictions,
    AcePermission_ReadGeneralInfo,
    AcePermission_WriteGeneralInfo,
    AcePermission_ReadGroupMembership,
    AcePermission_ReadLogonInfo,
    AcePermission_WriteLogonInfo,
    AcePermission_ReadPersonalInfo,
    AcePermission_WritePersonalInfo,
    AcePermission_ReadPhoneAndMailOptions,
    AcePermission_WritePhoneAndMailOptions,
    AcePermission_ReadPrivateInfo,
    AcePermission_WritePrivateInfo,
    AcePermission_ReadPublicInfo,
    AcePermission_WritePublicInfo,
    AcePermission_ReadRemoteAccessInfo,
    AcePermission_WriteRemoteAccessInfo,
    AcePermission_ReadTerminalServerLicenseServer,
    AcePermission_WriteTerminalServerLicenseServer,
    AcePermission_ReadWebInfo,
    AcePermission_WriteWebInfo,
    
    AcePermission_COUNT,
};

#define ENUM_TO_STRING(ENUM) {ENUM, #ENUM}

const QHash<AcePermission, QString> ace_permission_to_name_map = {
    ENUM_TO_STRING(AcePermission_FullControl),
    ENUM_TO_STRING(AcePermission_Read),
    ENUM_TO_STRING(AcePermission_Write),
    ENUM_TO_STRING(AcePermission_CreateChild),
    ENUM_TO_STRING(AcePermission_DeleteChild),
    ENUM_TO_STRING(AcePermission_AllowedToAuthenticate),
    ENUM_TO_STRING(AcePermission_ChangePassword),
    ENUM_TO_STRING(AcePermission_ReceiveAs),
    ENUM_TO_STRING(AcePermission_ResetPassword),
    ENUM_TO_STRING(AcePermission_SendAs),
    ENUM_TO_STRING(AcePermission_ReadAccountRestrictions),
    ENUM_TO_STRING(AcePermission_WriteAccountRestrictions),
    ENUM_TO_STRING(AcePermission_ReadGeneralInfo),
    ENUM_TO_STRING(AcePermission_WriteGeneralInfo),
    ENUM_TO_STRING(AcePermission_ReadGroupMembership),
    ENUM_TO_STRING(AcePermission_ReadLogonInfo),
    ENUM_TO_STRING(AcePermission_WriteLogonInfo),
    ENUM_TO_STRING(AcePermission_ReadPersonalInfo),
    ENUM_TO_STRING(AcePermission_WritePersonalInfo),
    ENUM_TO_STRING(AcePermission_ReadPhoneAndMailOptions),
    ENUM_TO_STRING(AcePermission_WritePhoneAndMailOptions),
    ENUM_TO_STRING(AcePermission_ReadPrivateInfo),
    ENUM_TO_STRING(AcePermission_WritePrivateInfo),
    ENUM_TO_STRING(AcePermission_ReadPublicInfo),
    ENUM_TO_STRING(AcePermission_WritePublicInfo),
    ENUM_TO_STRING(AcePermission_ReadRemoteAccessInfo),
    ENUM_TO_STRING(AcePermission_WriteRemoteAccessInfo),
    ENUM_TO_STRING(AcePermission_ReadTerminalServerLicenseServer),
    ENUM_TO_STRING(AcePermission_WriteTerminalServerLicenseServer),
    ENUM_TO_STRING(AcePermission_ReadWebInfo),
    ENUM_TO_STRING(AcePermission_WriteWebInfo),
};

// TODO: values of SEC_ADS_GENERIC_READ and
// SEC_ADS_GENERIC_WRITE constants don't match with the bits
// that ADUC sets when you enable those permissions in
// security tab. There are some extra bits in these
// constants, took them out as a quick fix.
const QHash<AcePermission, uint32_t> ace_permission_to_mask_map = {
    {AcePermission_FullControl, SEC_ADS_GENERIC_ALL},
    // {AcePermission_Read, SEC_ADS_GENERIC_READ},
    {AcePermission_Read, (SEC_STD_READ_CONTROL | SEC_ADS_LIST | SEC_ADS_READ_PROP)},
    // {AcePermission_Write, SEC_ADS_GENERIC_WRITE},
    {AcePermission_Write, (SEC_ADS_SELF_WRITE | SEC_ADS_WRITE_PROP)},
    {AcePermission_CreateChild, SEC_ADS_CREATE_CHILD},
    {AcePermission_DeleteChild, SEC_ADS_DELETE_CHILD},
    {AcePermission_AllowedToAuthenticate, SEC_ADS_CONTROL_ACCESS},
    {AcePermission_ChangePassword, SEC_ADS_CONTROL_ACCESS},
    {AcePermission_ReceiveAs, SEC_ADS_CONTROL_ACCESS},
    {AcePermission_ResetPassword, SEC_ADS_CONTROL_ACCESS},
    {AcePermission_SendAs, SEC_ADS_CONTROL_ACCESS},
    {AcePermission_ReadAccountRestrictions, SEC_ADS_READ_PROP},
    {AcePermission_WriteAccountRestrictions, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadGeneralInfo, SEC_ADS_READ_PROP},
    {AcePermission_WriteGeneralInfo, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadGroupMembership, SEC_ADS_READ_PROP},
    {AcePermission_ReadLogonInfo, SEC_ADS_READ_PROP},
    {AcePermission_WriteLogonInfo, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadPersonalInfo, SEC_ADS_READ_PROP},
    {AcePermission_WritePersonalInfo, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadPhoneAndMailOptions, SEC_ADS_READ_PROP},
    {AcePermission_WritePhoneAndMailOptions, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadPrivateInfo, SEC_ADS_READ_PROP},
    {AcePermission_WritePrivateInfo, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadPublicInfo, SEC_ADS_READ_PROP},
    {AcePermission_WritePublicInfo, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadRemoteAccessInfo, SEC_ADS_READ_PROP},
    {AcePermission_WriteRemoteAccessInfo, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadTerminalServerLicenseServer, SEC_ADS_READ_PROP},
    {AcePermission_WriteTerminalServerLicenseServer, SEC_ADS_WRITE_PROP},
    {AcePermission_ReadWebInfo, SEC_ADS_READ_PROP},
    {AcePermission_WriteWebInfo, SEC_ADS_WRITE_PROP},
};

// NOTE: store right's cn value here, then search for it to
// get right's guid, which is compared to ace type.
const QHash<AcePermission, QString> ace_permission_to_type_map = {
    {AcePermission_AllowedToAuthenticate, "Allowed-To-Authenticate"},
    {AcePermission_ChangePassword, "User-Change-Password"},
    {AcePermission_ReceiveAs, "Receive-As"},
    {AcePermission_ResetPassword, "User-Force-Change-Password"},
    {AcePermission_SendAs, "Send-As"},
    {AcePermission_ReadAccountRestrictions, "User-Account-Restrictions"},
    {AcePermission_WriteAccountRestrictions, "User-Account-Restrictions"},
    {AcePermission_ReadGeneralInfo, "General-Information"},
    {AcePermission_WriteGeneralInfo, "General-Information"},
    {AcePermission_ReadGroupMembership, "Membership"},
    {AcePermission_ReadLogonInfo, "User-Logon"},
    {AcePermission_WriteLogonInfo, "User-Logon"},
    {AcePermission_ReadPersonalInfo, "Personal-Information"},
    {AcePermission_WritePersonalInfo, "Personal-Information"},
    {AcePermission_ReadPhoneAndMailOptions, "Email-Information"},
    {AcePermission_WritePhoneAndMailOptions, "Email-Information"},
    {AcePermission_ReadPrivateInfo, "Private-Information"},
    {AcePermission_WritePrivateInfo, "Private-Information"},
    {AcePermission_ReadPublicInfo, "Public-Information"},
    {AcePermission_WritePublicInfo, "Public-Information"},
    {AcePermission_ReadRemoteAccessInfo, "RAS-Information"},
    {AcePermission_WriteRemoteAccessInfo, "RAS-Information"},
    {AcePermission_ReadTerminalServerLicenseServer, "Terminal-Server-License-Server"},
    {AcePermission_WriteTerminalServerLicenseServer, "Terminal-Server-License-Server"},
    {AcePermission_ReadWebInfo, "Web-Information"},
    {AcePermission_WriteWebInfo, "Web-Information"}
};

SecurityTab::SecurityTab() {
    trustee_model = new QStandardItemModel(0, 1, this);
    
    trustee_view = new QTreeView(this);
    trustee_view->setHeaderHidden(true);
    trustee_view->setModel(trustee_model);
    trustee_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    trustee_view->sortByColumn(0, Qt::AscendingOrder);
    trustee_view->setSortingEnabled(true);

    ace_model = new QStandardItemModel(0, AceColumn_COUNT, this);
    set_horizontal_header_labels_from_map(ace_model, {
        {AceColumn_Name, tr("Name")},
        {AceColumn_Allowed, tr("Allowed")},
        {AceColumn_Denied, tr("Denied")},
    });
    
    ace_view = new QTreeView(this);
    ace_view->setModel(ace_model);
    ace_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ace_view->sortByColumn(0, Qt::AscendingOrder);
    ace_view->setSortingEnabled(true);
    ace_view->setColumnWidth(AceColumn_Name, 400);

    selected_trustee_label = new QLabel();

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(trustee_view);
    layout->addWidget(selected_trustee_label);
    layout->addWidget(ace_view);

    // TODO: maybe use current, not selected? i think selection
    // may be empty if you press escape or something
    connect(
        trustee_view->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &SecurityTab::load_trustee_acl);
    connect(
        ace_model, &QStandardItemModel::itemChanged,
        this, &SecurityTab::on_item_changed);
}

void SecurityTab::load(AdInterface &ad, const AdObject &object) {
    trustee_model->removeRows(0, trustee_model->rowCount());

    const QByteArray descriptor_bytes = object.get_value(ATTRIBUTE_SECURITY_DESCRIPTOR);
    sd.load(descriptor_bytes);

    const QList<QString> trustee_order = sd.get_trustee_order();
    for (const QString &trustee_string : trustee_order) {
        auto item = new QStandardItem();
        
        const QString name = ad.get_trustee_name(trustee_string);
        item->setText(name);

        item->setData(trustee_string, TrusteeItemRole_Sid);

        trustee_model->appendRow(item);
    }

    trustee_model->sort(0, Qt::AscendingOrder);

    // Select first index
    // NOTE: load_trustee_acl() is called because setCurrentIndex
    // emits "current change" signal
    trustee_view->selectionModel()->setCurrentIndex(trustee_model->index(0, 0), QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);

    PropertiesTab::load(ad, object);
}

void SecurityTab::load_trustee_acl() {
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    const QList<QModelIndex> selected_list = trustee_view->selectionModel()->selectedRows();
    if (selected_list.isEmpty()) {
        return;
    }

    const QModelIndex selected_index = selected_list[0];
    QStandardItem *selected_item = trustee_model->itemFromIndex(selected_index);

    const QString label_text =
    [&]() {
        const QString selected_name = selected_item->data(Qt::DisplayRole).toString();
        const QString text = QString("Permissions for %1").arg(selected_name);

        return text;
    }();

    const QString trustee = selected_item->data(TrusteeItemRole_Sid).toString();

    selected_trustee_label->setText(label_text);

    ace_model->removeRows(0, ace_model->rowCount());

    const QHash<AcePermission, PermissionState> permission_state_map =
    [&]() {
        QHash<AcePermission, PermissionState> out;

        const QList<security_ace *> ace_list = sd.get_ace_list(trustee);

        for (security_ace *ace : ace_list) {
            const uint32_t ace_mask = ace->access_mask;

            for (int permission_i = 0; permission_i < AcePermission_COUNT; permission_i++) {
                const AcePermission permission = (AcePermission) permission_i;

                if (!ace_permission_to_mask_map.contains(permission)) {
                    continue;
                }

                const uint32_t permission_mask = ace_permission_to_mask_map[permission];

                const bool mask_match = ((ace_mask & permission_mask) == permission_mask);
                if (!mask_match) {
                    continue;
                }

                const bool object_match =
                [&]() {
                    const bool object_present = ((ace->object.object.flags & SEC_ACE_OBJECT_TYPE_PRESENT) != 0);
                    if (!object_present) {
                        return false;
                    }

                    const QString rights_guid =
                    [&]() {
                        const QString right_cn = ace_permission_to_type_map[permission];
                        const QString guid_out =  g_adconfig->get_right_guid(right_cn);

                        return guid_out;
                    }();

                    const QString ace_type_guid =
                    [&]() {
                        const GUID type = ace->object.object.type.type;
                        const QByteArray type_bytes = QByteArray((char *) &type, sizeof(GUID));

                        return attribute_display_value(ATTRIBUTE_OBJECT_GUID, type_bytes, g_adconfig);
                    }();

                    return (rights_guid.toLower() == ace_type_guid.toLower());
                }();

                switch (ace->type) {
                    case SEC_ACE_TYPE_ACCESS_ALLOWED: {
                        out[permission] = PermissionState_Allowed;
                        break;
                    }
                    case SEC_ACE_TYPE_ACCESS_DENIED: {
                        out[permission] = PermissionState_Denied;
                        break;
                    }
                    case SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT: {
                        if (object_match) {
                            out[permission] = PermissionState_Allowed;
                        }
                        break;
                    }
                    case SEC_ACE_TYPE_ACCESS_DENIED_OBJECT: {
                        if (object_match) {
                            out[permission] = PermissionState_Denied;
                        }
                        break;
                    }
                    default: break;
                }
            }
        }

        return out;
    }();
    
    for (int permission_i = 0; permission_i < AcePermission_COUNT; permission_i++) {
        const AcePermission permission = (AcePermission) permission_i;

        const QList<QStandardItem *> row = make_item_row(AceColumn_COUNT);

        const QString mask_string = ace_permission_to_name_map[permission];
        row[AceColumn_Name]->setText(mask_string);

        row[AceColumn_Allowed]->setCheckable(true);
        row[AceColumn_Denied]->setCheckable(true);

        const PermissionState permission_state = permission_state_map[permission];
        switch (permission_state) {
            case PermissionState_None: break;
            case PermissionState_Allowed: {
                row[AceColumn_Allowed]->setCheckState(Qt::Checked);
                break;
            }
            case PermissionState_Denied: {
                row[AceColumn_Denied]->setCheckState(Qt::Checked);
                break;
            }
        }

        row[0]->setData(permission, AcePermissionItemRole_Permission);

        ace_model->appendRow(row);
    }

    sd.print_acl(trustee);
}

// Permission states are interdependent. When the state of
// some permission changes, we sometimes also need to change
// the state of other permissions.
void SecurityTab::on_item_changed(QStandardItem *item) {
    // NOTE: First kind of dependence is opposite states for
    // same permission. Allowed/Denied are exclusive to each
    // other, both of them can't be checked at the same
    // time. So if one of them becomes checked, the opposite
    // one becomes unchecked.

    // NOTE: Second kind of dependence is parent/child.
    // There are 3 main "parent" checks: full control, read,
    // write. All other ones depend on some of these 3. When
    // a parent becomes checked in allowed or denied column,
    // it's children will also become checked in the same
    // column. When any of the children become unchecked in
    // some column, their parent(s) also become unchecked in
    // the same column. Parent/child relationship is based
    // on permission masks. If a permission mask contains
    // another mask, then these two permissions are
    // parent/child to each other.

    // NOTE: each time setCheckState() is called here,
    // itemChanged() signal is emitted by the model which
    // calls this f-n (item_changed()) again, since it is
    // connected as a slot. This recursion is intended
    // behavior and does useful work because it propagates
    // some state changes automatically. For example, if
    // "Read" becomes allowed, permissions that are children
    // of "Read" also become allowed. If some of these
    // children were previously denied, then denied is
    // automatically unchecked in the recursion caused by
    // setCheckState().

    auto permission_is_parent =
    [&](const int row_parent, const int row_child) {
        auto get_mask =
        [&](const int row) {
            QStandardItem *row_item = ace_model->item(row, 0);
            const AcePermission permission = (AcePermission) row_item->data(AcePermissionItemRole_Permission).toInt();
            const uint32_t mask = ace_permission_to_mask_map[permission];

            return mask;
        };

        // Mask "containing" another mask, means that the parent
        // mask contains all of the bits of the child mask and
        // that they are not equal to each other.
        const uint32_t parent_mask = get_mask(row_parent);
        const uint32_t child_mask = get_mask(row_child);
        const bool parent_contains_child = ((parent_mask & child_mask) == child_mask);
        const bool parent_is_not_child = (parent_mask != child_mask);
        const bool out = (parent_contains_child && parent_is_not_child);

        return out;
    };

    const AceColumn column = (AceColumn) item->column();

    const bool incorrect_column = (column != AceColumn_Allowed && column != AceColumn_Denied);
    if (incorrect_column) {
        return;
    }

    const bool checked = (item->checkState() == Qt::Checked);
    if (checked) {
        // Uncheck opposite item. For example: if allowed is
        // currently checked and you check denied, then
        // allowed should become unchecked.
        const AceColumn opposite_column =
        [&]() {
            if (column == AceColumn_Allowed) {
                return AceColumn_Denied;
            } else {
                return AceColumn_Allowed;
            }
        }();

        QStandardItem *opposite_item = ace_model->item(item->row(), opposite_column);
        opposite_item->setCheckState(Qt::Unchecked);

        // Check permissions which are children of this
        // permission. For example: if "Read" becomes
        // checked, then all permissions that are for
        // reading some property should become checked as
        // well.
        for (int row = 0; row < ace_model->rowCount(); row++) {
            const bool this_is_parent = permission_is_parent(item->row(), row);

            if (this_is_parent) {
                QStandardItem *other_item_check = ace_model->item(row, column);
                other_item_check->setCheckState(Qt::Checked);
            }
        }
    } else {
        // Uncheck items which this item depends on. For
        // example: if "Read email" becomes unchecked, then
        // "Read" should become unchecked as well.
        for (int row = 0; row < ace_model->rowCount(); row++) {
            const bool this_is_child = permission_is_parent(row, item->row());

            if (this_is_child) {
                QStandardItem *other_item_check = ace_model->item(row, column);
                other_item_check->setCheckState(Qt::Unchecked);
            }
        }
    }

    emit edited();
}
