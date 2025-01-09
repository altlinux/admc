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

#ifndef COMMON_PERMISSIONS_WIDGET_H
#define COMMON_PERMISSIONS_WIDGET_H

#include "permissions_widget.h"

class CommonPermissionsWidget final : public PermissionsWidget {
    Q_OBJECT

public:
    explicit CommonPermissionsWidget(QWidget *parent = nullptr);
    ~CommonPermissionsWidget();

    virtual void init(const QStringList &target_classes,
                      security_descriptor *sd_arg) override;
    virtual void update_permissions() override;

private:
    virtual QList<QStandardItem*> create_item_row(const SecurityRight &right) override;
    virtual bool there_are_rights_for_class(const QString &obj_class) override;
    virtual bool right_applies_to_class(const SecurityRight &right, const QString &obj_class) override;
    void update_object_typed_permission(const QModelIndex &main_index);

    // Contains access masks and their indexes for rights that can contain object type in their
    // ACEs. It's used to set superior rights when all its subordinates are set.
    // For example, "Read all properties" right is subordinate for all "Write property"
    // permission. This way is used because ACE matching isn't appliable in this case.
    QHash<uint32_t, QPersistentModelIndex> object_typed_superior_indexes;

};

#endif // COMMON_PERMISSIONS_WIDGET_H
