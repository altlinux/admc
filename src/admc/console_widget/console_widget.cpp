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
#include "console_widget/customize_columns_dialog.h"
#include "console_widget/results_description.h"
#include "console_widget/results_view.h"
#include "console_widget/scope_proxy_model.h"
#include "console_widget/console_impl.h"

#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QSplitter>
#include <QStack>
#include <QStackedWidget>
#include <QTreeView>
#include <QVBoxLayout>
#include <QToolBar>

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

ConsoleWidgetPrivate::ConsoleWidgetPrivate(ConsoleWidget *q_arg)
: QObject(q_arg) {
    q = q_arg;
}

ConsoleWidget::ConsoleWidget(QWidget *parent)
: QWidget(parent) {
    d = new ConsoleWidgetPrivate(this);

    d->scope_view = new QTreeView();
    d->scope_view->setHeaderHidden(true);
    d->scope_view->setExpandsOnDoubleClick(true);
    d->scope_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    d->scope_view->setContextMenuPolicy(Qt::CustomContextMenu);
    d->scope_view->setDragDropMode(QAbstractItemView::DragDrop);
    d->scope_view->sortByColumn(0, Qt::AscendingOrder);
    d->scope_view->setSortingEnabled(true);
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

    d->navigate_up_action = new QAction(QIcon::fromTheme("go-up"), tr("&Up one level"), this);
    d->navigate_back_action = new QAction(QIcon::fromTheme("go-previous"), tr("&Back"), this);
    d->navigate_forward_action = new QAction(QIcon::fromTheme("go-next"), tr("&Forward"), this);
    // NOTE: this refresh action is for the action menu.
    // Refreshes selected object, if it can be refreshed.
    // Hidden if can't refresh.
    d->refresh_action = new QAction(tr("&Refresh"), this);
    // NOTE: this refresh action is for the toolbar.
    // Refreshes current scope item, if it can be refreshed.
    // Always visible.
    d->refresh_current_scope_action = new QAction(QIcon::fromTheme("view-refresh"), tr("&Refresh"), this);
    d->customize_columns_action = new QAction(tr("&Customize columns..."), this);
    d->set_results_to_icons_action = new QAction(tr("&Icons"), this);
    d->set_results_to_list_action = new QAction(tr("&List"), this);
    d->set_results_to_detail_action = new QAction(tr("&Detail"), this);

    d->toggle_console_tree_action = new QAction(tr("Console Tree"), this);
    d->toggle_console_tree_action->setCheckable(true);
    d->toggle_description_bar_action = new QAction(tr("Description Bar"), this);
    d->toggle_description_bar_action->setCheckable(true);

    d->navigate_up_action->setShortcut(QKeySequence(Qt::ALT + Qt::Key_0));
    d->navigate_back_action->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Minus));
    d->navigate_forward_action->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Plus));

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

    auto default_results_widget = new QWidget();
    d->default_results = ResultsDescription(default_results_widget, nullptr, QList<QString>(), QList<int>());
    d->results_stacked_widget->addWidget(default_results_widget);

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

    connect(
        d->navigate_up_action, &QAction::triggered,
        d, &ConsoleWidgetPrivate::navigate_up);
    connect(
        d->navigate_back_action, &QAction::triggered,
        d, &ConsoleWidgetPrivate::navigate_back);
    connect(
        d->navigate_forward_action, &QAction::triggered,
        d, &ConsoleWidgetPrivate::navigate_forward);
    connect(
        d->refresh_current_scope_action, &QAction::triggered,
        d, &ConsoleWidgetPrivate::refresh_current_scope);
    connect(
        d->customize_columns_action, &QAction::triggered,
        d, &ConsoleWidgetPrivate::customize_columns);
    connect(
        d->set_results_to_icons_action, &QAction::triggered,
        d, &ConsoleWidgetPrivate::set_results_to_icons);
    connect(
        d->set_results_to_list_action, &QAction::triggered,
        d, &ConsoleWidgetPrivate::set_results_to_list);
    connect(
        d->set_results_to_detail_action, &QAction::triggered,
        d, &ConsoleWidgetPrivate::set_results_to_detail);
    connect(
        d->toggle_console_tree_action, &QAction::triggered,
        d, &ConsoleWidgetPrivate::on_toggle_console_tree);
    connect(
        d->toggle_description_bar_action, &QAction::triggered,
        d, &ConsoleWidgetPrivate::on_toggle_description_bar);

    for (const StandardAction &action_enum : standard_action_list) {
        QAction *action = d->standard_action_map[action_enum];

        connect(
            action, &QAction::triggered,
            [this, action_enum]() {
                d->on_standard_action(action_enum);
            });
    }

    connect(
        d->scope_view, &QWidget::customContextMenuRequested,
        d, &ConsoleWidgetPrivate::context_menu);

    connect(
        qApp, &QApplication::focusChanged,
        d, &ConsoleWidgetPrivate::on_focus_changed);

    d->update_navigation_actions();
    d->update_view_actions();
}

