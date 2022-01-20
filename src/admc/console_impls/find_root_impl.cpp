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

#include "console_impls/find_root_impl.h"

#include "adldap.h"
#include "console_impls/object_impl.h"
#include "console_widget/results_view.h"
#include "item_type.h"

#include <QStandardItem>
#include <QModelIndex>

FindRootImpl::FindRootImpl(ConsoleWidget *console_arg)
: ConsoleImpl(console_arg) {
    set_results_view(new ResultsView(console_arg));
}

QString FindRootImpl::get_description(const QModelIndex &index) const {
    const QString object_count_text = console_object_count_string(console, index);

    return object_count_text;
}

QList<QString> FindRootImpl::column_labels() const {
    return object_impl_column_labels();
}

QList<int> FindRootImpl::default_columns() const {
    return object_impl_default_columns();
}

QModelIndex get_find_tree_root(ConsoleWidget *console) {
    const QList<QModelIndex> index_list = console->search_items(QModelIndex(), ItemType_FindRoot);

    if (!index_list.isEmpty()) {
        return index_list[0];
    } else {
        return QModelIndex();
    }
}
