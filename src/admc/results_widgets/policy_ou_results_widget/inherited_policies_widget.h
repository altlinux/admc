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

#ifndef INHERITED_POLICIES_WIDGET_H
#define INHERITED_POLICIES_WIDGET_H

#include <QWidget>
#include <QModelIndex>

namespace Ui {
class InheritedPoliciesWidget;
}

class QStandardItemModel;
class ConsoleWidget;
class QStandardItem;
class QStringList;

class InheritedPoliciesWidget final : public QWidget
{
    Q_OBJECT

public:

    enum InheritedPoliciesColumns {
        InheritedPoliciesColumns_Prority,
        InheritedPoliciesColumns_Name,
        InheritedPoliciesColumns_Location,
        InheritedPoliciesColumns_Status,

        InheritedPoliciesColumns_COUNT
    };

    enum RowRoles {
       RowRole_DN = Qt::UserRole + 1,
       RowRole_IsEnforced,
    };

    explicit InheritedPoliciesWidget(ConsoleWidget *console_arg, QWidget *parent = nullptr);
    ~InheritedPoliciesWidget();

    void update(const QModelIndex &index);
    void hide_not_enforced_inherited_links(bool hide);

private:
    QStandardItemModel *model;
    Ui::InheritedPoliciesWidget *ui;
    ConsoleWidget *console;
    QModelIndex selected_scope_index;

    void add_enabled_policy_items(const QModelIndex &index, bool inheritance_blocked = false);
    void remove_link_duplicates();
    void set_priority_to_items();
    void load_item(const QList<QStandardItem *> row, const QModelIndex &ou_index, const QString &policy_dn, bool is_enforced);
};

#endif // INHERITED_POLICIES_WIDGET_H
