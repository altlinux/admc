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

#include "entry_widget.h"

class QItemSelection;
class AdModel;
class EntryProxyModel;

// Display tree of container entries
// And some other "container-like" entries like domain, built-in, etc
// Only shows the name column
class ContainersWidget final : public EntryWidget {
Q_OBJECT

public:
    ContainersWidget(AdModel *model, QWidget *parent);

signals:
    void selected_container_changed(const QModelIndex &selected);

private slots:
    void on_selection_changed(const QItemSelection &selected, const QItemSelection &);

private:
    EntryProxyModel *proxy = nullptr;

};

#endif /* CONTAINERS_WIDGET_H */
