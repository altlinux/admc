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

#include "console_impls/find_policy_impl.h"

#include "adldap.h"
#include "console_impls/object_impl.h"
#include "console_widget/results_view.h"
#include "item_type.h"
#include "utils.h"

#include <QModelIndex>
#include <QStandardItem>

FindPolicyImpl::FindPolicyImpl(ConsoleWidget *console_arg)
: ConsoleImpl(console_arg) {
    auto view = new ResultsView(console_arg);
    view->set_drag_drop_enabled(false);
    set_results_view(view);
}

QString FindPolicyImpl::get_description(const QModelIndex &index) const {
    const QString object_count_text = console_object_count_string(console, index);

    return object_count_text;
}

QList<QString> FindPolicyImpl::column_labels() const {
    const QList<QString> out = {
        tr("Name"),
        tr("GUID"),
    };

    return out;
}

QList<int> FindPolicyImpl::default_columns() const {
    const QList<int> out = {
        FindPolicyColumn_Name,
        FindPolicyColumn_GUID,
    };

    return out;
}

QModelIndex get_find_policy_root(ConsoleWidget *console) {
    const QModelIndex out = console->search_item(QModelIndex(), {ItemType_FindPolicy});

    return out;
}
