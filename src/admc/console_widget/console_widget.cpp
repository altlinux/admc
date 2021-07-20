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
#include "console_widget/scope_model.h"

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

    d->scope_model = new ConsoleDragModel(this);

    // NOTE: using a proxy model for scope to be able to do
    // case insensitive sorting
    d->scope_proxy_model = new ScopeModel(this);
    d->scope_proxy_model->setSourceModel(d->scope_model);
    d->scope_proxy_model->setSortCaseSensitivity(Qt::CaseInsensitive);

    d->scope_view->setModel(d->scope_proxy_model);

    d->focused_view = d->scope_view;

    d->description_bar = new QWidget();
    d->description_bar_left = new QLabel();
    d->description_bar_right = new QLabel();
    d->description_bar_left->setStyleSheet("font-weight: bold");

    d->results_stacked_widget = new QStackedWidget();

    d->properties_action = new QAction(tr("&Properties"), this);
    d->navigate_up_action = new QAction(tr("&Up one level"), this);
    d->navigate_back_action = new QAction(tr("&Back"), this);
    d->navigate_forward_action = new QAction(tr("&Forward"), this);
    d->refresh_action = new QAction(tr("&Refresh"), this);
    d->customize_columns_action = new QAction(tr("&Customize columns"), this);
    d->set_results_to_icons_action = new QAction(tr("&Icons"), this);
    d->set_results_to_list_action = new QAction(tr("&List"), this);
    d->set_results_to_detail_action = new QAction(tr("&Detail"), this);

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

    auto splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(d->scope_view);
    splitter->addWidget(results_wrapper);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);
    layout->addWidget(splitter);

    connect(
        d->scope_view, &QTreeView::expanded,
        d, &ConsoleWidgetPrivate::on_scope_expanded);
    connect(
        d->scope_view->selectionModel(), &QItemSelectionModel::currentChanged,
        d, &ConsoleWidgetPrivate::on_current_scope_item_changed);
    connect(
        d->scope_model, &QStandardItemModel::rowsAboutToBeRemoved,
        d, &ConsoleWidgetPrivate::on_scope_items_about_to_be_removed);
    connect(
        d->scope_model, &QAbstractItemModel::rowsInserted,
        this, &ConsoleWidget::results_count_changed);
    connect(
        d->scope_model, &QAbstractItemModel::rowsRemoved,
        this, &ConsoleWidget::results_count_changed);

    // TODO: for now properties are opened by user of console
    // widget but in the future it's planned to move this stuff
    // here, which will make this do more than just emit a
    // signal.
    connect(
        d->properties_action, &QAction::triggered,
        this, &ConsoleWidget::properties_requested);
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
        d->refresh_action, &QAction::triggered,
        d, &ConsoleWidgetPrivate::refresh);
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
        d->scope_view, &QWidget::customContextMenuRequested,
        this, &ConsoleWidget::context_menu);

    connect(
        qApp, &QApplication::focusChanged,
        d, &ConsoleWidgetPrivate::on_focus_changed);

    d->connect_to_drag_model(d->scope_model);

    d->update_navigation_actions();
    d->update_view_actions();
}

QStandardItem * ConsoleWidget::add_top_item(const int results_id, const ScopeNodeType scope_type) {
    auto item = new QStandardItem();
    d->scope_model->appendRow(item);

    item->setData(results_id, ConsoleRole_ResultsId);
    const bool is_dynamic = (scope_type == ScopeNodeType_Dynamic);
    item->setData(is_dynamic, ConsoleRole_ScopeIsDynamic);
    item->setData(false, ConsoleRole_WasFetched);
    item->setData(true, ConsoleRole_IsScope);

    return item;
}

QList<QStandardItem *> ConsoleWidget::add_scope_item(const int results_id, const ScopeNodeType scope_type, const QModelIndex &parent) {
    const QList<QStandardItem *> row = add_results_item(parent);

    row[0]->setData(results_id, ConsoleRole_ResultsId);

    const bool is_dynamic = (scope_type == ScopeNodeType_Dynamic);

    row[0]->setData(is_dynamic, ConsoleRole_ScopeIsDynamic);

    // NOTE: if item is not dynamic, then it's "fetched"
    // from creation
    if (is_dynamic) {
        row[0]->setData(false, ConsoleRole_WasFetched);
    } else {
        row[0]->setData(true, ConsoleRole_WasFetched);
    }

    row[0]->setData(true, ConsoleRole_IsScope);

    return row;
}

