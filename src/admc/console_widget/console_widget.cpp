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

#include "console_widget/console_widget.h"
#include "console_widget/console_widget_p.h"

#include "console_widget/console_drag_model.h"
#include "console_widget/console_impl.h"
#include "console_widget/customize_columns_dialog.h"
#include "console_widget/results_view.h"
#include "console_widget/scope_proxy_model.h"

#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QSplitter>
#include <QStack>
#include <QStackedWidget>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

#define SPLITTER_STATE "SPLITTER_STATE"
const QString CONSOLE_TREE_STATE = "CONSOLE_TREE_STATE";
const QString DESCRIPTION_BAR_STATE = "DESCRIPTION_BAR_STATE";

const QList<StandardAction> standard_action_list = {
    StandardAction_Copy,
    StandardAction_Cut,
    StandardAction_Rename,
    StandardAction_Delete,
    StandardAction_Paste,
    StandardAction_Print,
    StandardAction_Refresh,
    StandardAction_Properties,
};

QString results_state_name(const int type);

ConsoleWidget::ConsoleWidget(QWidget *parent)
: QWidget(parent) {
    d = new ConsoleWidgetPrivate(this);

    d->scope_view = new QTreeView();
    d->scope_view->setHeaderHidden(true);
    d->scope_view->setExpandsOnDoubleClick(true);
    d->scope_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    d->scope_view->setContextMenuPolicy(Qt::CustomContextMenu);
    d->scope_view->setDragDropMode(QAbstractItemView::DragDrop);
    // NOTE: this makes it so that you can't drag drop between rows (even though name/description don't say anything about that)
    d->scope_view->setDragDropOverwriteMode(true);

    d->model = new ConsoleDragModel(this);

    // NOTE: using a proxy model for scope to be able to do
    // case insensitive sorting
    d->scope_proxy_model = new ScopeProxyModel(this);
    d->scope_proxy_model->setSourceModel(d->model);
    d->scope_proxy_model->setSortCaseSensitivity(Qt::CaseInsensitive);

    d->scope_view->setModel(d->scope_proxy_model);

    d->focused_view = d->scope_view;

    d->description_bar = new QWidget();
    d->description_bar_left = new QLabel();
    d->description_bar_right = new QLabel();
    d->description_bar_left->setStyleSheet("font-weight: bold");

    d->results_stacked_widget = new QStackedWidget();

    d->standard_action_map[StandardAction_Copy] = new QAction(tr("Copy"), this);
    d->standard_action_map[StandardAction_Cut] = new QAction(tr("Cut"), this);
    d->standard_action_map[StandardAction_Rename] = new QAction(tr("Rename"), this);
    d->standard_action_map[StandardAction_Delete] = new QAction(tr("Delete"), this);
    d->standard_action_map[StandardAction_Paste] = new QAction(tr("Paste"), this);
    d->standard_action_map[StandardAction_Print] = new QAction(tr("Print"), this);
    d->standard_action_map[StandardAction_Refresh] = new QAction(tr("Refresh"), this);
    d->standard_action_map[StandardAction_Properties] = new QAction(tr("Properties"), this);

    // NOTE: need to add a dummy view until a real view is
    // added when a scope item is selected. If this is not
    // done and stacked widget is left empty, it's sizing is
    // set to minimum which causes an incorrect ratio in the
    // scope/results splitter
    auto dummy_view = new QTreeView();
    d->results_stacked_widget->addWidget(dummy_view);

    auto description_layout = new QHBoxLayout();
    description_layout->setContentsMargins(0, 0, 0, 0);
    description_layout->setSpacing(0);
    d->description_bar->setLayout(description_layout);
    description_layout->addWidget(d->description_bar_left);
    description_layout->addSpacing(10);
    description_layout->addWidget(d->description_bar_right);
    description_layout->addStretch();

    auto results_wrapper = new QWidget();
    auto results_layout = new QVBoxLayout();
    results_wrapper->setLayout(results_layout);
    results_layout->setContentsMargins(0, 0, 0, 0);
    results_layout->setSpacing(0);
    results_layout->addWidget(d->description_bar);
    results_layout->addWidget(d->results_stacked_widget);

    d->splitter = new QSplitter(Qt::Horizontal);
    d->splitter->addWidget(d->scope_view);
    d->splitter->addWidget(results_wrapper);
    d->splitter->setStretchFactor(0, 1);
    d->splitter->setStretchFactor(1, 2);

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);
    layout->addWidget(d->splitter);

    d->default_results_widget = new QWidget();
    d->results_stacked_widget->addWidget(d->default_results_widget);

    // NOTE: default impl uses base class which will do
    // nothing
    d->default_impl = new ConsoleImpl(this);

    connect(
        d->scope_view, &QTreeView::expanded,
        d, &ConsoleWidgetPrivate::on_scope_expanded);
    connect(
        d->scope_view->selectionModel(), &QItemSelectionModel::currentChanged,
        d, &ConsoleWidgetPrivate::on_current_scope_item_changed);
    connect(
        d->model, &QStandardItemModel::rowsAboutToBeRemoved,
        d, &ConsoleWidgetPrivate::on_scope_items_about_to_be_removed);

    // Update description bar when results count changes
    connect(
        d->model, &QAbstractItemModel::rowsInserted,
        d, &ConsoleWidgetPrivate::update_description);
    connect(
        d->model, &QAbstractItemModel::rowsRemoved,
        d, &ConsoleWidgetPrivate::update_description);

    for (const StandardAction &action_enum : standard_action_list) {
        QAction *action = d->standard_action_map[action_enum];

        connect(
            action, &QAction::triggered,
            this,
            [this, action_enum]() {
                d->on_standard_action(action_enum);
            });
    }

    connect(
        d->scope_view, &QWidget::customContextMenuRequested,
        d, &ConsoleWidgetPrivate::on_context_menu);

    connect(
        qApp, &QApplication::focusChanged,
        d, &ConsoleWidgetPrivate::on_focus_changed);
}

