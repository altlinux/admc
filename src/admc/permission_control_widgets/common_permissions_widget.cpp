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

#include "common_permissions_widget.h"

#include "adldap.h"
#include "globals.h"
#include "settings.h"
#include "utils.h"

#include "samba/ndr_security.h"

#include <QStandardItemModel>
#include <QStandardItem>
#include <QTreeView>
#include <QVBoxLayout>
#include <QDebug>

class CommonRightsSortModel final : public RightsSortModel {
public:
    using RightsSortModel::RightsSortModel;

    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override {
        SecurityRight left = source_left.data(RightsItemRole_SecurityRight).
                value<SecurityRight>();
        SecurityRight right = source_right.data(RightsItemRole_SecurityRight).
                value<SecurityRight>();
        const uint32_t access_mask_left = left.access_mask;
        const uint32_t access_mask_right = right.access_mask;

        // Generic among generic are in pre-defined order
        const int common_index_left = common_rights_list.indexOf(access_mask_left);
        const int common_index_right = common_rights_list.indexOf(access_mask_right);

        return (common_index_left < common_index_right);
    }
};

CommonPermissionsWidget::CommonPermissionsWidget(QWidget *parent) : PermissionsWidget(parent) {
    v_layout->addWidget(rights_view);

    rights_sort_model = new CommonRightsSortModel(this);
    rights_sort_model->setSourceModel(rights_model);

    rights_view->setModel(rights_sort_model);
    rights_view->setColumnWidth(PermissionColumn_Name, 400);

    settings_restore_header_state(SETTING_common_permissions_header_state, rights_view->header());
}

CommonPermissionsWidget::~CommonPermissionsWidget() {
    settings_save_header_state(SETTING_common_permissions_header_state, rights_view->header());
}

void CommonPermissionsWidget::init(const QStringList &target_classes, security_descriptor *sd_arg) {
    PermissionsWidget::init(target_classes, sd_arg);

    // ACEs with these masks and without object are superior for
    // ACEs with object. So if all subordinate permissions are set
    // then corresponding superior permission should be set.
    const QList<uint32_t> object_typed_masks {
        SEC_ADS_CREATE_CHILD,
        SEC_ADS_DELETE_CHILD,
        SEC_ADS_READ_PROP,
        SEC_ADS_WRITE_PROP,
        SEC_ADS_CONTROL_ACCESS
    };

    // Create items in rights model. These will not
    // change until target object changes. Only the
    // state of items changes during editing.
    const QList<SecurityRight> right_list = ad_security_get_common_rights();
    for (const SecurityRight &right : right_list) {
        const QList<QStandardItem *> row = create_item_row(right);
        rights_model->appendRow(row);

        if (object_typed_masks.contains(right.access_mask)) {
            object_typed_superior_indexes[right.access_mask] = row[0]->index();
        }
    }

    // NOTE: because rights model is dynamically filled
    // when trustee switches, we have to make rights
    // model read only again after it's reloaded
    if (read_only) {
        make_model_rights_read_only();
    }

    rights_sort_model->sort(0);
}

void CommonPermissionsWidget::update_permissions() {
    PermissionsWidget::update_permissions();

    for (uint32_t access_mask : object_typed_superior_indexes.keys()) {
        const QModelIndex main_index =  object_typed_superior_indexes[access_mask];
        const QModelIndex allow_sibling = main_index.sibling(main_index.row(), PermissionColumn_Allowed);
        const QModelIndex deny_sibling = main_index.sibling(main_index.row(), PermissionColumn_Denied);

        if (rights_model->itemFromIndex(allow_sibling)->checkState() != Qt::Checked &&
                rights_model->itemFromIndex(deny_sibling)->checkState() != Qt::Checked) {

            update_object_typed_permission(main_index);
        }
    }
}

QList<QStandardItem *> CommonPermissionsWidget::create_item_row(const SecurityRight &right) {
    auto row = PermissionsWidget::create_item_row(right);

    // TODO: for russian, probably do "read/write
    // property - [property name]" to avoid having
    // to do suffixes properties
    const QString right_name = ad_security_get_right_name(g_adconfig, right, language);
    row[PermissionColumn_Name]->setText(right_name);

    return row;
}

bool CommonPermissionsWidget::there_are_rights_for_class(const QString &obj_class) {
    Q_UNUSED(obj_class)
    return true;
}

bool CommonPermissionsWidget::right_applies_to_class(const SecurityRight &right, const QString &obj_class) {
    const QList<uint32_t> parent_related_access_masks = {
        SEC_ADS_DELETE_TREE,
        SEC_ADS_CREATE_CHILD,
        SEC_ADS_DELETE_CHILD
    };

    if (!parent_related_access_masks.contains(right.access_mask)) {
        return true;
    }

    // Ignore parent specific rights for classes with no possible inferiors
    const bool applies = !g_adconfig->get_possible_inferiors(obj_class).isEmpty();
    return applies;
}

void CommonPermissionsWidget::update_object_typed_permission(const QModelIndex &main_index) {
    const SecurityRight superior_right = main_index.data(RightsItemRole_SecurityRight).value<SecurityRight>();
    const QList<SecurityRight> subordinate_right_list = ad_security_get_subordinate_right_list(g_adconfig, superior_right, {appliable_class});

    bool all_allow_subordinates_set = true;
    bool all_deny_subordinates_set = true;
    for (const SecurityRight &right : subordinate_right_list) {
        if (!all_allow_subordinates_set && !all_deny_subordinates_set) {
            return;
        }

        const SecurityRightState state = security_descriptor_get_right_state(sd, trustee, right);
        for (int type_i = 0; type_i < SecurityRightStateType_COUNT; type_i++) {
            const SecurityRightStateType type = (SecurityRightStateType) type_i;

            const bool inherited_ace_state = state.get(SecurityRightStateInherited_Yes, type);
            if (inherited_ace_state) {
                type == SecurityRightStateType_Allow ? all_allow_subordinates_set = false :
                                                 all_deny_subordinates_set = false;
                continue;
            }

            const bool object_ace_state = state.get(SecurityRightStateInherited_No, type);
            if (!object_ace_state) {
                type == SecurityRightStateType_Allow ? all_allow_subordinates_set = false :
                                                 all_deny_subordinates_set = false;
            }
        }
    }

    if (all_allow_subordinates_set && all_deny_subordinates_set) {
        return;
    }

    security_descriptor_add_right(sd, g_adconfig, {appliable_class}, trustee, superior_right, all_allow_subordinates_set);

    const SecurityRightState state = security_descriptor_get_right_state(sd, trustee, superior_right);
    const SecurityRightStateType type = all_allow_subordinates_set ? SecurityRightStateType_Allow : SecurityRightStateType_Deny;
    const bool object_ace_state = state.get(SecurityRightStateInherited_No, type);
    if (object_ace_state) {
        const QModelIndex checkable_sibling_index = all_allow_subordinates_set ? main_index.sibling(main_index.row(), PermissionColumn_Allowed) :
                                                                                 main_index.sibling(main_index.row(), PermissionColumn_Denied);
        rights_model->itemFromIndex(checkable_sibling_index)->setCheckState(Qt::Checked);
    }
}