QList<QStandardItem *> ConsoleWidget::add_results_item(const QModelIndex &parent) {
    QStandardItem *parent_item = [&]() {
        if (parent.isValid()) {
            return d->scope_model->itemFromIndex(parent);
        } else {
            return d->scope_model->invisibleRootItem();
        }
    }();

    // Make item row
    const QList<QStandardItem *> row = [&]() {
        QList<QStandardItem *> out;
        
        const int column_count =
        [&]() {
            if (parent_item == d->scope_model->invisibleRootItem()) {
                return 1;
            } else {
                const int parent_results_id = parent.data(ConsoleRole_ResultsId).toInt();
                const ResultsDescription parent_results = d->results_descriptions[parent_results_id];

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

    parent_item->appendRow(row);

    return row;
}

void ConsoleWidget::set_has_properties(const QModelIndex &index, const bool has_properties) {
    QAbstractItemModel *model = (QAbstractItemModel *) index.model();
    model->setData(index, has_properties, ConsoleRole_HasProperties);
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

    d->scope_model->removeRows(index.row(), 1, index.parent());
}

void ConsoleWidget::set_current_scope(const QModelIndex &index) {
    const QModelIndex index_proxy = d->scope_proxy_model->mapFromSource(index);
    d->scope_view->selectionModel()->setCurrentIndex(index_proxy, QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);
}

void ConsoleWidget::refresh_scope(const QModelIndex &index) {
    if (!index.isValid()) {
        return;
    }

    d->scope_model->removeRows(0, d->scope_model->rowCount(index), index);

    // Emit item_fetched() so that user of console can
    // reload item's results
    emit item_fetched(index);
}

// No view, only widget
int ConsoleWidget::register_results(QWidget *widget) {
    return register_results(widget, nullptr, QList<QString>(), QList<int>());
}

// In this case, view *is* the widget
int ConsoleWidget::register_results(ResultsView *view, const QList<QString> &column_labels, const QList<int> &default_columns) {
    return register_results(view, view, column_labels, default_columns);
}

// Base register() f-n
int ConsoleWidget::register_results(QWidget *widget, ResultsView *view, const QList<QString> &column_labels, const QList<int> &default_columns) {
    static int id_max = 0;
    id_max++;
    const int id = id_max;

    d->results_descriptions[id] = ResultsDescription(widget, view, column_labels, default_columns);

    d->results_stacked_widget->addWidget(widget);

    if (view != nullptr) {
        view->set_model(d->scope_model);
        view->set_parent(get_current_scope_item());

        connect(
            view, &ResultsView::activated,
            d, &ConsoleWidgetPrivate::on_results_activated);
        connect(
            view, &ResultsView::context_menu,
            this, &ConsoleWidget::context_menu);

        // Hide non-default results view columns. Note that
        // at creation, view header doesnt have any
        // sections. Sections appear once a model is set.
        // Therefore we have to do this in the slot.
        QHeaderView *header = view->detail_view()->header();
        connect(
            header, &QHeaderView::sectionCountChanged,
            [header, default_columns, id](int oldCount, int) {
                for (int i = 0; i < header->count(); i++) {
                    const bool hidden = !default_columns.contains(i);
                    header->setSectionHidden(i, hidden);
                }
            });
    }

    return id;
}

void ConsoleWidget::set_description_bar_text(const QString &text) {
    const QString scope_name = [this]() {
        const QModelIndex current_scope = get_current_scope_item();
        const QString out = current_scope.data().toString();

        return out;
    }();

    d->description_bar_left->setText(scope_name);
    d->description_bar_right->setText(text);
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
        const QModelIndex start = get_item(parent)->child(0, 0)->index();
        QList<QModelIndex> out = d->scope_model->match(start, role, value, -1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));

        const QVariant parent_value = parent.data(role);
        const bool parent_is_match = (parent_value == value);

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

int ConsoleWidget::get_current_results_count() const {
    const QModelIndex current_scope = get_current_scope_item();

    return d->scope_model->rowCount(current_scope);
}

QStandardItem *ConsoleWidget::get_item(const QModelIndex &index) const {
    return d->scope_model->itemFromIndex(index);
}

QList<QStandardItem *> ConsoleWidget::get_row(const QModelIndex &index) const {
    QList<QStandardItem *> row;

    for (int col = 0; col < d->scope_model->columnCount(index.parent()); col++) {
        const QModelIndex sibling = index.siblingAtColumn(col);
        QStandardItem *item = d->scope_model->itemFromIndex(sibling);
        row.append(item);
    }

    return row;
}

bool ConsoleWidget::item_was_fetched(const QModelIndex &index) const {
    return index.data(ConsoleRole_WasFetched).toBool();
}

QWidget *ConsoleWidget::get_scope_view() const {
    return d->scope_view;
}

QWidget *ConsoleWidget::get_description_bar() const {
    return d->description_bar;
}

void ConsoleWidgetPrivate::connect_to_drag_model(ConsoleDragModel *model) {
    connect(
        model, &ConsoleDragModel::start_drag,
        this, &ConsoleWidgetPrivate::on_start_drag);
    connect(
        model, &ConsoleDragModel::can_drop,
        this, &ConsoleWidgetPrivate::on_can_drop);
    connect(
        model, &ConsoleDragModel::drop,
        this, &ConsoleWidgetPrivate::on_drop);
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
        emit q->properties_requested();
    }
}

// Show/hide actions based on what's selected and emit the
// signal
void ConsoleWidgetPrivate::update_actions() {
    const QList<QModelIndex> selected_list = q->get_selected_items();

    if (!selected_list.isEmpty() && !selected_list[0].isValid()) {
        return;
    }

    properties_action->setVisible(false);
    refresh_action->setVisible(false);

    properties_action->setEnabled(true);
    refresh_action->setEnabled(true);

    if (selected_list.size() == 1) {
        const QModelIndex selected = selected_list[0];

        const bool is_dynamic = selected.data(ConsoleRole_ScopeIsDynamic).toBool();
        if (is_dynamic) {
            refresh_action->setVisible(true);
        }

        const bool has_properties = selected.data(ConsoleRole_HasProperties).toBool();
        if (has_properties) {
            properties_action->setVisible(true);
        }
    } else if (selected_list.size() > 1) {
        const bool all_have_properties = [&]() {
            for (const QModelIndex &selected : selected_list) {
                const bool has_properties = selected.data(ConsoleRole_HasProperties).toBool();
                if (!has_properties) {
                    return false;
                }
            }

            return true;
        }();

        if (all_have_properties) {
            properties_action->setVisible(true);
        }
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
        scope_model->setHorizontalHeaderLabels(results.column_labels());

        // NOTE: setting horizontal labes may make columns
        // visible again in scope view, so re-hide them
        for (int i = 1; i < 100; i++) {
            scope_view->hideColumn(i);
        }

        // NOTE: need to add a dummy row if new current item
        // has no children because otherwise view header
        // will be hidden. Dummy row is removed later.
        const bool need_dummy = (scope_model->rowCount(current) == 0);
        if (need_dummy) {
            q->add_results_item(current);
        }

        results.view()->set_parent(current);

        if (need_dummy) {
            scope_model->removeRow(0, current);
        }
    }

    // Switch to this item's results widget
    results_stacked_widget->setCurrentWidget(results.widget());

    update_navigation_actions();
    update_view_actions();

    fetch_scope(current);

    emit q->current_scope_item_changed(current);
}

void ConsoleWidgetPrivate::on_scope_items_about_to_be_removed(const QModelIndex &parent, int first, int last) {
    const QList<QModelIndex> removed_scope_items = [=]() {
        QList<QModelIndex> out;

        QStack<QStandardItem *> stack;

        for (int r = first; r <= last; r++) {
            const QModelIndex removed_index = scope_model->index(r, 0, parent);
            auto removed_item = scope_model->itemFromIndex(removed_index);
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

void ConsoleWidgetPrivate::refresh() {
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

void ConsoleWidgetPrivate::on_start_drag(const QList<QPersistentModelIndex> &dropped_arg) {
    dropped = dropped_arg;
}

void ConsoleWidgetPrivate::on_can_drop(const QModelIndex &target, bool *ok) {
    emit q->items_can_drop(dropped, target, ok);
}

void ConsoleWidgetPrivate::on_drop(const QModelIndex &target) {
    emit q->items_dropped(dropped, target);
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

void ConsoleWidget::add_actions_to_action_menu(QMenu *menu) {
    menu->addAction(d->refresh_action);
    menu->addSeparator();
    menu->addAction(d->properties_action);

    d->properties_action->setVisible(false);
    d->refresh_action->setVisible(false);

    // Update actions right before menu opens
    connect(
        menu, &QMenu::aboutToShow,
        d, &ConsoleWidgetPrivate::update_actions);

    // Open action menu as context menu for central widget
    connect(
        this, &ConsoleWidget::context_menu,
        [=](const QPoint pos) {
            const QModelIndex index = d->focused_view->indexAt(pos);

            if (index.isValid()) {
                const QPoint global_pos = d->focused_view->mapToGlobal(pos);

                menu->exec(global_pos);
            }
        });
}

void ConsoleWidget::add_actions_to_navigation_menu(QMenu *menu) {
    menu->addAction(d->navigate_up_action);
    menu->addAction(d->navigate_back_action);
    menu->addAction(d->navigate_forward_action);
}

void ConsoleWidget::add_actions_to_view_menu(QMenu *menu) {
    menu->addAction(d->set_results_to_icons_action);
    menu->addAction(d->set_results_to_list_action);
    menu->addAction(d->set_results_to_detail_action);

    menu->addSeparator();

    menu->addAction(d->customize_columns_action);
}

const ResultsDescription ConsoleWidgetPrivate::get_current_results() const {
    const QModelIndex current_scope = q->get_current_scope_item();

    if (current_scope.isValid()) {
        const int results_id = current_scope.data(ConsoleRole_ResultsId).toInt();
        const ResultsDescription results = results_descriptions[results_id];

        return results;
    } else {
        return ResultsDescription();
    }
}

void ConsoleWidgetPrivate::fetch_scope(const QModelIndex &index) {
    const bool was_fetched = index.data(ConsoleRole_WasFetched).toBool();

    if (!was_fetched) {
        scope_model->setData(index, true, ConsoleRole_WasFetched);

        emit q->item_fetched(index);
    }
}
