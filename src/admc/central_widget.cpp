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

#include "central_widget.h"

#include "adldap.h"
#include "console_actions.h"
#include "console_types/console_object.h"
#include "console_types/console_policy.h"
#include "console_types/console_query.h"
#include "console_widget/console_widget.h"
#include "console_widget/results_view.h"

#include "filter_dialog.h"
#include "globals.h"
#include "gplink.h"
#include "policy_results_widget.h"
#include "settings.h"
#include "status.h"
#include "utils.h"

#include <QDebug>
#include <QHeaderView>
#include <QMenu>
#include <QSplitter>
#include <QStandardItem>
#include <QTreeView>
#include <QVBoxLayout>

CentralWidget::CentralWidget(AdInterface &ad)
: QWidget() {
    console_actions = new ConsoleActions(this);

    open_filter_action = new QAction(tr("&Filter objects"), this);

    // NOTE: these actions are not connected here because
    // they need to be connected to a custom slot
    dev_mode_action = settings_make_action(SETTING_dev_mode, tr("Dev mode"), this);
    show_noncontainers_action = settings_make_action(SETTING_show_non_containers_in_console_tree, tr("&Show non-container objects in Console tree"), this);
    advanced_features_action = settings_make_action(SETTING_advanced_features, tr("Advanced features"), this);

    toggle_console_tree_action = settings_make_action(SETTING_advanced_features, tr("Console Tree"), this);
    toggle_description_bar_action = settings_make_action(SETTING_show_results_header, tr("Description Bar"), this);

    console = new ConsoleWidget();

    filter_dialog = new FilterDialog(this);

    auto console_object = new ConsoleObject(filter_dialog, console);
    console->register_type(ItemType_Object, console_object);

    auto console_policy_root = new ConsolePolicyRoot(console);
    console->register_type(ItemType_PolicyRoot, console_policy_root);

    auto console_query_item = new ConsoleQueryItem(console);
    console->register_type(ItemType_QueryItem, console_query_item);

    auto object_results = new ResultsView(this);
    console_object_results_id = console->register_results(object_results, console_object_header_labels(), console_object_default_columns());

    auto policy_container_results = new ResultsView(this);
    policy_container_results->detail_view()->header()->setDefaultSectionSize(200);
    policy_container_results_id = console->register_results(policy_container_results, console_policy_header_labels(), console_policy_default_columns());

    policy_results_widget = new PolicyResultsWidget();
    policy_results_id = console->register_results(policy_results_widget);

    auto query_results = new ResultsView(this);
    query_results->detail_view()->header()->setDefaultSectionSize(200);
    console_query_folder_results_id = console->register_results(query_results, console_query_folder_header_labels(), console_query_folder_default_columns());

    // NOTE: requires all results to be initialized
    console_object_tree_init(console, ad);
    console_policy_tree_init(console, ad);
    console_query_tree_init(console);

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);
    layout->addWidget(console);

    const QVariant console_widget_state = settings_get_variant(SETTING_console_widget_state);
    console->restore_state(console_widget_state);

    connect_object_actions(console, console_actions);
    connect_policy_actions(console, console_actions, policy_results_widget);
    connect_query_actions(console, console_actions);

    connect(
        show_noncontainers_action, &QAction::toggled,
        this, &CentralWidget::on_show_non_containers);
    on_show_non_containers();
    
    connect(
        dev_mode_action, &QAction::toggled,
        this, &CentralWidget::on_dev_mode);
    on_dev_mode();

    connect(
        advanced_features_action, &QAction::toggled,
        this, &CentralWidget::on_advanced_features);
    on_dev_mode();

    connect(
        toggle_console_tree_action, &QAction::toggled,
        this, &CentralWidget::on_toggle_console_tree);
    on_toggle_console_tree();

    connect(
        toggle_description_bar_action, &QAction::toggled,
        this, &CentralWidget::on_toggle_description_bar);
    on_toggle_description_bar();

    connect(
        open_filter_action, &QAction::triggered,
        filter_dialog, &QDialog::open);
    connect(
        filter_dialog, &QDialog::accepted,
        this, &CentralWidget::refresh_object_tree);

    connect(
        console, &ConsoleWidget::current_scope_item_changed,
        this, &CentralWidget::on_current_scope_changed);
    connect(
        console, &ConsoleWidget::results_count_changed,
        this, &CentralWidget::update_description_bar);
    connect(
        console, &ConsoleWidget::items_can_drop,
        this, &CentralWidget::on_items_can_drop);
    connect(
        console, &ConsoleWidget::items_dropped,
        this, &CentralWidget::on_items_dropped);
    connect(
        console, &ConsoleWidget::actions_changed,
        this, &CentralWidget::on_actions_changed);

    // Set current scope to object head to load it
    console->set_current_scope(console_object_head()->index());
}

CentralWidget::~CentralWidget() {
    const QVariant state = console->save_state();
    settings_set_variant(SETTING_console_widget_state, state);
}

void CentralWidget::on_actions_changed() {
    const QList<QModelIndex> selected_list = console->get_selected_items();

    console_actions->update_actions_visibility(selected_list);
}

