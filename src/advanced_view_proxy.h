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

#ifndef AD_PROXY_MODEL_H
#define AD_PROXY_MODEL_H

#include <QSortFilterProxyModel>

class QModelIndex;

// Show/hide advanced objects depending on whether advanced view is on
class AdvancedViewProxy final : public QSortFilterProxyModel {
public:
    explicit AdvancedViewProxy(int dn_column_arg, QObject *parent);

private slots:
    void on_advanced_view_toggled(bool checked);

private:
    int dn_column;
    bool advanced_view_is_on;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

#endif /* AD_PROXY_MODEL_H */
