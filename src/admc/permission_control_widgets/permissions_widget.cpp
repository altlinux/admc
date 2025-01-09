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

#include "permissions_widget.h"

#include "utils.h"
#include "settings.h"
#include "samba/ndr_security.h"
#include "adldap.h"
#include "globals.h"

#include <QTreeView>
#include <QStandardItemModel>
#include <QLayout>
#include <QDebug>


PermissionsWidget::PermissionsWidget(QWidget *parent) :
    QWidget(parent), read_only(false) {

    rights_model = new QStandardItemModel(0, PermissionColumn_COUNT, this);
    set_horizontal_header_labels_from_map(rights_model,
        {
            {PermissionColumn_Name, tr("Name")},
            {PermissionColumn_Allowed, tr("Allowed")},
            {PermissionColumn_Denied, tr("Denied")},
        });;


    rights_view = new QTreeView(this);
    rights_view->setModel(rights_model);

    v_layout = new QVBoxLayout;
    v_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(v_layout);

    connect(
        rights_model, &QStandardItemModel::itemChanged,
        this, &PermissionsWidget::on_item_changed);

    const QLocale saved_locale = settings_get_variant(SETTING_locale).toLocale();
    language = saved_locale.language();
}

void PermissionsWidget::init(const QStringList &target_classes, security_descriptor *sd_arg) {
    sd = sd_arg;
    target_class_list = target_classes;
    rights_model->removeRows(0, rights_model->rowCount());
}

void PermissionsWidget::set_read_only() {
    read_only = true;
    make_model_rights_read_only();
}

void PermissionsWidget::set_current_trustee(const QByteArray &current_trustee) {
    trustee = current_trustee;
    update_permissions();
}

void PermissionsWidget::update_permissions(AppliedObjects applied_objs, const QString &appliable_child_class) {
    applied_objects = applied_objs;
    ignore_item_changed_signal = true;

    appliable_class = appliable_child_class.isEmpty() ? target_class_list.last() :
                                                        appliable_child_class;
    const bool there_are_rights = there_are_rights_for_class(appliable_class);
    show_no_rights_message(!there_are_rights);

    for (int row = 0; row < rights_model->rowCount(); row++) {
        const QModelIndex index = rights_model->index(row, 0);
        SecurityRight right = rights_model->data(index, RightsItemRole_SecurityRight).value<SecurityRight>();
        //const bool right_is_inherited = bitmask_is_set(right.access_mask, SEC_ACE_FLAG_INHERITED_ACE);
        if (item_is_message(index)/* || right_is_inherited*/) {
            continue;
        }

        if (!there_are_rights || !right_applies_to_class(right, appliable_class)) {
            rights_model->setData(index, true, RightsItemRole_HiddenItem);
            continue;
        }

        switch (applied_objs) {
        case AppliedObjects_ThisObject:
            right.flags = 0;
            right.inherited_object_type = QByteArray();
            break;

        case AppliedObjects_ThisAndChildObjects:
            right.flags = SEC_ACE_FLAG_CONTAINER_INHERIT;
            right.inherited_object_type = QByteArray();
            break;

        case AppliedObjects_AllChildObjects:
            right.flags = SEC_ACE_FLAG_CONTAINER_INHERIT | SEC_ACE_FLAG_INHERIT_ONLY;
            right.inherited_object_type = QByteArray();
            break;

        case AppliedObjects_ChildObjectClass:
            right.flags = SEC_ACE_FLAG_CONTAINER_INHERIT | SEC_ACE_FLAG_INHERIT_ONLY;
            right.inherited_object_type = g_adconfig->guid_from_class(appliable_child_class);
            break;

        default:
            ignore_item_changed_signal = false;
            return;
        }

        QVariant right_data;
        right_data.setValue(right);
        rights_model->setData(index, right_data, RightsItemRole_SecurityRight);
        rights_model->setData(index, false, RightsItemRole_HiddenItem);
    }

    rights_sort_model->hide_ignored_items();
    ignore_item_changed_signal = false;

    PermissionsWidget::update_permissions();
}

void PermissionsWidget::on_item_changed(QStandardItem *item) {
    if (ignore_item_changed_signal) {
        return;
    }

    const int column = item->column();
    const bool incorrect_column = (column != (int)PermissionColumn_Allowed && column != (int)PermissionColumn_Denied);
    if (incorrect_column) {
        return;
    }

    const QModelIndex main_item_index = item->index().siblingAtColumn(0);
    QStandardItem *main_item = rights_model->itemFromIndex(main_item_index);

    const bool checked = (item->checkState() == Qt::Checked);
    const bool allow = (column == (int)PermissionColumn_Allowed);


    const bool has_multiple_rights = !main_item->data(RightsItemRole_SecurityRightList).isNull();
    QList<SecurityRight> rights;
    if (has_multiple_rights) {
        rights = main_item->data(RightsItemRole_SecurityRightList).value<QList<SecurityRight>>();
    }
    else {
        SecurityRight right = main_item->data(RightsItemRole_SecurityRight).value<SecurityRight>();
        rights.append(right);
    }

    for (const SecurityRight &right : rights) {
        if (checked) {
            security_descriptor_add_right(sd, g_adconfig, {appliable_class}, trustee, right, allow);
        } else {
            security_descriptor_remove_right(sd, g_adconfig, {appliable_class}, trustee, right, allow);
        }
    }

    update_permissions();

    emit edited();
}

