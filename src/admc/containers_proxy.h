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

#ifndef CONTAINERS_PROXY_H
#define CONTAINERS_PROXY_H

#include <QSortFilterProxyModel>

class ContainersProxy;

/**
 * Proxy model for object model. Filters out objects that
 * aren't containers. If "Show non-containers" setting is
 * turned on, shows all objects.
 */

class ContainersProxy final : public QSortFilterProxyModel {
Q_OBJECT

public:
    ContainersProxy(QObject *parent);

private slots:
    void on_show_non_containers();
    
private:
    bool show_non_containers;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

#endif /* CONTAINERS_PROXY_H */