void CentralWidget::on_items_can_drop(const QList<QPersistentModelIndex> &dropped_list, const QPersistentModelIndex &target, bool *ok) {
    const bool dropped_contains_target = [&]() {
        for (const QPersistentModelIndex &dropped : dropped_list) {
            if (dropped == target) {
                return true;
            }
        }

        return false;
    }();

    if (dropped_contains_target) {
        return;
    }

    const ItemType target_type = (ItemType) target.data(ConsoleRole_Type).toInt();
    const QSet<ItemType> dropped_types = [&]() {
        QSet<ItemType> out;

        for (const QPersistentModelIndex &index : dropped_list) {
            const ItemType type = (ItemType) index.data(ConsoleRole_Type).toInt();
            out.insert(type);
        }

        return out;
    }();

    switch (target_type) {
        case ItemType_Unassigned: break;
        case ItemType_Object: {
            console_object_can_drop(dropped_list, target, dropped_types, ok);
            break;
        }
        case ItemType_PolicyRoot: break;
        case ItemType_Policy: {
            console_policy_can_drop(dropped_list, target, dropped_types, ok);
            break;
        }
        case ItemType_QueryRoot: {
            console_query_can_drop(dropped_list, target, dropped_types, ok);
            break;
        }
        case ItemType_QueryFolder: {
            console_query_can_drop(dropped_list, target, dropped_types, ok);
            break;
        }
        case ItemType_QueryItem: break;
        case ItemType_LAST: break;
    }
}

void CentralWidget::on_items_dropped(const QList<QPersistentModelIndex> &dropped_list, const QPersistentModelIndex &target) {
    const ItemType target_type = (ItemType) target.data(ConsoleRole_Type).toInt();
    const QSet<ItemType> dropped_types = [&]() {
        QSet<ItemType> out;

        for (const QPersistentModelIndex &index : dropped_list) {
            const ItemType type = (ItemType) index.data(ConsoleRole_Type).toInt();
            out.insert(type);
        }

        return out;
    }();

    switch (target_type) {
        case ItemType_Unassigned: break;
        case ItemType_Object: {
            console_object_drop(console, dropped_list, dropped_types, target, policy_results_widget);
            break;
        }
        case ItemType_PolicyRoot: break;
        case ItemType_Policy: {
            console_policy_drop(console, dropped_list, target, policy_results_widget);
            break;
        }
        case ItemType_QueryRoot: {
            console_query_drop(console, dropped_list, target);
            break;
        }
        case ItemType_QueryFolder: {
            console_query_drop(console, dropped_list, target);
            break;
        }
        case ItemType_QueryItem: break;
        case ItemType_LAST: break;
    }
}

void CentralWidget::on_current_scope_changed() {
    const QModelIndex current_scope = console->get_current_scope_item();
    policy_results_widget->update(current_scope);

    update_description_bar();
}

void CentralWidget::on_show_non_containers() {
    settings_set_bool(SETTING_show_non_containers_in_console_tree, show_noncontainers_action->isChecked());

    refresh_object_tree();
}

void CentralWidget::on_dev_mode() {
    settings_set_bool(SETTING_dev_mode, dev_mode_action->isChecked());

    refresh_object_tree();
}

void CentralWidget::on_advanced_features() {
    settings_set_bool(SETTING_advanced_features, advanced_features_action->isChecked());

    refresh_object_tree();
}

void CentralWidget::on_toggle_console_tree() {
    const bool visible = toggle_console_tree_action->isChecked();
    
    settings_set_bool(SETTING_advanced_features, visible);
    console->get_scope_view()->setVisible(visible);
}

void CentralWidget::on_toggle_description_bar() {
    const bool visible = toggle_description_bar_action->isChecked();
    
    settings_set_bool(SETTING_advanced_features, visible);
    console->get_description_bar()->setVisible(visible);
}

void CentralWidget::refresh_object_tree() {
    show_busy_indicator();

    console->refresh_scope(console_object_head()->index());

    hide_busy_indicator();
}

void CentralWidget::update_description_bar() {
    const QString text = [this]() {
        const QModelIndex current_scope = console->get_current_scope_item();
        const ItemType type = (ItemType) current_scope.data(ConsoleRole_Type).toInt();

        const QString object_count_text = [&]() {
            const int results_count = console->get_current_results_count();
            const QString out = tr("%n object(s)", "", results_count);

            return out;
        }();

        if (type == ItemType_Object) {
            QString out = object_count_text;

            const bool filtering_ON = filter_dialog->filtering_ON();
            if (filtering_ON) {
                out += tr(" [Filtering enabled]");
            }

            return out;
        } else if (type == ItemType_QueryItem) {
            return object_count_text;
        } else {
            return QString();
        }
    }();

    console->set_description_bar_text(text);
}

void CentralWidget::add_actions(QMenu *action_menu, QMenu *navigation_menu, QMenu *view_menu, QMenu *preferences_menu, QToolBar *toolbar) {
    console_actions->add_to_menu(action_menu);

    action_menu->addSeparator();

    console->add_actions(action_menu, navigation_menu, view_menu, toolbar);

    view_menu->addSeparator();

    view_menu->addAction(open_filter_action);

    view_menu->addAction(show_noncontainers_action);

#ifdef QT_DEBUG
    view_menu->addAction(dev_mode_action);
#endif

    preferences_menu->addAction(advanced_features_action);
    preferences_menu->addAction(toggle_console_tree_action);
    preferences_menu->addAction(toggle_description_bar_action);
}
