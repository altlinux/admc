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

class QTreeView;
class QStandardItemModel;
class QLabel;

class SecurityTab final : public PropertiesTab {
Q_OBJECT

public:
    SecurityTab();
    void load(AdInterface &ad, const AdObject &object) override;

private slots:
    void on_selected_trustee_changed();

private:
    QTreeView *view;
    QStandardItemModel *model;
    QLabel *selected_trustee_label;
};

#endif /* SECURITY_TAB_H */
