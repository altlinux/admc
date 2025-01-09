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

#ifndef GROUP_POLICY_TAB_H
#define GROUP_POLICY_TAB_H

#include "attribute_edits/attribute_edit.h"
#include <QWidget>
#include <QModelIndex>

#include "gplink.h"

/**
 * Tab for displaying, modifying group policy related
 * attributes of an object(not a gpo!), such as gplink and
 * gpoptions.
 */

class ConsoleWidget;
class InheritedPoliciesWidget;
class QCheckBox;

namespace Ui {
class GroupPolicyTab;
}

class GroupPolicyTab final : public QWidget {
    Q_OBJECT

public:
    Ui::GroupPolicyTab *ui;

    GroupPolicyTab(QList<AttributeEdit *> *edit_list, ConsoleWidget *console_widget, const QString &ou_dn, QWidget *parent);
    ~GroupPolicyTab();

private:
    ConsoleWidget *console;
    QModelIndex target_ou_index;
    InheritedPoliciesWidget *inheritance_widget;
    QCheckBox *gpo_options_check;
};

#endif /* GROUP_POLICY_TAB_H */