void ConsoleWidget::register_impl(const int type, ConsoleImpl *impl) {
    if (d->impl_map.contains(type)) {
        qDebug() << "Duplicate register_impl() call for type" << type;
    }

    d->impl_map[type] = impl;

    QWidget *results_widget = impl->widget();

    if (results_widget != nullptr) {
        d->results_stacked_widget->addWidget(results_widget);
    }

    ResultsView *results_view = impl->view();

    if (results_view != nullptr) {
        results_view->set_model(d->model);
        results_view->set_parent(get_current_scope_item());

        connect(
            results_view, &ResultsView::activated,
            d, &ConsoleWidgetPrivate::on_results_activated);
        connect(
            results_view, &ResultsView::context_menu,
            d, &ConsoleWidgetPrivate::on_context_menu);
    }
}

QList<QStandardItem *> ConsoleWidget::add_scope_item(const int type, const QModelIndex &parent) {
    const QList<QStandardItem *> row = add_results_item(type, parent);

    row[0]->setData(false, ConsoleRole_WasFetched);
    row[0]->setData(true, ConsoleRole_IsScope);

    d->scope_proxy_model->sort(0, Qt::AscendingOrder);

    return row;
}

QList<QStandardItem *> ConsoleWidget::add_results_item(const int type, const QModelIndex &parent) {
    QStandardItem *parent_item = [&]() {
        if (parent.isValid()) {
            return d->model->itemFromIndex(parent);
        } else {
            return d->model->invisibleRootItem();
        }
    }();

    // Make item row
    const QList<QStandardItem *> row = [&]() {
        QList<QStandardItem *> out;

        const int column_count = [&]() {
            if (parent_item == d->model->invisibleRootItem()) {
                return 1;
            } else {
                ConsoleImpl *parent_impl = d->get_impl(parent);

                return parent_impl->column_labels().size();
            }
        }();

        for (int i = 0; i < column_count; i++) {
            const auto item = new QStandardItem();
            out.append(item);
        }

        return out;
    }();

    row[0]->setData(false, ConsoleRole_IsScope);
    row[0]->setData(type, ConsoleRole_Type);

    parent_item->appendRow(row);

    return row;
}

void ConsoleWidget::delete_item(const QModelIndex &index) {
    if (!index.isValid()) {
        return;
    }

    // NOTE: select parent of index, if index is currently
    // selected. Qt crashes otherwise, not sure why.
    const QModelIndex current_scope = get_current_scope_item();
    if (current_scope == index) {
        set_current_scope(index.parent());
    }

    d->model->removeRows(index.row(), 1, index.parent());
}

