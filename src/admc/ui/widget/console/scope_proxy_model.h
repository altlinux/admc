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

#ifndef SCOPE_PROXY_MODEL_H
#define SCOPE_PROXY_MODEL_H

#include <QSortFilterProxyModel>

/**
 * Proxy model for the scope pane of the console. Filters
 * out items that shouldn't be shown in scope. Also
 * implements showing expander for items that haven't been
 * fetched yet.
 */

class ScopeProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    using QSortFilterProxyModel::QSortFilterProxyModel;

    bool hasChildren(const QModelIndex &parent) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
};

#endif /* SCOPE_PROXY_MODEL_H */
