/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
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

#include "scope_model.h"

#include "console_widget_p.h"

// This tricks the view into thinking that an item in tree
// has children while the item is unfetched. This causes the
// expander to be shown while the item is unfetched. After
// the item is fetched, normal behavior is restored.
bool ScopeModel::hasChildren(const QModelIndex &parent) const {
    const bool was_fetched = parent.data(ConsoleRole_WasFetched).toBool();
    const bool unfetched = !was_fetched;

    if (unfetched) {
        return true;
    } else {
        return QStandardItemModel::hasChildren(parent);
    }
}
