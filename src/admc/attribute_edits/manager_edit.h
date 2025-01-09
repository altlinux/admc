/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#ifndef MANAGER_EDIT_H
#define MANAGER_EDIT_H

/**
 * Edit for editing manager. Displays current manager and
 * allows changing manager, opening manager properties and
 * clearing manager. Accepts manager attribute argument
 * because there are two manager attributes: "manager" and
 * "managedBy".
 */

#include "attribute_edits/attribute_edit.h"

class ManagerWidget;

class ManagerEdit final : public AttributeEdit {
    Q_OBJECT
public:
    ManagerEdit(ManagerWidget *widget_arg, const QString &manager_attribute_arg, QObject *parent);

    void load(AdInterface &ad, const AdObject &object) override;
    bool apply(AdInterface &ad, const QString &dn) const override;
    void set_enabled(const bool enabled) override;

    QString get_manager() const;

private:
    ManagerWidget *widget;
    QString manager_attribute;
};

#endif /* MANAGER_EDIT_H */
