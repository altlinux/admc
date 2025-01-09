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

#ifndef POLICY_IMPL_H
#define POLICY_IMPL_H

/**
 * Impl for policy objects which represent GPC + GPT.
 * Displayed as children of OU's which they are linked to
 * and in "All policies" folder.
 */

#include "console_widget/console_impl.h"
#include "console_widget/console_widget.h"

#include <QProcess>
#include "gplink.h"

class QStandardItem;
class AdObject;
class AdInterface;
class ConsoleActions;
class PolicyResultsWidget;
template <typename T>
class QList;

enum PolicyRole {
    PolicyRole_DN = ConsoleRole_LAST + 1,
    PolicyRole_GPO_Status,
    PolicyRole_LAST,
};

class PolicyImpl final : public ConsoleImpl {
    Q_OBJECT

public:
    PolicyImpl(ConsoleWidget *console_arg);

    bool can_drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type) override;
    void drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type) override;

    void selected_as_scope(const QModelIndex &index) override;

    QList<QAction *> get_all_custom_actions() const override;
    QSet<QAction *> get_custom_actions(const QModelIndex &index, const bool single_selection) const override;
    QSet<StandardAction> get_standard_actions(const QModelIndex &index, const bool single_selection) const override;

    void rename(const QList<QModelIndex> &index_list) override;
    void delete_action(const QList<QModelIndex> &index_list) override;
    void refresh(const QList<QModelIndex> &index_list) override;
    void properties(const QList<QModelIndex> &index_list) override;

private slots:
    void on_add_link();
    void on_edit();
    void on_ou_gplink_changed(const QString &ou_dn, const Gplink &gplink, const QString &policy_dn, GplinkOption option);

private:
    PolicyResultsWidget *policy_results;
    QAction *add_link_action;
    QAction *edit_action;
    mutable QAction *enforce_action;
    mutable QAction *disable_action;

    void on_gpui_error(QProcess::ProcessError error);
    void set_policy_item_icon(const QModelIndex &policy_index, bool is_checked, GplinkOption option);
    void on_change_gplink_option_action(QAction *action);
    void update_gplink_option_check_actions() const;
};

void console_policy_load(const QList<QStandardItem *> &row, const AdObject &object);
void console_policy_load_item(QStandardItem *item, const AdObject &object);
QList<QString> console_policy_search_attributes();
void console_policy_edit(ConsoleWidget *console, const int item_type, const int dn_role);
void console_policy_edit(const QString &policy_dn, ConsoleWidget *console);
void console_policy_rename(const QList<ConsoleWidget *> &console_list, PolicyResultsWidget *policy_results, const int item_type, const int dn_role);
void console_policy_add_link(const QList<ConsoleWidget *> &console_list, PolicyResultsWidget *policy_results, const int item_type, const int dn_role);
void console_policy_delete(const QList<ConsoleWidget *> &console_list, PolicyResultsWidget *policy_results, const int item_type, const int dn_role);
void console_policy_properties(const QList<ConsoleWidget *> &console_list, PolicyResultsWidget *policy_results, const int item_type, const int dn_role);

bool policy_is_enforced(QStandardItem *policy_item);
bool policy_is_disabled(QStandardItem *policy_item);
void set_policy_link_icon(QStandardItem *policy_item, bool is_enforced, bool is_disabled);
void set_enforced_policy_icon(QStandardItem *policy_item, bool is_enforced);
void set_disabled_policy_icon(QStandardItem *policy_item, bool is_disabled);

#endif /* POLICY_IMPL_H */
