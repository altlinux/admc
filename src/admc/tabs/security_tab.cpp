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
    {AcePermission_AllowedToAuthenticate, "ALlowed-To-Authenticate"},
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

    connect(
        trustee_view->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &SecurityTab::on_selected_trustee_changed);
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
    trustee_view->selectionModel()->setCurrentIndex(trustee_model->index(0, 0), QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);

    PropertiesTab::load(ad, object);
}

// TODO: maybe use current, not selected? i think selection
// may be empty if you press escape or something
void SecurityTab::on_selected_trustee_changed() {
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

    const QList<security_ace *> ace_list = sd.ace_map[trustee];

    auto get_permission_state =
    [&](security_ace *ace, const AcePermission permission) {
        const bool permission_has_type = ace_permission_to_type_map.contains(permission);

        if (permission_has_type) {
            // Check that ace type equals to permission
            // type, if it exists
            const bool type_matches =
            [&]() {
                const QString rights_guid =
                [&]() {
                    const QString filter =
                    [&]() {
                        const QString permission_right_cn = ace_permission_to_type_map[permission];
                        const QString cn_filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_CN, permission_right_cn);

                        return cn_filter;
                    }();

                    const QList<QString> attributes = {
                        ATTRIBUTE_RIGHTS_GUID,
                    };

                    const QString search_base = g_adconfig->extended_rights_dn();

                    const QHash<QString, AdObject> search_results = ad.search(filter, attributes, SearchScope_Children, search_base);

                    if (search_results.isEmpty()) {
                        return QString();
                    }

                    const AdObject object = search_results.values()[0];

                    return object.get_string(ATTRIBUTE_RIGHTS_GUID);
                }();

                const QString ace_type_guid =
                [&]() {
                    const GUID type = ace->object.object.type.type;
                    const QByteArray type_bytes = QByteArray((char *) &type, sizeof(GUID));

                    return attribute_display_value(ATTRIBUTE_OBJECT_GUID, type_bytes, g_adconfig);
                }();

                return (rights_guid.toLower() == ace_type_guid.toLower());
            }();

            if (!type_matches) {
                return PermissionState_None;
            }
        }

        const uint32_t permission_mask = ace_permission_to_mask_map[permission];
        const bool ace_ok = ((ace->access_mask & permission_mask) == permission_mask);

        if (ace_ok) {
            if (ace->type == SEC_ACE_TYPE_ACCESS_ALLOWED || ace->type == SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT) {
                
                return PermissionState_Allowed;
            } else if (ace->type == SEC_ACE_TYPE_ACCESS_DENIED || ace->type == SEC_ACE_TYPE_ACCESS_DENIED_OBJECT) {
                return PermissionState_Denied;
            }
        }

        return PermissionState_None;
    };

    for (int permission_i = 0; permission_i < AcePermission_COUNT; permission_i++) {
        const AcePermission permission = (AcePermission) permission_i;

        const QList<QStandardItem *> row = make_item_row(AceColumn_COUNT);

        const QString mask_string = ace_permission_to_name_map[permission];
        row[AceColumn_Name]->setText(mask_string);

        row[AceColumn_Allowed]->setCheckable(true);
        row[AceColumn_Denied]->setCheckable(true);

        for (security_ace *ace : ace_list) {
            const PermissionState permission_state = get_permission_state(ace, permission);

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
        }

        row[0]->setData(permission, AcePermissionItemRole_Permission);

        ace_model->appendRow(row);
    }
}

void SecurityTab::on_item_changed(QStandardItem *item) {
    const AceColumn changed_column = (AceColumn) item->column();

    const bool incorrect_column = (changed_column != AceColumn_Allowed && changed_column != AceColumn_Denied);
    if (incorrect_column) {
        return;
    }

    const QString trustee =
    [&]() {
        const QList<QModelIndex> selected_list = trustee_view->selectionModel()->selectedRows();
        if (selected_list.isEmpty()) {
            return QString();
        }

        const QModelIndex selected_index = selected_list[0];
        QStandardItem *selected_item = trustee_model->itemFromIndex(selected_index);
        const QString out = selected_item->data(TrusteeItemRole_Sid).toString();

        return out;
    }();

    auto get_checked =
    [&](const AceColumn column) {
        const QModelIndex index = item->index();
        const QModelIndex column_index = index.siblingAtColumn(column);
        QStandardItem *column_item = ace_model->itemFromIndex(column_index);

        const Qt::CheckState check_state = column_item->checkState();
        const bool checked = (check_state == Qt::Checked);

        return checked;
    };

    const uint32_t permission_mask =
    [&]() {
        const QModelIndex index = item->index();
        const QModelIndex main_index = index.siblingAtColumn(0);
        const AcePermission permission = (AcePermission) main_index.data(AcePermissionItemRole_Permission).toInt();
        const uint32_t mask = ace_permission_to_mask_map[permission];

        return mask;
    }();

    const bool allowed_checked = get_checked(AceColumn_Allowed);
    const bool denied_checked = get_checked(AceColumn_Denied);
    const bool allowed_changed = (changed_column == AceColumn_Allowed);
    const bool denied_changed = (changed_column == AceColumn_Denied);
    qDebug() << "allowed_checked = " << allowed_checked;
    qDebug() << "denied_checked = " << denied_checked;

    sd.modify_sd(trustee, allowed_checked, denied_checked, allowed_changed, denied_changed, permission_mask);
    
    emit edited();
}
