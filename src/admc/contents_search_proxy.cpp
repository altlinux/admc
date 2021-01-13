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

#include "contents_search_proxy.h"

#include "ad_config.h"
#include "utils.h"

void ContentsSearchProxy::update_search_text(const QString &search_text_arg) {
    search_text = search_text_arg;
    invalidateFilter();
}

void ContentsSearchProxy::update_root_index(const QModelIndex &root_index_arg) {
    root_index = root_index_arg;
    invalidateFilter();
}

bool ContentsSearchProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    const QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    // NOTE: always match root index. If this is not done and no children match search text, nothing matches in the whole proxy model and it becomes empty, which messes up root index.
    if (index == root_index) {
        return true;
    }

    const QString name = get_dn_from_index(index, ADCONFIG()->get_column_index(ATTRIBUTE_NAME));

    return name.contains(search_text);
}
