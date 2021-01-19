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
class QMenu;
class ObjectMenu;

enum Role {
    Role_Id = Qt::UserRole + 1,
    Role_Fetched = Qt::UserRole + 2,
    Role_DN = Qt::UserRole + 3,
    Role_ObjectClass = Qt::UserRole + 4,
};

class Panes final : public QWidget {
Q_OBJECT

public:
    FilterDialog *filter_dialog;
    QTreeView *scope_view;
    QTreeView *results_view;
    ObjectMenu *menu;
    
    Panes();

    void reset();

private slots:
    void change_results_target(const QModelIndex &current, const QModelIndex &);

private:

    QStandardItemModel *scope_model;

    void fetch_scope_item(const QModelIndex &index);
};

#endif /* PANES_H */