void ConsoleWidget::register_impl(const int type, ConsoleImpl *impl) {
    if (!d->impl_map.contains(type)) {
        d->impl_map[type] = impl;
    } else {
        qDebug() << "Duplicate register_impl() call for type" << type;
    }
}

QList<QStandardItem *> ConsoleWidget::add_scope_item(const int type, const QModelIndex &parent) {
    const QList<QStandardItem *> row = add_results_item(type, parent);

    row[0]->setData(false, ConsoleRole_WasFetched);
    row[0]->setData(true, ConsoleRole_IsScope);

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
        
        const int column_count =
        [&]() {
            if (parent_item == d->model->invisibleRootItem()) {
                return 1;
            } else {
                const ResultsDescription parent_results = d->get_results(parent);

                return parent_results.column_count();
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
    d->scope_view->selectionModel()->setCurrentIndex(index_proxy, QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);
}

void ConsoleWidget::refresh_scope(const QModelIndex &index) {
    if (!index.isValid()) {
        return;
    }

    ConsoleImpl *impl = d->get_impl(index);
    impl->refresh({index});
}

// No view, only widget
void ConsoleWidget::register_results(const int type, QWidget *widget) {
    register_results(type, widget, nullptr, QList<QString>(), QList<int>());
}

// In this case, view *is* the widget
void ConsoleWidget::register_results(const int type, ResultsView *view, const QList<QString> &column_labels, const QList<int> &default_columns) {
    register_results(type, view, view, column_labels, default_columns);
}

// Base register() f-n
void ConsoleWidget::register_results(const int type, QWidget *widget, ResultsView *view, const QList<QString> &column_labels, const QList<int> &default_columns) {
    // NOTE: reusing widget or view is not allowed because
    // it causes buggy behavior due to duplicate connections
    const bool already_registered_widget = d->registered_results_widget_list.contains(widget);
    if (already_registered_widget) {
        qDebug() << "register_results() for type" << type << " uses widget that has already been registered. This is not allowed, so aborting";

        return;
    }

    const bool already_registered_view = d->registered_results_view_list.contains(view);
    if (already_registered_view) {
        qDebug() << "register_results() for type" << type << " uses view that has already been registered. This is not allowed, so aborting";

        return;
    }

    d->registered_results_widget_list.append(widget);

    d->results_descriptions[type] = ResultsDescription(widget, view, column_labels, default_columns);

    d->results_stacked_widget->addWidget(widget);

    if (view != nullptr) {
        d->registered_results_view_list.append(view);

        view->set_model(d->model);
        view->set_parent(get_current_scope_item());

        connect(
            view, &ResultsView::activated,
            d, &ConsoleWidgetPrivate::on_results_activated);
        connect(
            view, &ResultsView::context_menu,
            d, &ConsoleWidgetPrivate::context_menu);
    }
}

QList<QModelIndex> ConsoleWidget::get_selected_items() const {
    ResultsView *results_view = d->get_current_results().view();

    const bool focused_scope = (d->focused_view == d->scope_view);
    const bool focused_results = (results_view != nullptr && d->focused_view == results_view->current_view());

    const QList<QModelIndex> scope_selected = [&]() {
        QList<QModelIndex> out;

        const QList<QModelIndex> selected_proxy = d->scope_view->selectionModel()->selectedRows();
        for (const QModelIndex &index : selected_proxy) {
            const QModelIndex source_index = d->scope_proxy_model->mapToSource(index);
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

QModelIndex ConsoleWidget::get_selected_item() const {
    const QList<QModelIndex> selected_list = get_selected_items();

    return selected_list[0];
}

QList<QModelIndex> ConsoleWidget::search_items(const QModelIndex &parent, int role, const QVariant &value, const int type) const {
    const QList<QModelIndex> all_matches = [&]() {
        QList<QModelIndex> out;

        // NOTE: start index may be invalid if parent has no
        // children
        const QModelIndex start_index = d->model->index(0, 0, parent);
        if (start_index.isValid()) {
            const QList<QModelIndex> descendant_matches= d->model->match(start_index, role, value, -1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));
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

    state[CONSOLE_TREE_STATE] = d->toggle_console_tree_action->isChecked();
    state[DESCRIPTION_BAR_STATE] = d->toggle_description_bar_action->isChecked();

    for (const int type : d->results_descriptions.keys()) {
        const ResultsDescription results = d->results_descriptions[type];
        const QString results_name = results_state_name(type);
        const QVariant results_state = results.save_state();

        state[results_name] = results_state;
    }

    return QVariant(state);
}

void ConsoleWidget::restore_state(const QVariant &state_variant) {
    const QHash<QString, QVariant> state = state_variant.toHash();

    const QByteArray splitter_state = state.value(SPLITTER_STATE, QVariant()).toByteArray();
    d->splitter->restoreState(splitter_state);

    d->toggle_console_tree_action->setChecked(state[CONSOLE_TREE_STATE].toBool());
    d->on_toggle_console_tree();

    d->toggle_description_bar_action->setChecked(state[DESCRIPTION_BAR_STATE].toBool());
    d->on_toggle_description_bar();

    for (const int type : d->results_descriptions.keys()) {
        ResultsDescription results = d->results_descriptions[type];

        // NOTE: need to call this before restoring results
        // state because otherwise results view header can
        // have incorrect sections which breaks state
        // restoration
        d->model->setHorizontalHeaderLabels(results.column_labels());

        const QString results_name = results_state_name(type);
        const QVariant results_state = state.value(results_name, QVariant());

        results.restore_state(results_state);
    }
}

QAction *ConsoleWidget::get_refresh_action() const {
    return d->refresh_action;
}

void ConsoleWidget::delete_children(const QModelIndex &parent) {
    d->model->removeRows(0, d->model->rowCount(parent), parent);
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
        emit q->results_item_activated(index);
    }
}

// Show/hide actions based on what's selected and emit the
// signal
void ConsoleWidgetPrivate::on_action_menu_show() {
    action_menu->clear();

    const QList<QModelIndex> selected_list = q->get_selected_items();

    if (!selected_list.isEmpty() && !selected_list[0].isValid()) {
        return;
    }

    const bool single_selection = (selected_list.size() == 1);

    //
    // Custom actions
    //

    // TODO: duplicated, probably extract as a f-n
    const QSet<int> type_set = [&]() {
        QSet<int> out;

        for (const QModelIndex &index : selected_list) {
            const int type = index.data(ConsoleRole_Type).toInt();
            out.insert(type);
        } 

        return out;
    }();

    // First, add all possible custom actions
    const QList<QAction *> custom_action_list = [&]() {
        QList<QAction *> out;

        for (const int type : type_set) {
            ConsoleImpl *impl = impl_map[type];
            QList<QAction *> for_this_type = impl->get_all_custom_actions();

            for (QAction *action : for_this_type) {
                if (!out.contains(action)) {
                    out.append(action);
                }
            }
        }

        return out;
    }();

    for (QAction *action : custom_action_list) {
        action_menu->addAction(action);
    }

    action_menu->addSeparator();

    // Then show/hide custom actions
    const QSet<QAction *> visible_custom_action_set = [&]() {
        QSet<QAction *> out;

        for (int i = 0; i < selected_list.size(); i++) {
            const QModelIndex index = selected_list[i];

            ConsoleImpl *impl = get_impl(index);
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

            ConsoleImpl *impl = get_impl(index);
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

    // Set visibility state for custom actions
    for (QAction *action : custom_action_list) {
        const bool visible = visible_custom_action_set.contains(action);

        action->setVisible(visible);
    }

    // Set enabled state for custom actions
    for (QAction *action : custom_action_list) {
        const bool disabled = disabled_custom_action_set.contains(action);

        action->setDisabled(disabled);
    }

    //
    // Standard actions
    //

    // Add all actions first
    action_menu->addAction(standard_action_map[StandardAction_Copy]);
    action_menu->addAction(standard_action_map[StandardAction_Cut]);
    action_menu->addAction(standard_action_map[StandardAction_Rename]);
    action_menu->addAction(standard_action_map[StandardAction_Delete]);
    action_menu->addAction(standard_action_map[StandardAction_Paste]);
    action_menu->addAction(standard_action_map[StandardAction_Print]);
    action_menu->addSeparator();
    action_menu->addAction(standard_action_map[StandardAction_Refresh]);
    action_menu->addSeparator();
    action_menu->addAction(standard_action_map[StandardAction_Properties]);

    const QSet<StandardAction> visible_standard_actions = [&]() {
        QSet<StandardAction> out;

        for (int i = 0; i < selected_list.size(); i++) {
            const QModelIndex index = selected_list[i];

            ConsoleImpl *impl = get_impl(index);
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

            ConsoleImpl *impl = get_impl(index);
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

    // Set visibility state for standard actions
    for (const StandardAction &action_enum : standard_action_list) {
        const bool visible = visible_standard_actions.contains(action_enum);

        QAction *action = standard_action_map[action_enum];
        action->setVisible(visible);
    }

    // Set enabled state for standard actions
    for (const StandardAction &action_enum : standard_action_list) {
        const bool disabled = disabled_standard_actions.contains(action_enum);

        QAction *action = standard_action_map[action_enum];
        action->setDisabled(disabled);
    }

    // TODO: delete code below
    return;

    refresh_action->setVisible(false);

    refresh_action->setEnabled(true);

    if (selected_list.size() == 1) {
        // const QModelIndex selected = selected_list[0];

        // const bool is_dynamic = selected.data(ConsoleRole_ScopeIsDynamic).toBool();
        // const bool is_scope = selected.data(ConsoleRole_IsScope).toBool();
        // if (is_scope && is_dynamic) {
        //     refresh_action->setVisible(true);
        // }
    }

    emit q->actions_changed();
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

    const ResultsDescription results = get_current_results();

    // Switch to new parent for results view's model if view
    // exists
    const bool results_view_exists = (results.view() != nullptr);
    if (results_view_exists) {
        model->setHorizontalHeaderLabels(results.column_labels());

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

        results.view()->set_parent(current);

        if (need_dummy) {
            model->removeRow(0, current);
        }
    }

    // Switch to this item's results widget
    results_stacked_widget->setCurrentWidget(results.widget());

    update_navigation_actions();
    update_view_actions();

    fetch_scope(current);

    update_description();

    emit q->current_scope_item_changed(current);
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
    QAbstractItemView *new_focused_view = qobject_cast<QAbstractItemView *>(now);

    if (new_focused_view != nullptr) {
        const ResultsDescription current_results = get_current_results();
        QAbstractItemView *results_view = [=]() -> QAbstractItemView * {
            if (current_results.view() != nullptr) {
                return current_results.view()->current_view();
            } else {
                return nullptr;
            }
        }();

        if (new_focused_view == scope_view || new_focused_view == results_view) {
            focused_view = new_focused_view;
        }
    }
}

void ConsoleWidgetPrivate::refresh_current_scope() {
    const QModelIndex current_scope = q->get_current_scope_item();

    q->refresh_scope(current_scope);
}

void ConsoleWidgetPrivate::customize_columns() {
    const ResultsDescription current_results = get_current_results();
    ResultsView *results_view = current_results.view();
    if (results_view != nullptr) {
        auto dialog = new CustomizeColumnsDialog(results_view->detail_view(), current_results.default_columns(), q);
        dialog->open();
    }
}

// Set target to parent of current target
void ConsoleWidgetPrivate::navigate_up() {
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
void ConsoleWidgetPrivate::navigate_back() {
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

void ConsoleWidgetPrivate::navigate_forward() {
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

void ConsoleWidgetPrivate::set_results_to_icons() {
    set_results_to_type(ResultsViewType_Icons);
}

void ConsoleWidgetPrivate::set_results_to_list() {
    set_results_to_type(ResultsViewType_List);
}

void ConsoleWidgetPrivate::set_results_to_detail() {
    set_results_to_type(ResultsViewType_Detail);
}

void ConsoleWidgetPrivate::set_results_to_type(const ResultsViewType type) {
    const ResultsDescription results = get_current_results();

    if (results.view() != nullptr) {
        results.view()->set_view_type(type);
    }
}

void ConsoleWidgetPrivate::on_toggle_console_tree() {
    const bool visible = toggle_console_tree_action->isChecked();
    scope_view->setVisible(visible);
}

void ConsoleWidgetPrivate::on_toggle_description_bar() {
    const bool visible = toggle_description_bar_action->isChecked();
    description_bar->setVisible(visible);
}

// NOTE: as long as this is called where appropriate (on every target change), it is not necessary to do any condition checks in navigation f-ns since the actions that call them will be disabled if they can't be done
void ConsoleWidgetPrivate::update_navigation_actions() {
    const bool can_navigate_up = [this]() {
        const QModelIndex current = q->get_current_scope_item();
        const QModelIndex current_parent = current.parent();

        return (current.isValid() && current_parent.isValid());
    }();

    navigate_up_action->setEnabled(can_navigate_up);
    navigate_back_action->setEnabled(!targets_past.isEmpty());
    navigate_forward_action->setEnabled(!targets_future.isEmpty());
}

void ConsoleWidgetPrivate::update_view_actions() {
    const ResultsDescription results = get_current_results();
    const bool results_view_exists = (results.view() != nullptr);

    set_results_to_icons_action->setVisible(results_view_exists);
    set_results_to_list_action->setVisible(results_view_exists);
    set_results_to_detail_action->setVisible(results_view_exists);
    customize_columns_action->setVisible(results_view_exists);
}

void ConsoleWidget::add_actions(QMenu *action_menu, QMenu *view_menu, QMenu *preferences_menu, QToolBar *toolbar) {
    // Action
    d->refresh_action->setVisible(false);

    d->action_menu = action_menu;

    // Update actions right before menu opens
    connect(
        action_menu, &QMenu::aboutToShow,
        d, &ConsoleWidgetPrivate::on_action_menu_show);

    // Open action menu as context menu for central widget
    connect(
        d, &ConsoleWidgetPrivate::context_menu,
        [=](const QPoint pos) {
            const QModelIndex index = d->focused_view->indexAt(pos);

            if (index.isValid()) {
                const QPoint global_pos = d->focused_view->mapToGlobal(pos);

                action_menu->exec(global_pos);
            }
        });

    // View
    view_menu->addAction(d->set_results_to_icons_action);
    view_menu->addAction(d->set_results_to_list_action);
    view_menu->addAction(d->set_results_to_detail_action);

    view_menu->addSeparator();

    view_menu->addAction(d->customize_columns_action);
    
    preferences_menu->addAction(d->toggle_console_tree_action);
    preferences_menu->addAction(d->toggle_description_bar_action);

    // Toolbar
    toolbar->addAction(d->navigate_back_action);
    toolbar->addAction(d->navigate_forward_action);
    toolbar->addAction(d->navigate_up_action);
    toolbar->addSeparator();
    toolbar->addAction(d->refresh_current_scope_action);
}

const ResultsDescription ConsoleWidgetPrivate::get_current_results() const {
    const QModelIndex current_scope = q->get_current_scope_item();

    if (current_scope.isValid()) {
        const ResultsDescription results = get_results(current_scope);

        return results;
    } else {
        return ResultsDescription();
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

ConsoleImpl *ConsoleWidgetPrivate::get_impl(const QModelIndex &index) const {
    // NOTE: default type uses base class which will do nothing
    static ConsoleImpl *default_type = new ConsoleImpl(q);

    const int type = index.data(ConsoleRole_Type).toInt();
    ConsoleImpl *impl = impl_map.value(type, default_type);

    return impl;
}

ResultsDescription ConsoleWidgetPrivate::get_results(const QModelIndex &index) const {
    const int type = index.data(ConsoleRole_Type).toInt();

    if (results_descriptions.contains(type)) {
        return results_descriptions[type];
    } else {
        qDebug() << "No results were registered for type" << type;

        return default_results;
    }
}

void ConsoleWidgetPrivate::update_description() {
    const QModelIndex current_scope = q->get_current_scope_item();

    const QString scope_name = current_scope.data().toString();

    ConsoleImpl *impl = get_impl(current_scope);
    const QString description = impl->get_description(current_scope);

    description_bar_left->setText(scope_name);
    description_bar_right->setText(description);
}

void ConsoleWidgetPrivate::on_standard_action(const StandardAction action_enum) {
    const QList<QModelIndex> selected_list = q->get_selected_items();

    const QSet<int> type_set = [&]() {
        QSet<int> out;

        for (const QModelIndex &index : selected_list) {
            const int type = index.data(ConsoleRole_Type).toInt();
            out.insert(type);
        } 

        return out;
    }();

    // Call impl's action f-n for all present types
    for (const int type : type_set) {
        // Filter selected list so that it only contains indexes of this type
        const QList<QModelIndex> selected_of_type = [&]() {
            QList<QModelIndex> out;

            for (const QModelIndex &index : selected_list) {
                const int index_type = index.data(ConsoleRole_Type).toInt();
                if (index_type == type) {
                    out.append(index);
                }
            }

            return out;
        }();

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
