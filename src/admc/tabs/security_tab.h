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

#include "adldap.h"

class QTreeView;
class QStandardItemModel;
class QStandardItem;
class QLabel;

class SecurityTab final : public PropertiesTab {
Q_OBJECT

public:
    SecurityTab();
    void load(AdInterface &ad, const AdObject &object) override;

private slots:
    void load_trustee_acl();
    void on_item_changed(QStandardItem *item);

private:
    QTreeView *trustee_view;
    QStandardItemModel *trustee_model;
    QTreeView *ace_view;
    QStandardItemModel *ace_model;
    QLabel *selected_trustee_label;
    SecurityDescriptor sd;
};

#endif /* SECURITY_TAB_H */
