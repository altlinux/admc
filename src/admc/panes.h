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
class MenuBar;
template <typename T> class QList;

class Panes final : public QWidget {
Q_OBJECT

public:
    FilterDialog *filter_dialog;
    QTreeView *scope_view;
    QTreeView *results_view;
    
    Panes(MenuBar *menubar_arg);

private slots:
    void on_current_scope_changed(const QModelIndex &current, const QModelIndex &);
    void on_focus_changed(QWidget *old, QWidget *now);
    void on_object_added(const QString &dn);
    void on_object_changed(const QString &dn);
    void on_object_deleted(const QString &dn);
    void on_scope_rows_about_to_be_removed(const QModelIndex &parent, int first, int last);

    void navigate_up();
    void navigate_back();
    void navigate_forward();

private:
    QStandardItemModel *scope_model;
    QTreeView *focused_view;
    MenuBar *menubar;

    // NOTE: store target history as scope node id's
    // Last is closest to current
    QList<int> targets_past;
    // First is closest to current
    QList<int> targets_future;
    int targets_current;
    bool navigated_through_history = false;

    void load_menu(QMenu *menu);
    void fetch_scope_node(const QModelIndex &index);
    void open_context_menu(const QPoint pos);
    void load_results_row(QList<QStandardItem *> row, const AdObject &object);
    void make_results_row(QStandardItemModel * model, const AdObject &object);
    QStandardItem *make_scope_item(const AdObject &object);
    void refresh_head();
    void set_current_scope(const QModelIndex &index);
    QModelIndex get_scope_node_from_id(const int id) const;
    void update_navigation_actions();
};

#endif /* PANES_H */
