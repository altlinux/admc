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

#ifndef ALL_POLICIES_FOLDER_IMPL_H
#define ALL_POLICIES_FOLDER_IMPL_H

/**
 * Impl for a virtual container for "All policies". Displays
 * all of the policies present in the domain.
 */

#include "console_widget/console_impl.h"
#include "console_widget/console_widget.h"

class AdObject;
class AdInterface;

class AllPoliciesFolderImpl final : public ConsoleImpl {
    Q_OBJECT

public:
    AllPoliciesFolderImpl(ConsoleWidget *console_arg);

    void fetch(const QModelIndex &index) override;
    void refresh(const QList<QModelIndex> &index_list) override;

    QList<QAction *> get_all_custom_actions() const override;
    QSet<QAction *> get_custom_actions(const QModelIndex &index, const bool single_selection) const override;
    QSet<StandardAction> get_standard_actions(const QModelIndex &index, const bool single_selection) const override;

    QList<QString> column_labels() const override;
    QList<int> default_columns() const override;

private:
    QAction *create_policy_action;

    void create_policy();
};

QModelIndex get_all_policies_folder_index(ConsoleWidget *console);
void all_policies_folder_impl_add_objects(ConsoleWidget *console, const QList<AdObject> &object_list, const QModelIndex &parent);

#endif /* ALL_POLICIES_FOLDER_IMPL_H */
