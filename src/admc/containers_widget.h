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

#ifndef CONTAINERS_WIDGET_H
#define CONTAINERS_WIDGET_H

#include <QWidget>
#include <QSortFilterProxyModel>

class QItemSelection;
class AdvancedViewProxy;
class QTreeView;
class ObjectModel;
class ContainersFilterProxy;

class ContainersWidget final : public QWidget {
Q_OBJECT

public:
    ContainersWidget(QWidget *parent);
    ObjectModel *model;

signals:
    void selected_changed(const QString &dn);

private slots:
    void on_selection_changed(const QItemSelection &selected, const QItemSelection &);
    void on_context_menu(const QPoint pos);

private:
    QTreeView *view;
    ContainersFilterProxy *proxy;

    void showEvent(QShowEvent *event);
};

class ContainersFilterProxy final : public QSortFilterProxyModel {
Q_OBJECT

public:
    ContainersFilterProxy(QObject *parent);

private slots:
    void on_show_non_containers();
    
private:
    bool show_non_containers;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

#endif /* CONTAINERS_WIDGET_H */
