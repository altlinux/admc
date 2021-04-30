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
    AcePermission_ChangePassword,

    AcePermission_COUNT,
};

#define ENUM_TO_STRING(ENUM) {ENUM, #ENUM}

const QHash<AcePermission, QString> ace_permission_to_name_map = {
    ENUM_TO_STRING(AcePermission_FullControl),
    ENUM_TO_STRING(AcePermission_Read),
    ENUM_TO_STRING(AcePermission_Write),
    ENUM_TO_STRING(AcePermission_CreateChild),
    ENUM_TO_STRING(AcePermission_DeleteChild),
    ENUM_TO_STRING(AcePermission_ChangePassword),
};

const QHash<AcePermission, uint32_t> ace_permission_to_mask_map = {
    {AcePermission_FullControl, 0x000F01FF},
    {AcePermission_Read, SEC_ADS_GENERIC_READ},
    {AcePermission_Write, SEC_ADS_GENERIC_WRITE},
    {AcePermission_CreateChild, SEC_ADS_CREATE_CHILD},
    {AcePermission_DeleteChild, SEC_ADS_DELETE_CHILD},
    {AcePermission_ChangePassword, SEC_ADS_CONTROL_ACCESS},
};

const QHash<AcePermission, QString> ace_permission_to_type_map = {
    {AcePermission_ChangePassword, GUID_DRS_USER_CHANGE_PASSWORD},
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
}

void SecurityTab::load(AdInterface &ad, const AdObject &object) {
    trustee_model->removeRows(0, trustee_model->rowCount());

    const QByteArray descriptor_bytes = object.get_value(ATTRIBUTE_SECURITY_DESCRIPTOR);
    sd.load(descriptor_bytes);

    const QList<QString> trustee_list = sd.get_trustee_list();
    for (const QString &trustee : trustee_list) {
        auto item = new QStandardItem();
        
        const QString name = ad.get_trustee_name(trustee);
        item->setText(name);

        item->setData(trustee, TrusteeItemRole_Sid);
        
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
    [](security_ace *ace, const AcePermission permission) {
        const bool permission_has_type = ace_permission_to_type_map.contains(permission);

        if (permission_has_type) {
            // Check that ace type equals to permission
            // type, if it exists
            const bool type_matches =
            [&]() {
                const GUID type = ace->object.object.type.type;
                const QByteArray type_bytes = QByteArray((char *) &type, sizeof(GUID));
                const QString type_string = attribute_display_value(ATTRIBUTE_OBJECT_GUID, type_bytes, g_adconfig);

                return (type_string.toLower() == QString(GUID_DRS_USER_CHANGE_PASSWORD).toLower());
            }();

            if (!type_matches) {
                return PermissionState_None;
            }
        }

        const uint32_t permission_mask = ace_permission_to_mask_map[permission];
        const bool ace_ok = ((ace->access_mask & permission_mask) != 0);

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

        ace_model->appendRow(row);
    }
}
