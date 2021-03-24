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

class QAbstractItemView;
class QStandardItemModel;
class QModelIndex;
class QString;
class FilterDialog;
class QMenu;
class QPoint;
class QStandardItem;
class AdObject;
class QLabel;
class QSortFilterProxyModel;
class AdInterface;
class ConsoleWidget;
class ResultsView;
class ObjectActions;
template <typename T> class QList;

class Console final : public QWidget {
Q_OBJECT

public:    
    ConsoleWidget *console_widget;
    
    Console();

    void go_online(AdInterface &ad);
    void add_actions_to_action_menu(QMenu *menu);
    void add_actions_to_navigation_menu(QMenu *menu);
    void add_actions_to_view_menu(QMenu *menu);

signals:
    void context_menu(const QPoint pos);

private slots:
    void refresh_head();

    void open_filter();
    void on_properties_requested();
    void delete_objects();
    void rename();
    void move();
    void add_to_group();
    void enable();
    void disable();
    void find();
    void reset_password();
    void create_user();
    void create_computer();
    void create_ou();
    void create_group();

    void fetch_scope_node(const QModelIndex &index);
    
    void on_items_can_drop(const QList<QModelIndex> &dropped, const QModelIndex &target, bool *ok);
    void on_items_dropped(const QList<QModelIndex> &dropped, const QModelIndex &target);

private:
    int object_results_id;
    QPersistentModelIndex scope_head_index;
    FilterDialog *filter_dialog;

    ObjectActions *object_actions;

    QAction *open_filter_action;
    QAction *show_noncontainers_action;
    QAction *dev_mode_action;
    ResultsView *object_results;

    void update_description_bar();
    void setup_scope_item(QStandardItem *item, const AdObject &object);
    void setup_results_row(const QList<QStandardItem *> row, const AdObject &object);
    void add_object_to_console(const AdObject &object, const    QModelIndex &parent);
    void move_object_in_console(AdInterface &ad, const QPersistentModelIndex &old_index, const QString &new_parent_dn, const QPersistentModelIndex &new_parent_index);
    void update_console_item(const QModelIndex &index, const AdObject &object);
    void disable_drag_if_object_cant_be_moved(const QList<QStandardItem *> &items, const AdObject &object);
    QList<QString> get_dns(const QList<QModelIndex> &indexes);
    void enable_disable_helper(const bool disabled);
    void update_actions_visibility();
    void create_helper(const QString &object_class);
    QHash<QString, QPersistentModelIndex> get_selected_dns_and_indexes();
    QList<QString> get_selected_dns();
};

#endif /* CONSOLE_H */
