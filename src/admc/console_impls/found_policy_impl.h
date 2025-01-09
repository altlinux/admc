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

#ifndef FOUND_POLICY_IMPL_H
#define FOUND_POLICY_IMPL_H

/**
 * Impl for policy objects displayed in FindPolicyDialog.
 */

#include "console_widget/console_impl.h"
#include "console_widget/console_widget.h"

class AdObject;

enum FoundPolicyRole {
    FoundPolicyRole_DN = ConsoleRole_LAST + 1,

    FoundPolicyRole_LAST = ConsoleRole_LAST + 2,
};

class FoundPolicyImpl final : public ConsoleImpl {
    Q_OBJECT

public:
    FoundPolicyImpl(ConsoleWidget *console_arg);

    void set_buddy_console(ConsoleWidget *buddy_console);

    QList<QAction *> get_all_custom_actions() const override;
    QSet<QAction *> get_custom_actions(const QModelIndex &index, const bool single_selection) const override;
    QSet<StandardAction> get_standard_actions(const QModelIndex &index, const bool single_selection) const override;

    void rename(const QList<QModelIndex> &index_list) override;
    void delete_action(const QList<QModelIndex> &index_list) override;
    void properties(const QList<QModelIndex> &index_list) override;

private slots:
    void on_add_link();
    void on_edit();

private:
    QList<ConsoleWidget *> console_list;
    QAction *add_link_action;
    QAction *edit_action;
};

QList<QString> console_policy_search_attributes();

void found_policy_impl_load(const QList<QStandardItem *> &row, const AdObject &object);

#endif /* FOUND_POLICY_IMPL_H */
