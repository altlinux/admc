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

#ifndef ADVANCED_VIEW_PROXY_H
#define ADVANCED_VIEW_PROXY_H

/**
 * Proxy model for object model. Filters out objects that
 * are advanced view only if advanced view is turned off. If
 * advanced view is ON, shows all objects.
 */

#include <QSortFilterProxyModel>

class AdvancedViewProxy final : public QSortFilterProxyModel {
Q_OBJECT

public:
    AdvancedViewProxy(QObject *parent);

private slots:
    void on_setting_changed();
    
private:
    bool advanced_view;
    
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

#endif /* ADVANCED_VIEW_PROXY_H */
