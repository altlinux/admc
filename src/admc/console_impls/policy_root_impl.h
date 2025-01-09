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

#ifndef POLICY_ROOT_IMPL_H
#define POLICY_ROOT_IMPL_H

/**
 * Impl for the root of the policy tree which contains the
 * root of the domain and "All policies" folder.
 */

#include "console_widget/console_impl.h"
#include "console_widget/console_widget.h"

class PolicyRootImpl final : public ConsoleImpl {
    Q_OBJECT

public:
    PolicyRootImpl(ConsoleWidget *console_arg);

    void fetch(const QModelIndex &index) override;
    void refresh(const QList<QModelIndex> &index_list) override;

    QSet<StandardAction> get_standard_actions(const QModelIndex &index, const bool single_selection) const override;

    QList<QString> column_labels() const override;
    QList<int> default_columns() const override;
};

void console_policy_tree_init(ConsoleWidget *console);
QModelIndex get_policy_tree_root(ConsoleWidget *console);

#endif /* POLICY_ROOT_IMPL_H */
