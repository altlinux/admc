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

#ifndef CONSOLE_H
#define CONSOLE_H

/**
 * The central widget of the app through which user can
 * browse and manipulate objects. Contains two panes:
 * "scope" and "results". Scope pane contains a tree of objects.
 * Results pane contains a list of objects which are children of
 * currently selected scope item.
 */

#include <QWidget>
#include <QPersistentModelIndex>

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
class QLabel;
class QSortFilterProxyModel;
class AdInterface;
class ConsoleWidget;
template <typename T> class QList;

class Console final : public QWidget {
Q_OBJECT

public:    
    ConsoleWidget *console_widget;
    
    Console();

    void go_online(AdInterface &ad);

private slots:
    void refresh_head();

    void open_filter();
    void delete_objects(const QList<QModelIndex> &indexes);
    void on_properties_requested();
    void rename();
    void create(const QString &object_class);
    void move();

    void fetch_scope_node(const QModelIndex &index);
    void on_action_menu_about_to_open(QMenu *menu, QTreeView *view);
    void on_view_menu_about_to_open(QMenu *menu);
    
    void on_items_can_drop(const QList<QModelIndex> &dropped, const QModelIndex &target, bool *ok);
    void on_items_dropped(const QList<QModelIndex> &dropped, const QModelIndex &target);

private:
    int object_results_id;
    QPersistentModelIndex scope_head_index;
    FilterDialog *filter_dialog;
    QAction *rename_action;
    QAction *move_action;
    QAction *open_filter_action;
    QAction *advanced_view_action;
    QAction *show_noncontainers_action;
    QAction *dev_mode_action;
    QTreeView *object_results;

    void update_description_bar();
    void setup_scope_item(QStandardItem *item, const AdObject &object);
    void add_object_to_console(const AdObject &object, const    QModelIndex &parent);
    void move_object_in_console(AdInterface &ad, const QModelIndex &old_index, const QModelIndex &new_parent_index);
};

#endif /* CONSOLE_H */
