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

#ifndef SECURITY_TAB_H
#define SECURITY_TAB_H

#include "tabs/properties_tab.h"

#include "ad_defines.h"

#include <QDialog>

class QStandardItemModel;
class QStandardItem;

enum AceColumn {
    AceColumn_Name,
    AceColumn_Allowed,
    AceColumn_Denied,

    AceColumn_COUNT,
};

namespace Ui {
class SecurityTab;
}

class SecurityTab final : public PropertiesTab {
    Q_OBJECT

public:
    Ui::SecurityTab *ui;

    SecurityTab();
    ~SecurityTab();

    static QHash<AcePermission, QString> ace_permission_to_name_map();

    void load(AdInterface &ad, const AdObject &object) override;
    bool verify(AdInterface &ad, const QString &target) const override;
    bool apply(AdInterface &ad, const QString &target) override;

    // NOTE: f-ns for testings
    QStandardItem *get_item(const AcePermission permission, const AceColumn column);
    bool set_trustee(const QString &trustee_name);

private slots:
    void load_trustee_acl();

private:
    QStandardItemModel *trustee_model;
    QStandardItemModel *ace_model;
    QHash<AcePermission, QHash<AceColumn, QStandardItem *>> permission_item_map;
    QHash<QByteArray, QHash<AcePermission, PermissionState>> original_permission_state_map;
    QHash<QByteArray, QHash<AcePermission, PermissionState>> permission_state_map;
    bool ignore_item_changed_signal;
    bool is_policy;

    void on_item_changed(QStandardItem *item);
    void on_add_trustee_button();
    void on_remove_trustee_button();
    void apply_current_state_to_items();
    void add_trustees(const QList<QByteArray> &sid_list, AdInterface &ad);
    void on_add_well_known_trustee();
};

#endif /* SECURITY_TAB_H */
