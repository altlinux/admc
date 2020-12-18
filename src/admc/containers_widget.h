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

class QItemSelection;
class AdvancedViewProxy;
class QTreeView;
class ObjectModel;
class ContainersProxy;

class ContainersWidget final : public QWidget {
Q_OBJECT

public:
    ContainersWidget(ObjectModel *model, QWidget *parent);

signals:
    void selected_changed(const QModelIndex &source_index);

private slots:
    void on_selection_changed(const QItemSelection &selected, const QItemSelection &);

private:
    QTreeView *view;
    AdvancedViewProxy *advanced_view_proxy;
    ContainersProxy *containers_proxy;

    void showEvent(QShowEvent *event);
};

#endif /* CONTAINERS_WIDGET_H */
