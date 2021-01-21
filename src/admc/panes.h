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
class QPoint;
class QStandardItem;
class AdObject;
template <typename T> class QList;

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
    
    Panes();

    void load_head();
    void setup_menubar_menu(QMenu *menu);

private slots:
    void change_results_target(const QModelIndex &current, const QModelIndex &);
    void on_focus_changed(QWidget *old, QWidget *now);
    void on_object_added(const QString &dn);
    void on_object_changed(const QString &dn);
    void on_object_deleted(const QString &dn);
    void on_scope_rows_about_to_be_removed(const QModelIndex &parent, int first, int last);

private:
    QStandardItemModel *scope_model;
    QTreeView *focused_view;

    void fetch_scope_node(const QModelIndex &index);
    void open_context_menu(const QPoint pos);
    void load_results_row(QList<QStandardItem *> row, const AdObject &object);
    void make_results_row(QStandardItemModel * model, const AdObject &object);
    void make_scope_item(QStandardItem *parent, const AdObject &object);
    void load_menu(QMenu *menu);
};

#endif /* PANES_H */
