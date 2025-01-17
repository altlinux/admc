/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2022 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
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

#ifndef POLICY_OU_IMPL_H
#define POLICY_OU_IMPL_H

/**
 * Impl for OU's in policy tree. Different from OU's in
 * object tree because in policy tree OU's have as children
 * only other OU's and GPO's that are linked to them. OU's
 * in policy tree also have different actions. Domain object
 * in policy tree also uses this impl even though it's not
 * an OU.
 */

#include "ad_interface.h"
#include "console_impls/my_console_role.h"
#include "console_widget/console_impl.h"

enum PolicyOURole {
    PolicyOURole_DN = MyConsoleRole_LAST + 1,
    PolicyOURole_Inheritance_Block,

    PolicyOURole_LAST,
};

class AdInterface;
class AdObject;
class PolicyOUResultsWidget;
class Gplink;

class PolicyOUImpl final : public ConsoleImpl {
    Q_OBJECT

public:
    PolicyOUImpl(ConsoleWidget *console_arg);

    void selected_as_scope(const QModelIndex &index) override;

    void fetch(const QModelIndex &index) override;
    bool can_drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type) override;
    void drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type) override;
    void refresh(const QList<QModelIndex> &index_list) override;
    void activate(const QModelIndex &index) override;

    QList<QAction *> get_all_custom_actions() const override;
    QSet<QAction *> get_custom_actions(const QModelIndex &index, const bool single_selection) const override;
    QSet<StandardAction> get_standard_actions(const QModelIndex &index, const bool single_selection) const override;

    QList<QString> column_labels() const override;
    QList<int> default_columns() const override;

    void rename(const QList<QModelIndex> &index_list) override;
    void properties(const QList<QModelIndex> &index_list) override;
    void delete_action(const QList<QModelIndex> &index_list) override;

private:
    PolicyOUResultsWidget *policy_ou_results_widget;
    QAction *create_ou_action;
    QAction *create_and_link_gpo_action;
    QAction *link_gpo_action;
    QAction *find_gpo_action;
    mutable QAction *change_gp_options_action;

    void create_ou();
    void create_and_link_gpo();
    void link_gpo();
    void link_gpo_to_ou(const QModelIndex &ou_index, const QString &ou_dn, const QList<QString> &gpo_list);
    void find_gpo();
    void change_gp_options();
    void update_gp_options_check_state() const;
};

void policy_ou_impl_load_row(const QList<QStandardItem *> row, const AdObject &object);
void policy_ou_impl_add_objects_from_dns(ConsoleWidget *console, AdInterface &ad, const QList<QString> &dn_list, const QModelIndex &parent);
void policy_ou_impl_add_objects_to_console(ConsoleWidget *console, const QList<AdObject> &object_list, const QModelIndex &parent);
void policy_ou_impl_load_item_data(QStandardItem *item, const AdObject &object);

//Get policy index for a given parent that is not in All policies folder.
//It is applied to exclude policy items from All policies folder
//those can be found first
QModelIndex get_ou_child_policy_index(ConsoleWidget *console, const QModelIndex &ou_index, const QString &policy_dn);

//Searches OU's index with given dn under "Group policy objects" item
QModelIndex search_gpo_ou_index(ConsoleWidget *console, const QString &ou_dn);

void update_ou_gplink_data(const QString &gplink, const QModelIndex &ou_index);

#endif /* POLICY_OU_IMPL_H */
