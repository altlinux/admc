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

#ifndef SECURITY_TAB_H
#define SECURITY_TAB_H

#include "tabs/properties_tab.h"

#include "adldap.h"

class QTreeView;
class QStandardItemModel;
class QStandardItem;
class QLabel;

enum AceColumn {
    AceColumn_Name,
    AceColumn_Allowed,
    AceColumn_Denied,

    AceColumn_COUNT,
};

enum AcePermissionItemRole {
    AcePermissionItemRole_Permission = Qt::UserRole,
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

extern const QSet<AcePermission> all_permissions;
extern const QSet<AcePermission> access_permissions;
extern const QSet<AcePermission> read_prop_permissions;
extern const QSet<AcePermission> write_prop_permissions;

class SecurityTab final : public PropertiesTab {
Q_OBJECT

public:
    SecurityTab();
    void load(AdInterface &ad, const AdObject &object) override;
    bool apply(AdInterface &ad, const QString &target) override;

    QStandardItemModel *get_ace_model() const;
    void set_permission_state(const QSet<AcePermission> &permission_list, const AceColumn column, const Qt::CheckState state);
    QStandardItem *get_item(const AcePermission permission, const AceColumn column);

private slots:
    void load_trustee_acl();
    void on_item_changed(QStandardItem *item);

private:
    QTreeView *trustee_view;
    QStandardItemModel *trustee_model;
    QTreeView *ace_view;
    QStandardItemModel *ace_model;
    QLabel *selected_trustee_label;
    SecurityDescriptor sd;
    QHash<AcePermission, QHash<AceColumn, QStandardItem *>> permission_item_map;
    QHash<QString, QHash<AcePermission, PermissionState>> permission_state_map;
};

#endif /* SECURITY_TAB_H */
