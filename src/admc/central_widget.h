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
class ResultsView;
class ObjectActions;
class PolicyResultsWidget;
template <typename T> class QList;

enum ItemType {
    ItemType_Unassigned,
    ItemType_DomainObject,
    ItemType_PoliciesRoot,
    ItemType_Policy,
    ItemType_QueriesRoot,
    ItemType_QueryFolder,
    ItemType_QueryItem,

    ItemType_LAST,
};

class CentralWidget final : public QWidget {
Q_OBJECT

public:    
    ConsoleWidget *console_widget;
    
    CentralWidget();

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
    void edit_upn_suffixes();

    void create_policy();
    void add_link();
    void rename_policy();
    void delete_policy();

    void new_query_folder();
    void new_query();
    void delete_query_item_or_folder();
    
    void on_items_can_drop(const QList<QModelIndex> &dropped, const QModelIndex &target, bool *ok);
    void on_items_dropped(const QList<QModelIndex> &dropped, const QModelIndex &target);

    void on_current_scope_changed();

private:
    int object_results_id;
    int policies_results_id;
    int policy_links_results_id;
    int query_folder_results_id;
    QPersistentModelIndex scope_head_index;
    QPersistentModelIndex policies_index;
    QPersistentModelIndex queries_index;
    FilterDialog *filter_dialog;
    PolicyResultsWidget *policy_results_widget;

    ObjectActions *object_actions;

    QHash<ItemType, QList<QAction *>> item_actions;
    
    QAction *open_filter_action;
    QAction *show_noncontainers_action;
    QAction *dev_mode_action;
    ResultsView *object_results;

    void update_description_bar();
    bool console_add_objects_check(const QModelIndex &parent);
    void console_add_objects(const QList<AdObject> &object_list, const QModelIndex &parent);
    void console_add_objects(AdInterface &ad, const QList<QString> &dn_list, const QModelIndex &parent);
    void console_move_objects(AdInterface &ad, const QList<QString> &old_dn_list, const QList<QString> &new_dn_list, const QString &new_parent_dn);
    void console_move_objects(AdInterface &ad, const QList<QString> &old_dn_list, const QString &new_parent_dn);
    void console_update_object(const AdObject &object);
    void console_update_policy(const QModelIndex &index, const AdObject &object);
    QList<QString> get_dns(const QList<QModelIndex> &indexes);
    void enable_disable_helper(const bool disabled);
    void update_actions_visibility();
    void create_helper(const QString &object_class);
    QHash<QString, QPersistentModelIndex> get_selected_dns_and_indexes();
    QList<QString> get_selected_dns();
    void console_add_policy(const AdObject &object);
    void fetch_scope_node(const QModelIndex &index);
    void fetch_query(const QModelIndex &index);
    void fetch_object(const QModelIndex &index);
    void save_queries();
    void console_delete_objects(const QList<QString> &dn_list, const bool ignore_query_tree = false);
};

#endif /* CENTRAL_WIDGET_H */
