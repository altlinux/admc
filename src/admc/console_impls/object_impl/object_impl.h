/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#ifndef OBJECT_IMPL_H
#define OBJECT_IMPL_H

/**
 * Impl for AD objects. Displays a hierarchical tree of
 * objects that exist in the domain.
 */

#include "adldap.h"
#include "console_impls/my_console_role.h"
#include "console_widget/console_impl.h"
#include "console_widget/console_widget.h"
#include "console_object_operations.h"

class QStandardItem;
class AdObject;
class AdInterface;
class ConsoleActions;
class QMenu;
template <typename T>
class QList;
class ConsoleWidget;
class ConsoleFilterDialog;
class GeneralUserTab;
class GeneralGroupTab;
class QStackedWidget;
class PSOResultsWidget;
class SubnetResultsWidget;

enum ObjectRole {
    ObjectRole_DN = MyConsoleRole_LAST + 1,
    ObjectRole_ObjectClasses,
    ObjectRole_ObjectCategory,
    ObjectRole_CannotMove,
    ObjectRole_CannotRename,
    ObjectRole_CannotDelete,
    ObjectRole_AccountDisabled,
    ObjectRole_Fetching,
    ObjectRole_SearchId,

    ObjectRole_LAST,
};

class ObjectImpl final : public ConsoleImpl {
    Q_OBJECT

public:
    ObjectImpl(ConsoleWidget *console);

    // This is for cases where there are multiple consoles
    // in the app and you need to propagate changes from one
    // to another. For example, when objects are deleted in
    // this console, they will also get removed from the
    // buddy console.
    void set_buddy_console(ConsoleWidget *buddy_console);

    void fetch(const QModelIndex &index) override;
    bool can_drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type) override;
    void drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type) override;
    QString get_description(const QModelIndex &index) const override;
    void activate(const QModelIndex &index) override;

    QList<QAction *> get_all_custom_actions() const override;
    QSet<QAction *> get_custom_actions(const QModelIndex &index, const bool single_selection) const override;
    QSet<QAction *> get_disabled_custom_actions(const QModelIndex &index, const bool single_selection) const override;

    QSet<StandardAction> get_standard_actions(const QModelIndex &index, const bool single_selection) const override;
    QSet<StandardAction> get_disabled_standard_actions(const QModelIndex &index, const bool single_selection) const override;

    void rename(const QList<QModelIndex> &index_list) override;
    void properties(const QList<QModelIndex> &index_list) override;
    void refresh(const QList<QModelIndex> &index_list) override;
    void delete_action(const QList<QModelIndex> &index_list) override;

    void selected_as_scope(const QModelIndex &index) override;

    void update_results_widget(const QModelIndex &index) const override;

    void set_find_action_enabled(const bool enabled);
    void set_refresh_action_enabled(const bool enabled);
    void set_toolbar_actions(QAction *create_user, QAction *create_group, QAction *create_ou);

    QList<QString> column_labels() const override;
    QList<int> default_columns() const override;

    void refresh_tree();

    void open_console_filter_dialog();

private slots:
    void on_new_user();
    void on_new_computer();
    void on_new_ou();
    void on_new_group();
    void on_new_shared_folder();
    void on_move();
    void on_enable();
    void on_disable();
    void on_add_to_group();
    void on_find();
    void on_reset_password();
    void on_edit_upn_suffixes();
    void on_reset_account();

private:
    QList<ConsoleWidget *> console_list;
    QString object_filter;
    bool object_filter_enabled;

    QAction *find_action;
    QAction *move_action;
    QAction *add_to_group_action;
    QAction *enable_action;
    QAction *disable_action;
    QAction *reset_password_action;
    QAction *reset_account_action;
    QAction *edit_upn_suffixes_action;
    QAction *new_action;
    QAction *create_subnet_action;
    QAction *create_site_action;
    QAction *create_site_link_action;
    QAction *create_site_link_bridge_action;
    QHash<QString, QAction *> standard_create_action_map; // For containers that can contain children of
                                                          // different classes;

    QAction *toolbar_create_user;
    QAction *toolbar_create_group;
    QAction *toolbar_create_ou;

    QStackedWidget *stacked_widget;
    GeneralGroupTab *group_results_widget;
    GeneralUserTab *user_results_widget;
    PSOResultsWidget *pso_results_widget;
    SubnetResultsWidget *subnet_results_widget;


    bool find_action_enabled;
    bool refresh_action_enabled;

    void new_object(const QString &object_class);
    void set_disabled(const bool disabled);
    void move_and_rename(AdInterface &ad, const QHash<QString, QString> &old_dn_list, const QString &new_parent_dn);
    void move(AdInterface &ad, const QList<QString> &old_dn_list, const QString &new_parent_dn);
    void update_toolbar_actions();
    QList<QString> get_selected_dn_list_object();
    QString get_selected_target_dn_object();
    bool can_create_class_at_parent(const QString &create_class, const QString &parent_class) const;
    void setup_widgets();
    void setup_filters();
    void setup_actions();
};


#endif /* OBJECT_IMPL_H */
