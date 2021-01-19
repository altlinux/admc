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

#ifndef PANES_H
#define PANES_H

#include <QWidget>

class QTreeView;
class QStandardItemModel;
class QModelIndex;
class QString;
class FilterDialog;

class Panes final : public QWidget {
Q_OBJECT

public:
    FilterDialog *filter_dialog;
    
    Panes();

private slots:
    void on_current_scope_changed(const QModelIndex &current, const QModelIndex &);

private:
    QTreeView *scope_view;
    QTreeView *results_view;
    QStandardItemModel *scope_model;

    void fetch_scope_item(const QModelIndex &index);
    void reset();
};

#endif /* PANES_H */
