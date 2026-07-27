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

#ifndef CREATIONDELETIONPERMISSIONSWIDGET_H
#define CREATIONDELETIONPERMISSIONSWIDGET_H

#include "permissions_widget.h"

class CreationDeletionPermissionsWidget final : public PermissionsWidget {
    Q_OBJECT

public:
    explicit CreationDeletionPermissionsWidget(QWidget *parent = nullptr);
    ~CreationDeletionPermissionsWidget();

    virtual void init(const QStringList &target_classes, security_descriptor *sd_arg) override;

private:
    virtual QList<QStandardItem*> create_item_row(const SecurityRight &right) override;
    virtual bool right_applies_to_class(const SecurityRight &right, const QString &obj_class) override;
    virtual bool there_are_rights_for_class(const QString &obj_class) override;
};

#endif // CREATIONDELETIONPERMISSIONSWIDGET_H
