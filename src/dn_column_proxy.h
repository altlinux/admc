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

#ifndef DN_COLUMN_PROXY_H
#define DN_COLUMN_PROXY_H

#include <QSortFilterProxyModel>

class QModelIndex;
class EntryModel;

// Show/hide DN column depending on "show DN" setting
class DnColumnProxy final : public QSortFilterProxyModel {
public:
    explicit DnColumnProxy(int dn_column_arg, QObject *parent);

private slots:
    void on_toggle_show_dn_column(bool checked);

private:
    int dn_column;
    bool show_dn_column;

    bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;
};

#endif /* DN_COLUMN_PROXY_H */
