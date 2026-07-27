/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2024-2026 BaseALT Ltd.
 * Copyright (C) 2024 Semyon Knyazev
 * Copyright (C) 2026 Artyom V. Poptsov
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

#ifndef DELEGATION_PERMISSIONS_WIDGET_H
#define DELEGATION_PERMISSIONS_WIDGET_H

#include "permissions_widget.h"

struct SecurityRight;
class QRadioButton;
class AdConfig;

class DelegationPermissionsWidget final : public PermissionsWidget {

    Q_OBJECT

    enum TaskDelegationColumn {
        TaskDelegationColumn_Name = PermissionColumn_Name,
        TaskDelegationColumn_Assigned = PermissionColumn_Allowed,
        TaskDelegationColumn_COUNT
    };

public:
    DelegationPermissionsWidget(QWidget *parent = nullptr);
    ~DelegationPermissionsWidget();

    virtual void init(const QStringList &target_classes,
                      security_descriptor *sd_arg) override;
    virtual void update_permissions(AppliedObjects applied_objs, const QString &appliable_child_class) override;
    virtual void update_permissions() override;

private:
    void append_common_tasks();

    QList<QStandardItem*> create_item_row(const QString &name, const QList<SecurityRight> rights, const QString &object_type_name);

    virtual bool there_are_rights_for_class(const QString &obj_class) override;
    virtual bool right_applies_to_class(const SecurityRight &right, const QString &obj_class) override;

    void update_task_check_state(QStandardItem *item);

    virtual void make_model_rights_read_only() override;

    QString target_class;
};

#endif // DELEGATION_PERMISSIONS_WIDGET_H
