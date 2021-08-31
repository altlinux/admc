/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

#include "central_widget.h"
#include "console_widget/console_widget.h"
#include "console_widget/console_impl.h"

class QStandardItem;
class AdObject;
class AdInterface;
class ConsoleActions;
class PolicyResultsWidget;
class CreatePolicyDialog;
template <typename T>
class QList;

enum PolicyRole {
    PolicyRole_DN = ConsoleRole_LAST + 1,

    PolicyRole_LAST = ConsoleRole_LAST + 2,
};

void console_policy_load(const QList<QStandardItem *> &row, const AdObject &object);
QList<QString> console_policy_search_attributes();
void console_policy_add_link(ConsoleWidget *console, const QList<QString> &policy_list, const QList<QString> &ou_list, PolicyResultsWidget *policy_results_widget);

class PolicyImpl final : public ConsoleImpl {
    Q_OBJECT

public:
    PolicyImpl(PolicyResultsWidget *policy_results_widget_arg, ConsoleWidget *console_arg);

    bool can_drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type) override;
    void drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type) override;

    void selected_as_scope(const QModelIndex &index) override;

    QList<QAction *> get_all_custom_actions() const override;
    QSet<QAction *> get_custom_actions(const QModelIndex &index, const bool single_selection) const override;
    QSet<StandardAction> get_standard_actions(const QModelIndex &index, const bool single_selection) const override;

    virtual void rename(const QList<QModelIndex> &index_list);
    virtual void delete_action(const QList<QModelIndex> &index_list);
    virtual void refresh(const QList<QModelIndex> &index_list);

private:
    PolicyResultsWidget *policy_results_widget;
    QDialog *rename_dialog;
    QAction *add_link_action;
    QAction *edit_action;

    void on_add_link();
    void on_edit();
};

#endif /* POLICY_IMPL_H */