void PermissionsWidget::update_permissions() {
    // NOTE: this flag is turned on so that
    // on_item_changed() slot doesn't react to us
    // changing state of items
    ignore_item_changed_signal = true;

    for (int row = 0; row < rights_model->rowCount(); row++) {
        const QModelIndex index = rights_model->index(row, 0);
        if (!index.isValid() || item_is_message(index)) {
            continue;
        }

        QStandardItem *main_item = rights_model->itemFromIndex(index);

        SecurityRight right = main_item->data(RightsItemRole_SecurityRight).value<SecurityRight>();
        update_row_check_state(row, right);
    }

    // NOTE: need to make read only again because
    // during load, items are enabled/disabled based on
    // their inheritance state
    if (read_only) {
        make_model_rights_read_only();
    }

    ignore_item_changed_signal = false;
}

bool PermissionsWidget::there_are_selected_permissions() const {
    for (int row = 0; row < rights_model->rowCount(); ++row) {
        const QList<PermissionColumn> columns = {PermissionColumn_Allowed, PermissionColumn_Denied};
        for (PermissionColumn col : columns) {
            const QModelIndex index = rights_model->index(row, col);
            if (!index.isValid()) {
                continue;
            }

            QStandardItem *item = rights_model->itemFromIndex(index);
            const bool checked = item->checkState() == Qt::Checked && item->isEnabled();
            if (checked) {
                return true;
            }
        }
    }

    return false;
}

void PermissionsWidget::make_model_rights_read_only() {
    // NOTE: important to ignore this signal because
    // it's slot reloads the rights model
    ignore_item_changed_signal = true;

    for (int row = 0; row < rights_model->rowCount(); row++) {
        const QList<int> col_list = {
            PermissionColumn_Allowed,
            PermissionColumn_Denied,
        };

        for (const int col : col_list) {
            QStandardItem *item = rights_model->item(row, col);
            item->setEnabled(false);
        }
    }

    ignore_item_changed_signal = false;
}

void PermissionsWidget::show_no_rights_message(bool show) {
    if (!message_index.isValid()) {
        return;
    }

    rights_model->setData(message_index, !show, RightsItemRole_HiddenItem);
}

bool PermissionsWidget::item_is_message(const QModelIndex &index) const {
    return index == message_index;
}

void PermissionsWidget::append_message_item() {
    if (message_index.isValid()) {
        return;
    }

    QList<QStandardItem *> message_row = make_item_row(PermissionColumn_COUNT);
    message_row[0]->setText(tr("There are no rights for this class of objects"));
    message_row[0]->setData(true, RightsItemRole_HiddenItem);
    for (QStandardItem *item : message_row) {
        item->setEditable(false);
    }
    rights_model->appendRow(message_row);
    message_index = rights_model->indexFromItem(message_row[0]);
}

QList<QStandardItem *> PermissionsWidget::create_item_row(const SecurityRight &right)  {
    const QList<QStandardItem *> row = make_item_row(PermissionColumn_COUNT);
    row[PermissionColumn_Name]->setEditable(false);
    row[PermissionColumn_Allowed]->setCheckable(true);
    row[PermissionColumn_Allowed]->setEditable(false);
    row[PermissionColumn_Denied]->setCheckable(true);
    row[PermissionColumn_Denied]->setEditable(false);

    QVariant right_data;
    right_data.setValue(right);
    row[0]->setData(right_data, RightsItemRole_SecurityRight);
    row[0]->setEditable(false);

    return row;
}

void PermissionsWidget::update_row_check_state(int row, const SecurityRight &right) {
    const QHash<SecurityRightStateType, QModelIndex> checkable_index_map = {
        {SecurityRightStateType_Allow, rights_model->index(row, PermissionColumn_Allowed)},
        {SecurityRightStateType_Deny, rights_model->index(row, PermissionColumn_Denied)},
    };

    const SecurityRightState state = security_descriptor_get_right_state(sd, trustee, right);
    for (int type_i = 0; type_i < SecurityRightStateType_COUNT; type_i++) {
        const SecurityRightStateType type = (SecurityRightStateType) type_i;

        QStandardItem *item = rights_model->itemFromIndex(checkable_index_map[type]);

        const bool object_ace_state = state.get(SecurityRightStateInherited_No, type);
        const bool inherited_ace_state = state.get(SecurityRightStateInherited_Yes, type);

        // Checkboxes become disabled if they
        // contain only inherited state. Note that
        // if there's both inherited and object
        // state for same right, checkbox is
        // enabled so that user can remove object
        // state.
        const bool disabled = (inherited_ace_state && !object_ace_state);
        item->setEnabled(!disabled);

        Qt::CheckState check_state = object_ace_state || inherited_ace_state ? Qt::Checked : Qt::Unchecked;
        item->setCheckState(check_state);
    }
}

void RightsSortModel::hide_ignored_items() {
    invalidateFilter();
}

bool RightsSortModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    const QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    return !index.data(RightsItemRole_HiddenItem).toBool();
}
