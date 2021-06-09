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

#include "ad_defines.h"

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

extern const QList<AcePermission> all_permissions_list;
extern const QSet<AcePermission> all_permissions;
extern const QSet<AcePermission> access_permissions;
extern const QSet<AcePermission> read_prop_permissions;
extern const QSet<AcePermission> write_prop_permissions;
extern const QHash<AcePermission, QString> ace_permission_to_name_map;

class SecurityTab final : public PropertiesTab {
Q_OBJECT

public:
    SecurityTab();
    void load(AdInterface &ad, const AdObject &object) override;
    bool apply(AdInterface &ad, const QString &target) override;

    // NOTE: f-ns for testings
    QStandardItem *get_item(const AcePermission permission, const AceColumn column);
    bool set_trustee(const QString &trustee_name);

private slots:
    void load_trustee_acl();
    void on_item_changed(QStandardItem *item);

private:
    QTreeView *trustee_view;
    QStandardItemModel *trustee_model;
    QTreeView *ace_view;
    QStandardItemModel *ace_model;
    QLabel *trustee_label;
    QHash<AcePermission, QHash<AceColumn, QStandardItem *>> permission_item_map;
    QHash<QByteArray, QHash<AcePermission, PermissionState>> original_permission_state_map;
    QHash<QByteArray, QHash<AcePermission, PermissionState>> permission_state_map;
    bool ignore_item_changed_signal;

    void add_trustee();
    void remove_trustee();
    void add_trustee_item(const QByteArray &sid, AdInterface &ad);
    void apply_current_state_to_items();
};

#endif /* SECURITY_TAB_H */
