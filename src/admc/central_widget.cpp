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
#include "item_type.h"

#include <QDebug>
#include <QHeaderView>
#include <QMenu>
#include <QSplitter>
#include <QStandardItem>
#include <QTreeView>
#include <QVBoxLayout>

CentralWidget::CentralWidget(AdInterface &ad, QMenu *action_menu)
: QWidget() {
    open_filter_action = new QAction(tr("&Filter objects..."), this);

    // NOTE: these actions are not connected here because
    // they need to be connected to a custom slot
    dev_mode_action = settings_make_action(SETTING_dev_mode, tr("Dev mode"), this);
    show_noncontainers_action = settings_make_action(SETTING_show_non_containers_in_console_tree, tr("&Show non-container objects in Console tree"), this);
    advanced_features_action = settings_make_action(SETTING_advanced_features, tr("Advanced features"), this);

    console = new ConsoleWidget(this);
    console->connect_to_action_menu(action_menu);

    auto filter_dialog = new FilterDialog(this);

    //
    // Register console results
    //
    auto object_results = new ResultsView(this);
    console->register_results(ItemType_Object, object_results, console_object_header_labels(), console_object_default_columns());
    
    auto query_item_results = new ResultsView(this);
    console->register_results(ItemType_QueryItem, query_item_results, console_object_header_labels(), console_object_default_columns());

    auto policy_container_results = new ResultsView(this);
    console->register_results(ItemType_PolicyRoot, policy_container_results, console_policy_header_labels(), console_policy_default_columns());

    auto policy_results_widget = new PolicyResultsWidget(this);
    console->register_results(ItemType_Policy, policy_results_widget);

    auto query_folder_results = new ResultsView(this);
    console->register_results(ItemType_QueryFolder, query_folder_results, console_query_folder_header_labels(), console_query_folder_default_columns());

    //
    // Register console types
    //
    auto object_impl = new ConsoleObject(policy_results_widget, filter_dialog, console);
    console->register_impl(ItemType_Object, object_impl);

    auto policy_root_impl = new ConsolePolicyRoot(console);
    console->register_impl(ItemType_PolicyRoot, policy_root_impl);

    auto policy_impl = new ConsolePolicy(policy_results_widget, console);
    console->register_impl(ItemType_Policy, policy_impl);

    auto query_item_impl = new ConsoleQueryItem(console);
    console->register_impl(ItemType_QueryItem, query_item_impl);

    auto query_folder_impl = new ConsoleQueryFolder(console);
    console->register_impl(ItemType_QueryFolder, query_folder_impl);

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

    connect(
        show_noncontainers_action, &QAction::toggled,
        this, &CentralWidget::on_show_non_containers);
    connect(
        dev_mode_action, &QAction::toggled,
        this, &CentralWidget::on_dev_mode);
    connect(
        advanced_features_action, &QAction::toggled,
        this, &CentralWidget::on_advanced_features);

    connect(
        open_filter_action, &QAction::triggered,
        filter_dialog, &QDialog::open);
    connect(
        filter_dialog, &QDialog::accepted,
        this, &CentralWidget::refresh_object_tree);

    // Set current scope to object head to load it
    console->set_current_scope(console_object_head()->index());
}

CentralWidget::~CentralWidget() {
    const QVariant state = console->save_state();
    settings_set_variant(SETTING_console_widget_state, state);
}

// NOTE: f-ns below need to manually change a setting and
// then refresh_object_tree() because setting needs to be
// changed before tree is refreshed. If you do this in a
// more convenient way by connecting as a slot, call order
// will be undefined.

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

void CentralWidget::refresh_object_tree() {
    show_busy_indicator();

    console->refresh_scope(console_object_head()->index());

    hide_busy_indicator();
}

void CentralWidget::add_actions(QMenu *view_menu, QMenu *preferences_menu, QToolBar *toolbar) {
    // Preferences
    preferences_menu->addAction(advanced_features_action);
    console->add_preferences_actions(preferences_menu);

    // View
    console->add_view_actions(view_menu);

#ifdef QT_DEBUG
    view_menu->addAction(dev_mode_action);
#endif

    // Toolbar
    console->add_toolbar_actions(toolbar);
}
