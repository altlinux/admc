/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

#ifndef CONSOLE_OBJECT_H
#define CONSOLE_OBJECT_H

#include "adldap.h"
#include "central_widget.h"
#include "console_widget/console_widget.h"
#include "console_widget/console_impl.h"
#include "console_types/my_console_role.h"

class QStandardItem;
class AdObject;
class AdInterface;
class ConsoleActions;
class QMenu;
class PolicyResultsWidget;
template <typename T>
class QList;
class FilterDialog;

/**
 * Some f-ns used for models that store objects.
 */

enum ObjectRole {
    ObjectRole_DN = MyConsoleRole_LAST + 1,
    ObjectRole_ObjectClasses,
    ObjectRole_CannotMove,
    ObjectRole_CannotRename,
    ObjectRole_CannotDelete,
    ObjectRole_AccountDisabled,
    ObjectRole_Fetching,
    ObjectRole_SearchId,

    ObjectRole_LAST,
};

void console_object_load(const QList<QStandardItem *> row, const AdObject &object);
void console_object_item_data_load(QStandardItem *item, const AdObject &object);
QList<QString> console_object_header_labels();
QList<int> console_object_default_columns();
QList<QString> console_object_search_attributes();
void console_object_create(ConsoleWidget *console, const QList<AdObject> &object_list, const QModelIndex &parent);
void console_object_create(ConsoleWidget *console, AdInterface &ad, const QList<QString> &dn_list, const QModelIndex &parent);
void console_object_search(ConsoleWidget *console, const QModelIndex &index, const QString &base, const SearchScope scope, const QString &filter, const QList<QString> &attributes);
void console_object_tree_init(ConsoleWidget *console, AdInterface &ad);

// NOTE: this f-ns don't do anything with console. Just
// convenience code for reuse
QList<QString> object_operation_delete(const QList<QString> &targets, QWidget *parent);
QList<QString> object_operation_set_disabled(const QList<QString> &targets, const bool disabled, QWidget *parent);
void object_operation_add_to_group(const QList<QString> &targets, QWidget *parent);

bool console_object_is_ou(const QModelIndex &index);
void console_object_load_root_text(QStandardItem *item);

// NOTE: this may return an invalid index if there's no tree
// of objects setup
QModelIndex get_object_tree_root(ConsoleWidget *console);

QString console_object_count_string(ConsoleWidget *console, const QModelIndex &index);

class ConsoleObject final : public ConsoleImpl {
    Q_OBJECT

public:
    ConsoleObject(PolicyResultsWidget *policy_results_widget_arg, FilterDialog *filter_dialog_arg, ConsoleWidget *console_arg);

    void fetch(const QModelIndex &index);
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

    void set_find_action_enabled(const bool enabled);
    void set_refresh_action_enabled(const bool enabled);

private:
    FilterDialog *filter_dialog;
    PolicyResultsWidget *policy_results_widget;

    QAction *find_action;
    QAction *move_action;
    QAction *add_to_group_action;
    QAction *enable_action;
    QAction *disable_action;
    QAction *reset_password_action;
    QAction *reset_account_action;
    QAction *edit_upn_suffixes_action;
    QAction *change_dc_action;
    QAction *new_action;

    bool find_action_enabled;
    bool refresh_action_enabled;

    void on_new_user();
    void on_new_computer();
    void on_new_ou();
    void on_new_group();
    void on_move();
    void on_enable();
    void on_disable();
    void on_add_to_group();
    void on_find();
    void on_reset_password();
    void on_edit_upn_suffixes();
    void on_reset_account();
    void on_change_dc();

    void new_object(const QString &object_class);
    void set_disabled(const bool disabled);
    void drop_objects(const QList<QPersistentModelIndex> &dropped_list, const QPersistentModelIndex &target);
    void drop_policies(const QList<QPersistentModelIndex> &dropped_list, const QPersistentModelIndex &target);
    void move_and_rename(AdInterface &ad, const QList<QString> &old_dn_list, const QString &new_parent_dn, const QList<QString> &new_dn_list);
    void move(AdInterface &ad, const QList<QString> &old_dn_list, const QString &new_parent_dn);
};

#endif /* CONSOLE_OBJECT_H */
