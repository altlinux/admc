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

#ifndef CENTRAL_WIDGET_H
#define CENTRAL_WIDGET_H

/**
 * The central widget of the app through which user can
 * browse and manipulate objects. Wraps ConsoleWidget inside it.
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
class ConsoleActions;
class PolicyResultsWidget;
template <typename T> class QList;

enum ItemType {
    ItemType_Unassigned,
    ItemType_Object,
    ItemType_PolicyRoot,
    ItemType_Policy,
    ItemType_QueryRoot,
    ItemType_QueryFolder,
    ItemType_QueryItem,

    ItemType_LAST,
};

class CentralWidget final : public QWidget {
Q_OBJECT

public:        
    CentralWidget();

    void go_online(AdInterface &ad);
    void add_actions_to_action_menu(QMenu *menu);
    void add_actions_to_navigation_menu(QMenu *menu);
    void add_actions_to_view_menu(QMenu *menu);

signals:
    void context_menu(const QPoint pos);

private slots:
    void open_filter();

    void object_properties();
    void object_delete();
    void object_rename();
    void object_move();
    void object_add_to_group();
    void object_enable();
    void object_disable();
    void object_find();
    void object_reset_password();
    void object_create_user();
    void object_create_computer();
    void object_create_ou();
    void object_create_group();
    void object_edit_upn_suffixes();

    void policy_add_link();
    void policy_delete();

    void query_create();
    void query_edit();
    void query_delete();
    void query_export();
    void query_import();
    void query_cut();
    void query_copy();
    void query_paste();
    
    void on_items_can_drop(const QList<QPersistentModelIndex> &dropped, const QPersistentModelIndex &target, bool *ok);
    void on_items_dropped(const QList<QPersistentModelIndex> &dropped, const QPersistentModelIndex &target);

    void on_current_scope_changed();
    void on_object_properties_applied();

private:
    ConsoleWidget *console;
    QPersistentModelIndex object_tree_head;
    FilterDialog *filter_dialog;
    PolicyResultsWidget *policy_results_widget;
    
    ConsoleActions *console_actions;

    QAction *open_filter_action;
    QAction *show_noncontainers_action;
    QAction *dev_mode_action;

    void update_description_bar();
    void enable_disable_helper(const bool disabled);
    void update_actions_visibility();
    void object_create_helper(const QString &object_class);
    QHash<QString, QPersistentModelIndex> get_selected_dns_and_indexes();
    QList<QString> get_selected_dns();
    QString get_selected_dn();
    void fetch_scope_node(const QModelIndex &index);
    void refresh_head();
};

#endif /* CENTRAL_WIDGET_H */