void ConsoleWidget::set_current_scope(const QModelIndex &index) {
    const QModelIndex index_proxy = d->scope_proxy_model->mapFromSource(index);
    d->scope_view->selectionModel()->setCurrentIndex(index_proxy, QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void ConsoleWidget::refresh_scope(const QModelIndex &index) {
    if (!index.isValid()) {
        return;
    }

    ConsoleImpl *impl = d->get_impl(index);
    impl->refresh({index});
}

QList<QModelIndex> ConsoleWidget::get_selected_items(const int type) const {
    QList<QModelIndex> out;

    const QList<QModelIndex> all_selected = d->get_all_selected_items();

    for (const QModelIndex &index : all_selected) {
        const int this_type = console_item_get_type(index);
        if (this_type == type) {
            out.append(index);
        }
    }

    return out;
}

QModelIndex ConsoleWidget::get_selected_item(const int type) const {
    const QList<QModelIndex> selected_list = get_selected_items(type);

    if (!selected_list.isEmpty()) {
        return selected_list[0];
    } else {
        return QModelIndex();
    }
}

QList<QModelIndex> ConsoleWidget::search_items(const QModelIndex &parent, int role, const QVariant &value, const int type) const {
    const QList<QModelIndex> all_matches = [&]() {
        QList<QModelIndex> out;

        // NOTE: start index may be invalid if parent has no
        // children
        const QModelIndex start_index = d->model->index(0, 0, parent);
        if (start_index.isValid()) {
            const QList<QModelIndex> descendant_matches = d->model->match(start_index, role, value, -1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));
            out.append(descendant_matches);
        }

        const QVariant parent_value = parent.data(role);
        const bool parent_is_match = (parent_value.isValid() && parent_value == value);
        if (parent_is_match) {
            out.append(parent);
        }

        return out;
    }();

    const QList<QModelIndex> filtered_matches = [&]() {
        if (type == -1) {
            return all_matches;
        }

        QList<QModelIndex> out;

        for (const QModelIndex &index : all_matches) {
            const QVariant type_variant = index.data(ConsoleRole_Type);
            if (type_variant.isValid()) {
                const int this_type = type_variant.toInt();

                if (this_type == type) {
                    out.append(index);
                }
            }
        }

        return out;
    }();

    return filtered_matches;
}

QModelIndex ConsoleWidget::get_current_scope_item() const {
    const QModelIndex index = d->scope_view->selectionModel()->currentIndex();

    if (index.isValid()) {
        const QModelIndex source_index = d->scope_proxy_model->mapToSource(index);

        return source_index;
    } else {
        return QModelIndex();
    }
}

int ConsoleWidget::get_child_count(const QModelIndex &index) const {
    const int out = d->model->rowCount(index);

    return out;
}

QStandardItem *ConsoleWidget::get_item(const QModelIndex &index) const {
    return d->model->itemFromIndex(index);
}

QList<QStandardItem *> ConsoleWidget::get_row(const QModelIndex &index) const {
    QList<QStandardItem *> row;

    for (int col = 0; col < d->model->columnCount(index.parent()); col++) {
        const QModelIndex sibling = index.siblingAtColumn(col);
        QStandardItem *item = d->model->itemFromIndex(sibling);
        row.append(item);
    }

    return row;
}

QVariant ConsoleWidget::save_state() const {
    QHash<QString, QVariant> state;

    const QByteArray splitter_state = d->splitter->saveState();
    state[SPLITTER_STATE] = QVariant(splitter_state);

    state[CONSOLE_TREE_STATE] = d->actions.toggle_console_tree->isChecked();
    state[DESCRIPTION_BAR_STATE] = d->actions.toggle_description_bar->isChecked();

    for (const int type : d->impl_map.keys()) {
        const ConsoleImpl *impl = d->impl_map[type];
        const QString results_name = results_state_name(type);
        const QVariant results_state = impl->save_state();

        state[results_name] = results_state;
    }

    return QVariant(state);
}

void ConsoleWidget::restore_state(const QVariant &state_variant) {
    const QHash<QString, QVariant> state = state_variant.toHash();

    const QByteArray splitter_state = state.value(SPLITTER_STATE, QVariant()).toByteArray();
    d->splitter->restoreState(splitter_state);

    d->actions.toggle_console_tree->setChecked(state[CONSOLE_TREE_STATE].toBool());
    d->on_toggle_console_tree();

    d->actions.toggle_description_bar->setChecked(state[DESCRIPTION_BAR_STATE].toBool());
    d->on_toggle_description_bar();

    for (const int type : d->impl_map.keys()) {
        ConsoleImpl *impl = d->impl_map[type];

        // NOTE: need to call this before restoring results
        // state because otherwise results view header can
        // have incorrect sections which breaks state
        // restoration
        d->model->setHorizontalHeaderLabels(impl->column_labels());

        const QString results_name = results_state_name(type);
        const QVariant results_state = state.value(results_name, QVariant());

        impl->restore_state(results_state);
    }
}

void ConsoleWidget::set_scope_view_visible(const bool visible) {
    d->scope_view->setVisible(visible);
}

void ConsoleWidget::delete_children(const QModelIndex &parent) {
    d->model->removeRows(0, d->model->rowCount(parent), parent);
}

// Show/hide actions based on what's selected and emit the
// signal
void ConsoleWidget::add_actions(QMenu *menu) {
    // Add custom actions
    const QList<QAction *> custom_action_list = d->get_custom_action_list();

    for (QAction *action : custom_action_list) {
        menu->addAction(action);
    }

    menu->addSeparator();

    // Add standard actions
    menu->addAction(d->standard_action_map[StandardAction_Copy]);
    menu->addAction(d->standard_action_map[StandardAction_Cut]);
    menu->addAction(d->standard_action_map[StandardAction_Rename]);
    menu->addAction(d->standard_action_map[StandardAction_Delete]);
    menu->addAction(d->standard_action_map[StandardAction_Paste]);
    menu->addAction(d->standard_action_map[StandardAction_Print]);
    menu->addAction(d->standard_action_map[StandardAction_Refresh]);
    menu->addSeparator();
    menu->addAction(d->standard_action_map[StandardAction_Properties]);
}

void ConsoleWidget::update_actions() {
    const QList<QModelIndex> selected_list = d->get_all_selected_items();

    if (!selected_list.isEmpty() && !selected_list[0].isValid()) {
        return;
    }

    const bool single_selection = (selected_list.size() == 1);

    //
    // Collect information about action state from impl's
    //

    const QSet<QAction *> visible_custom_action_set = [&]() {
        QSet<QAction *> out;

        for (int i = 0; i < selected_list.size(); i++) {
            const QModelIndex index = selected_list[i];

            ConsoleImpl *impl = d->get_impl(index);
            QSet<QAction *> for_this_index = impl->get_custom_actions(index, single_selection);

            if (i == 0) {
                // NOTE: for first index, add the whole set
                // instead of intersecting, otherwise total set
                // would just stay empty
                out = for_this_index;
            } else {
                out.intersect(for_this_index);
            }
        }

        return out;
    }();

    const QSet<QAction *> disabled_custom_action_set = [&]() {
        QSet<QAction *> out;

        for (int i = 0; i < selected_list.size(); i++) {
            const QModelIndex index = selected_list[i];

            ConsoleImpl *impl = d->get_impl(index);
            QSet<QAction *> for_this_index = impl->get_disabled_custom_actions(index, single_selection);

            if (i == 0) {
                // NOTE: for first index, add the whole set
                // instead of intersecting, otherwise total set
                // would just stay empty
                out = for_this_index;
            } else {
                out.intersect(for_this_index);
            }
        }

        return out;
    }();

    const QSet<StandardAction> visible_standard_actions = [&]() {
        QSet<StandardAction> out;

        for (int i = 0; i < selected_list.size(); i++) {
            const QModelIndex index = selected_list[i];

            ConsoleImpl *impl = d->get_impl(index);
            QSet<StandardAction> for_this_index = impl->get_standard_actions(index, single_selection);

            if (i == 0) {
                // NOTE: for first index, add the whole set
                // instead of intersecting, otherwise total set
                // would just stay empty
                out = for_this_index;
            } else {
                out.intersect(for_this_index);
            }
        }

        return out;
    }();

    const QSet<StandardAction> disabled_standard_actions = [&]() {
        QSet<StandardAction> out;

        for (int i = 0; i < selected_list.size(); i++) {
            const QModelIndex index = selected_list[i];

            ConsoleImpl *impl = d->get_impl(index);
            QSet<StandardAction> for_this_index = impl->get_disabled_standard_actions(index, single_selection);

            if (i == 0) {
                // NOTE: for first index, add the whole set
                // instead of intersecting, otherwise total set
                // would just stay empty
                out = for_this_index;
            } else {
                out.intersect(for_this_index);
            }
        }

        return out;
    }();

    //
    // Apply action state
    //

    const QList<QAction *> custom_action_list = d->get_custom_action_list();

    // Show/hide custom actions
    for (QAction *action : custom_action_list) {
        const bool visible = visible_custom_action_set.contains(action);

        action->setVisible(visible);
    }

    // Enable/disable custom actions
    for (QAction *action : custom_action_list) {
        const bool disabled = disabled_custom_action_set.contains(action);

        action->setDisabled(disabled);
    }

    // Show/hide standard actions
    for (const StandardAction &action_enum : standard_action_list) {
        const bool visible = visible_standard_actions.contains(action_enum);

        QAction *action = d->standard_action_map[action_enum];
        action->setVisible(visible);
    }

    // Enable/disable standard actions
    for (const StandardAction &action_enum : standard_action_list) {
        const bool disabled = disabled_standard_actions.contains(action_enum);

        QAction *action = d->standard_action_map[action_enum];
        action->setDisabled(disabled);
    }
}

void ConsoleWidget::set_actions(const ConsoleWidgetActions &actions_arg) {
    d->actions = actions_arg;

    // Setup exclusivity for view type actions
    auto view_type_group = new QActionGroup(this);
    view_type_group->addAction(d->actions.view_icons);
    view_type_group->addAction(d->actions.view_list);
    view_type_group->addAction(d->actions.view_detail);

    connect(
        d->actions.navigate_up, &QAction::triggered,
        d, &ConsoleWidgetPrivate::on_navigate_up);
    connect(
        d->actions.navigate_back, &QAction::triggered,
        d, &ConsoleWidgetPrivate::on_navigate_back);
    connect(
        d->actions.navigate_forward, &QAction::triggered,
        d, &ConsoleWidgetPrivate::on_navigate_forward);
    connect(
        d->actions.refresh, &QAction::triggered,
        d, &ConsoleWidgetPrivate::on_refresh);
    connect(
        d->actions.customize_columns, &QAction::triggered,
        d, &ConsoleWidgetPrivate::on_customize_columns);
    connect(
        d->actions.view_icons, &QAction::triggered,
        d, &ConsoleWidgetPrivate::on_view_icons);
    connect(
        d->actions.view_list, &QAction::triggered,
        d, &ConsoleWidgetPrivate::on_view_list);
    connect(
        d->actions.view_detail, &QAction::triggered,
        d, &ConsoleWidgetPrivate::on_view_detail);
    connect(
        d->actions.toggle_console_tree, &QAction::triggered,
        d, &ConsoleWidgetPrivate::on_toggle_console_tree);
    connect(
        d->actions.toggle_description_bar, &QAction::triggered,
        d, &ConsoleWidgetPrivate::on_toggle_description_bar);

    d->update_navigation_actions();
    d->update_view_actions();
}

ConsoleWidgetPrivate::ConsoleWidgetPrivate(ConsoleWidget *q_arg)
: QObject(q_arg) {
    q = q_arg;
}

// NOTE: as long as this is called where appropriate (on every target change), it is not necessary to do any condition checks in navigation f-ns since the actions that call them will be disabled if they can't be done
void ConsoleWidgetPrivate::update_navigation_actions() {
    const bool can_navigate_up = [this]() {
        const QModelIndex current = q->get_current_scope_item();
        const QModelIndex current_parent = current.parent();

        return (current.isValid() && current_parent.isValid());
    }();

    actions.navigate_up->setEnabled(can_navigate_up);
    actions.navigate_back->setEnabled(!targets_past.isEmpty());
    actions.navigate_forward->setEnabled(!targets_future.isEmpty());
}

void ConsoleWidgetPrivate::update_view_actions() {
    ConsoleImpl *impl = get_current_scope_impl();
    const bool results_view_exists = (impl->view() != nullptr);

    actions.view_icons->setVisible(results_view_exists);
    actions.view_list->setVisible(results_view_exists);
    actions.view_detail->setVisible(results_view_exists);
    actions.customize_columns->setVisible(results_view_exists);
}

void ConsoleWidgetPrivate::start_drag(const QList<QPersistentModelIndex> &dropped_list_arg) {
    dropped_list = dropped_list_arg;

    dropped_type_list = [&]() {
        QSet<int> out;

        for (const QPersistentModelIndex &index : dropped_list) {
            const int type = index.data(ConsoleRole_Type).toInt();
            out.insert(type);
        }

        return out;
    }();
}

bool ConsoleWidgetPrivate::can_drop(const QModelIndex &target) {
    const int target_type = target.data(ConsoleRole_Type).toInt();

    ConsoleImpl *impl = get_impl(target);
    const bool ok = impl->can_drop(dropped_list, dropped_type_list, target, target_type);

    return ok;
}

void ConsoleWidgetPrivate::drop(const QModelIndex &target) {
    const int target_type = target.data(ConsoleRole_Type).toInt();

    ConsoleImpl *impl = get_impl(target);
    impl->drop(dropped_list, dropped_type_list, target, target_type);
}

void ConsoleWidgetPrivate::set_results_to_type(const ResultsViewType type) {
    ConsoleImpl *impl = get_current_scope_impl();

    if (impl->view() != nullptr) {
        impl->view()->set_view_type(type);
    }
}

void ConsoleWidgetPrivate::fetch_scope(const QModelIndex &index) {
    const bool was_fetched = index.data(ConsoleRole_WasFetched).toBool();

    if (!was_fetched) {
        model->setData(index, true, ConsoleRole_WasFetched);

        ConsoleImpl *impl = get_impl(index);
        impl->fetch(index);
    }
}

ConsoleImpl *ConsoleWidgetPrivate::get_current_scope_impl() const {
    const QModelIndex current_scope = q->get_current_scope_item();

    ConsoleImpl *impl = get_impl(current_scope);

    return impl;
}

ConsoleImpl *ConsoleWidgetPrivate::get_impl(const QModelIndex &index) const {
    const int type = index.data(ConsoleRole_Type).toInt();
    ConsoleImpl *impl = impl_map.value(type, default_impl);

    return impl;
}

void ConsoleWidgetPrivate::update_description() {
    const QModelIndex current_scope = q->get_current_scope_item();

    const QString scope_name = current_scope.data().toString();

    ConsoleImpl *impl = get_impl(current_scope);
    const QString description = impl->get_description(current_scope);

    description_bar_left->setText(scope_name);
    description_bar_right->setText(description);
}

QList<QModelIndex> ConsoleWidgetPrivate::get_all_selected_items() const {
    ConsoleImpl *current_impl = get_current_scope_impl();
    ResultsView *results_view = current_impl->view();

    const bool focused_scope = (focused_view == scope_view);
    const bool focused_results = (results_view != nullptr && focused_view == results_view->current_view());

    const QList<QModelIndex> scope_selected = [&]() {
        QList<QModelIndex> out;

        const QList<QModelIndex> selected_proxy = scope_view->selectionModel()->selectedRows();
        for (const QModelIndex &index : selected_proxy) {
            const QModelIndex source_index = scope_proxy_model->mapToSource(index);
            out.append(source_index);
        }

        return out;
    }();

    if (focused_scope) {
        return scope_selected;
    } else if (focused_results) {
        const QList<QModelIndex> results_selected = results_view->get_selected_indexes();

        if (!results_selected.isEmpty()) {
            return results_selected;
        } else {
            return scope_selected;
        }
    } else {
        return QList<QModelIndex>();
    }
}

QSet<int> ConsoleWidgetPrivate::get_selected_types() const {
    QSet<int> out;

    const QList<QModelIndex> index_list = get_all_selected_items();

    for (const QModelIndex &index : index_list) {
        const int type = index.data(ConsoleRole_Type).toInt();
        out.insert(type);
    }

    return out;
}

// Get all custom actions from impl's, in order
QList<QAction *> ConsoleWidgetPrivate::get_custom_action_list() const {
    QList<QAction *> out;

    for (const int type : impl_map.keys()) {
        ConsoleImpl *impl = impl_map[type];
        QList<QAction *> for_this_type = impl->get_all_custom_actions();

        for (QAction *action : for_this_type) {
            if (!out.contains(action)) {
                out.append(action);
            }
        }
    }

    return out;
}

void ConsoleWidgetPrivate::on_current_scope_item_changed(const QModelIndex &current_proxy, const QModelIndex &previous_proxy) {
    // NOTE: technically this slot should never be called
    // with invalid current index
    if (!current_proxy.isValid()) {
        return;
    }

    const QModelIndex current = scope_proxy_model->mapToSource(current_proxy);
    const QModelIndex previous = scope_proxy_model->mapToSource(previous_proxy);

    // Move current to past, if current changed and is valid
    if (previous.isValid() && (previous != current)) {
        targets_past.append(QPersistentModelIndex(previous));
    }

    // When a new current is selected, a new future begins
    targets_future.clear();

    ConsoleImpl *impl = get_current_scope_impl();

    impl->selected_as_scope(current);

    // Switch to new parent for results view's model if view
    // exists
    const bool results_view_exists = (impl->view() != nullptr);
    if (results_view_exists) {
        model->setHorizontalHeaderLabels(impl->column_labels());

        // NOTE: setting horizontal labes may make columns
        // visible again in scope view, so re-hide them
        for (int i = 1; i < 100; i++) {
            scope_view->hideColumn(i);
        }

        // NOTE: need to add a dummy row if new current item
        // has no children because otherwise view header
        // will be hidden. Dummy row is removed later.
        const bool need_dummy = (model->rowCount(current) == 0);
        if (need_dummy) {
            q->add_results_item(0, current);
        }

        impl->view()->set_parent(current);

        if (need_dummy) {
            model->removeRow(0, current);
        }

        const ResultsViewType results_type = impl->view()->current_view_type();
        switch (results_type) {
            case ResultsViewType_Icons: {
                actions.view_icons->setChecked(true);

                break;
            }
            case ResultsViewType_List: {
                actions.view_list->setChecked(true);

                break;
            }
            case ResultsViewType_Detail: {
                actions.view_detail->setChecked(true);

                break;
            }
        }
    }

    // Switch to this item's results widget
    QWidget *results_widget = impl->widget();
    if (results_widget != nullptr) {
        results_stacked_widget->setCurrentWidget(results_widget);
    } else {
        results_stacked_widget->setCurrentWidget(default_results_widget);
    }

    update_navigation_actions();
    update_view_actions();

    fetch_scope(current);

    update_description();
}

void ConsoleWidgetPrivate::on_scope_items_about_to_be_removed(const QModelIndex &parent, int first, int last) {
    const QList<QModelIndex> removed_scope_items = [=]() {
        QList<QModelIndex> out;

        QStack<QStandardItem *> stack;

        for (int r = first; r <= last; r++) {
            const QModelIndex removed_index = model->index(r, 0, parent);
            auto removed_item = model->itemFromIndex(removed_index);
            stack.push(removed_item);
        }

        while (!stack.isEmpty()) {
            auto item = stack.pop();

            out.append(item->index());

            // Iterate through children
            for (int r = 0; r < item->rowCount(); r++) {
                auto child = item->child(r, 0);
                stack.push(child);
            }
        }

        return out;
    }();

    // Remove removed items from navigation history
    for (const QModelIndex index : removed_scope_items) {
        targets_past.removeAll(index);
        targets_future.removeAll(index);
    }

    // Update navigation since an item in history could've been removed
    update_navigation_actions();
}

void ConsoleWidgetPrivate::on_focus_changed(QWidget *old, QWidget *now) {
    UNUSED_ARG(old);

    QAbstractItemView *new_focused_view = qobject_cast<QAbstractItemView *>(now);

    if (new_focused_view != nullptr) {
        ConsoleImpl *current_impl = get_current_scope_impl();
        QAbstractItemView *results_view = [=]() -> QAbstractItemView * {
            ResultsView *view = current_impl->view();
            if (view != nullptr) {
                return view->current_view();
            } else {
                return nullptr;
            }
        }();

        if (new_focused_view == scope_view || new_focused_view == results_view) {
            focused_view = new_focused_view;
        }
    }
}

void ConsoleWidgetPrivate::on_refresh() {
    const QModelIndex current_scope = q->get_current_scope_item();

    q->refresh_scope(current_scope);
}

void ConsoleWidgetPrivate::on_customize_columns() {
    ConsoleImpl *current_impl = get_current_scope_impl();
    ResultsView *results_view = current_impl->view();

    if (results_view != nullptr) {
        auto dialog = new CustomizeColumnsDialog(results_view->detail_view(), current_impl->default_columns(), q);
        dialog->open();
    }
}

// Set target to parent of current target
void ConsoleWidgetPrivate::on_navigate_up() {
    const QPersistentModelIndex old_current = q->get_current_scope_item();

    if (!old_current.isValid()) {
        return;
    }

    const QModelIndex new_current = old_current.parent();

    // NOTE: parent of target can be invalid, if current is
    // head item in which case we've reached top of tree and
    // can't go up higher
    const bool can_go_up = (!new_current.isValid());
    if (!can_go_up) {
        q->set_current_scope(new_current);
    }
}

// NOTE: for "back" and "forward" navigation, setCurrentIndex() triggers "current changed" slot which by default erases future history, so manually restore correct navigation state afterwards
void ConsoleWidgetPrivate::on_navigate_back() {
    const QPersistentModelIndex old_current = q->get_current_scope_item();

    if (!old_current.isValid()) {
        return;
    }

    // NOTE: saving and restoring past and future
    // before/after setCurrentIndex() because on_current()
    // slot modifies them with the assumption that current
    // changed due to user input.
    auto saved_past = targets_past;
    auto saved_future = targets_future;

    const QPersistentModelIndex new_current = targets_past.last();
    q->set_current_scope(new_current);

    targets_past = saved_past;
    targets_future = saved_future;

    targets_past.removeLast();
    targets_future.prepend(old_current);

    update_navigation_actions();
}

void ConsoleWidgetPrivate::on_navigate_forward() {
    const QPersistentModelIndex old_current = q->get_current_scope_item();

    if (!old_current.isValid()) {
        return;
    }

    // NOTE: saving and restoring past and future
    // before/after setCurrentIndex() because on_current()
    // slot modifies them with the assumption that current
    // changed due to user input.
    auto saved_past = targets_past;
    auto saved_future = targets_future;

    const QPersistentModelIndex new_current = targets_future.first();
    q->set_current_scope(new_current);

    targets_past = saved_past;
    targets_future = saved_future;

    targets_past.append(old_current);
    targets_future.removeFirst();

    update_navigation_actions();
}

void ConsoleWidgetPrivate::on_view_icons() {
    set_results_to_type(ResultsViewType_Icons);
}

void ConsoleWidgetPrivate::on_view_list() {
    set_results_to_type(ResultsViewType_List);
}

void ConsoleWidgetPrivate::on_view_detail() {
    set_results_to_type(ResultsViewType_Detail);
}

void ConsoleWidgetPrivate::on_toggle_console_tree() {
    const bool visible = actions.toggle_console_tree->isChecked();
    scope_view->setVisible(visible);
}

void ConsoleWidgetPrivate::on_toggle_description_bar() {
    const bool visible = actions.toggle_description_bar->isChecked();
    description_bar->setVisible(visible);
}

void ConsoleWidgetPrivate::on_standard_action(const StandardAction action_enum) {
    const QSet<int> type_set = get_selected_types();

    // Call impl's action f-n for all present types
    for (const int type : type_set) {
        // Filter selected list so that it only contains indexes of this type
        const QList<QModelIndex> selected_of_type = q->get_selected_items(type);

        ConsoleImpl *impl = impl_map[type];
        switch (action_enum) {
            case StandardAction_Copy: {
                impl->copy(selected_of_type);

                break;
            }
            case StandardAction_Cut: {
                impl->cut(selected_of_type);

                break;
            }
            case StandardAction_Rename: {
                impl->rename(selected_of_type);

                break;
            }
            case StandardAction_Delete: {
                impl->delete_action(selected_of_type);

                break;
            }
            case StandardAction_Paste: {
                impl->paste(selected_of_type);

                break;
            }
            case StandardAction_Print: {
                impl->print(selected_of_type);

                break;
            }
            case StandardAction_Refresh: {
                impl->refresh(selected_of_type);

                break;
            }
            case StandardAction_Properties: {
                impl->properties(selected_of_type);

                break;
            }
        }
    }
}

void ConsoleWidgetPrivate::on_context_menu(const QPoint &pos) {
    const QModelIndex index = focused_view->indexAt(pos);

    if (index.isValid()) {
        const QPoint global_pos = focused_view->mapToGlobal(pos);

        auto menu = new QMenu(q);
        menu->setAttribute(Qt::WA_DeleteOnClose);
        q->add_actions(menu);
        q->update_actions();
        menu->popup(global_pos);
    }
}

void ConsoleWidgetPrivate::on_scope_expanded(const QModelIndex &index_proxy) {
    const QModelIndex index = scope_proxy_model->mapToSource(index_proxy);
    fetch_scope(index);
}

void ConsoleWidgetPrivate::on_results_activated(const QModelIndex &index) {
    const QModelIndex main_index = index.siblingAtColumn(0);
    const bool is_scope = main_index.data(ConsoleRole_IsScope).toBool();

    if (is_scope) {
        q->set_current_scope(main_index);
    } else {
        ConsoleImpl *impl = get_impl(main_index);
        impl->activate(main_index);
    }
}

int console_item_get_type(const QModelIndex &index) {
    const int type = index.data(ConsoleRole_Type).toInt();

    return type;
}

bool console_item_get_was_fetched(const QModelIndex &index) {
    const int was_fetched = index.data(ConsoleRole_WasFetched).toBool();

    return was_fetched;
}

QString results_state_name(const int type) {
    return QString("RESULTS_STATE_%1").arg(type);
}
