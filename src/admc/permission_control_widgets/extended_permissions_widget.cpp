/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
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

#include "extended_permissions_widget.h"

#include "adldap.h"
#include "globals.h"
#include "settings.h"
#include "utils.h"

#include "samba/ndr_security.h"

#include <QStandardItemModel>
#include <QStandardItem>
#include <QTreeView>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>
#include <QDebug>


class ExtendedRightsSortModel final : public RightsSortModel {
public:
    using RightsSortModel::RightsSortModel;

    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override {
        SecurityRight left = source_left.data(RightsItemRole_SecurityRight).
                value<SecurityRight>();
        SecurityRight right = source_right.data(RightsItemRole_SecurityRight).
                value<SecurityRight>();

        const QString name_left = source_left.data(Qt::DisplayRole).toString();
        const QString name_right = source_right.data(Qt::DisplayRole).toString();
        const QString object_type_name_left = source_left.data(RightsItemRole_ObjectTypeName).toString();
        const QString object_type_name_right = source_right.data(RightsItemRole_ObjectTypeName).toString();
        const QByteArray object_type_left = left.object_type;
        const QByteArray object_type_right = right.object_type;
        const uint32_t access_mask_left = left.access_mask;
        const uint32_t access_mask_right = right.access_mask;
        const bool is_control_left = (access_mask_left == SEC_ADS_CONTROL_ACCESS);
        const bool is_control_right = (access_mask_right == SEC_ADS_CONTROL_ACCESS);
        const bool is_read_left = (access_mask_left == SEC_ADS_READ_PROP);
        const bool is_read_right = (access_mask_right == SEC_ADS_READ_PROP);

        // Control rights are before read/write rights
        if (is_control_left != is_control_right) {
            return is_control_left;
        }

        // Control rights are sorted by name
        if (is_control_left && is_control_right) {
            return name_left < name_right;
        }

        // Read/write rights are sorted by name
        if (object_type_left != object_type_right) {
            return object_type_name_left < object_type_name_right;
        }

        // Read rights are before write rights
        if (is_read_left != is_read_right) {
            return is_read_left;
        }

        return name_left < name_right;
    }
};

ExtendedPermissionsWidget::ExtendedPermissionsWidget(QWidget *parent) : PermissionsWidget(parent) {
    v_layout->addWidget(rights_view);

    rights_sort_model = new ExtendedRightsSortModel(this);
    rights_sort_model->setSourceModel(rights_model);

    rights_view->setModel(rights_sort_model);
    rights_view->setColumnWidth(PermissionColumn_Name, 400);

    settings_restore_header_state(SETTING_extended_permissions_header_state, rights_view->header());
}

ExtendedPermissionsWidget::~ExtendedPermissionsWidget() {
    settings_save_header_state(SETTING_extended_permissions_header_state, rights_view->header());
}

void ExtendedPermissionsWidget::init(const QStringList &target_classes, security_descriptor *sd_arg) {
    PermissionsWidget::init(target_classes, sd_arg);
    append_message_item();

    const QString target_class = target_class_list.last();
    const QStringList all_inferiors = g_adconfig->all_inferiors_list(target_class);
    const QStringList all_ext_right_classes = g_adconfig->all_extended_right_classes();
    // Get inferiors that have extended rights with intersecting two lists above (also add target class)
    QSet<QString> ext_rights_class_set = QSet<QString>(all_inferiors.begin(), all_inferiors.end()).intersect(
                QSet<QString>(all_ext_right_classes.begin(), all_ext_right_classes.end()));
    ext_rights_class_set.insert(target_class);

    const QList<SecurityRight> extended_right_list = ad_security_get_extended_rights_for_class(g_adconfig, ext_rights_class_set.values());
    for(const SecurityRight &right : extended_right_list) {
       QList<QStandardItem*> right_row = create_item_row(right);
       rights_model->appendRow(right_row);
    }

    rights_sort_model->sort(0);
}

QList<QStandardItem *> ExtendedPermissionsWidget::create_item_row(const SecurityRight &right) {
    auto row = PermissionsWidget::create_item_row(right);

    const QString name = ad_security_get_right_name(g_adconfig, right, language);
    const QString object_type_name = g_adconfig->get_right_name(right.object_type, language);

    row[PermissionColumn_Name]->setText(name);
    row[0]->setData(object_type_name, RightsItemRole_ObjectTypeName);

    return row;
}

bool ExtendedPermissionsWidget::there_are_rights_for_class(const QString &obj_class) {
    return g_adconfig->all_extended_right_classes().contains(obj_class);
}

bool ExtendedPermissionsWidget::right_applies_to_class(const SecurityRight &right, const QString &obj_class) {
    for (const SecurityRight &ext_right : ad_security_get_extended_rights_for_class(g_adconfig, {obj_class})) {
        if (ext_right.access_mask == right.access_mask && ext_right.object_type == right.object_type) {
            return true;
        }
    }

    return false;
}
