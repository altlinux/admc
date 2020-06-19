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
class EntryModel;

// Show/hide advanced entries depending on whether advanced view is on
// Show/hide non-container entries
class AdProxyModel final : public QSortFilterProxyModel {
public:
    explicit AdProxyModel(EntryModel *model_arg, QObject *parent);

    bool only_show_containers = false;

private slots:
    void on_advanced_view_toggled(bool checked);

private:
    EntryModel *model;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

#endif /* AD_PROXY_MODEL_H */
