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

#include "console_widget.h"

#include "console_drag_model.h"
#include "scope_model.h"
#include "customize_columns_dialog.h"

#include <QDebug>
#include <QTreeView>
#include <QVBoxLayout>
#include <QAction>
#include <QStack>
#include <QSplitter>
#include <QSortFilterProxyModel>
#include <QStackedWidget>
#include <QLabel>
#include <QApplication>
#include <QMenu>
#include <QHeaderView>
#include <QDialog>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QPushButton>
#include <QScrollArea>

ConsoleWidget::ConsoleWidget()
: QWidget()
{
    action_menu = new QMenu(this);
    navigation_menu = new QMenu(this);
    view_menu = new QMenu(this);

    refresh_action = new QAction(tr("&Refresh"), this);
    customize_columns_action = new QAction(tr("&Customize columns"), this);
    properties_action = new QAction(tr("&Properties"), this);
    navigate_up_action = new QAction(tr("&Up one level"), this);
    navigate_back_action = new QAction(tr("&Back"), this);
    navigate_forward_action = new QAction(tr("&Forward"), this);

    navigation_menu->addAction(navigate_up_action);
    navigation_menu->addAction(navigate_back_action);
    navigation_menu->addAction(navigate_forward_action);

    // TODO: how to allow user of scopewidget to implement their own drag logic?
    scope_model = new ScopeModel(this);

    scope_view = new QTreeView(this);
    scope_view->setHeaderHidden(true);
    scope_view->setExpandsOnDoubleClick(true);
    scope_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    scope_view->setContextMenuPolicy(Qt::CustomContextMenu);
    scope_view->setDragDropMode(QAbstractItemView::DragDrop);
    // NOTE: this makes it so that you can't drag drop between rows (even though name/description don't say anything about that)
    scope_view->setDragDropOverwriteMode(true);

    scope_view->setModel(scope_model);

    results_proxy_model = new QSortFilterProxyModel(this);

    results_stacked_widget = new QStackedWidget();

    description_bar = new QLabel();

    // NOTE: need results wrapper because layouts can't be inserted into splitter
    auto results_wrapper = new QWidget();
    auto results_layout = new QVBoxLayout();
    results_wrapper->setLayout(results_layout);
    results_layout->setContentsMargins(0, 0, 0, 0);
    results_layout->setSpacing(0);
    results_layout->addWidget(description_bar);
    results_layout->addWidget(results_stacked_widget);

    auto splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(scope_view);
    splitter->addWidget(results_wrapper);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);
    layout->addWidget(splitter);

    focused_view = scope_view;

    connect(
        scope_view, &QTreeView::expanded,
        this, &ConsoleWidget::fetch_scope);
    connect(
        scope_view->selectionModel(), &QItemSelectionModel::currentChanged,
        this, &ConsoleWidget::on_current_scope_item_changed);

    connect(
        refresh_action, &QAction::triggered,
        this, &ConsoleWidget::refresh);
    connect(
        customize_columns_action, &QAction::triggered,
        this, &ConsoleWidget::customize_columns);
    connect(
        properties_action, &QAction::triggered,
        this, &ConsoleWidget::properties);
    connect(
        navigate_up_action, &QAction::triggered,
        this, &ConsoleWidget::navigate_up);
    connect(
        navigate_back_action, &QAction::triggered,
        this, &ConsoleWidget::navigate_back);
    connect(
        navigate_forward_action, &QAction::triggered,
        this, &ConsoleWidget::navigate_forward);
    
    update_navigation_actions();

    connect(
        scope_model, &QStandardItemModel::rowsAboutToBeRemoved,
        this, &ConsoleWidget::on_scope_items_about_to_be_removed);

    connect(
        scope_view, &QWidget::customContextMenuRequested,
        this, &ConsoleWidget::open_action_menu_as_context_menu);
    connect(
        action_menu, &QMenu::aboutToShow,
        this, &ConsoleWidget::on_action_menu_show);
    connect(
        view_menu, &QMenu::aboutToShow,
        this, &ConsoleWidget::on_view_menu_show);

    connect(
        qApp, &QApplication::focusChanged,
        this, &ConsoleWidget::on_focus_changed);

    connect_to_drag_model(scope_model);
}

void ConsoleWidget::delete_item(const QModelIndex &index) {
    if (!index.isValid()) {
        return;
    }

    // NOTE: i *think* discarding const from model is fine.
    // Qt's reason: "A const pointer to the model is
    // returned because calls to non-const functions of the
    // model might invalidate the model index and possibly
    // crash your application.". Index becoming invalid is
    // expected since we're deleting it from the modelZZ.

    // Remove buddy item
    const QModelIndex buddy = index.data(ConsoleRole_Buddy).toModelIndex();
    if (buddy.isValid()) {
        ((QAbstractItemModel *)buddy.model())->removeRows(buddy.row(), 1, buddy.parent());
    }

    // Remove item from it's own model
    ((QAbstractItemModel *)index.model())->removeRows(index.row(), 1, index.parent());
}

QStandardItem *ConsoleWidget::add_scope_item(const int results_id, const QModelIndex &parent) {
    QStandardItem *parent_item =
    [=]() {
        if (parent.isValid()) {
            return scope_model->itemFromIndex(parent);
        } else {
            return scope_model->invisibleRootItem();
        }
    }();

    auto item = new QStandardItem();
    item->setData(false, ConsoleRole_WasFetched);

    item->setData(results_id, ConsoleRole_ResultsId);

    item->setData(true, ConsoleRole_IsScope);

    parent_item->appendRow(item);

    // Create a results model for this scope item
    auto new_results = new ConsoleDragModel(this);
    connect_to_drag_model(new_results);
    results_models[item->index()] = new_results;

    const ResultsDescription results = results_descriptions[results_id];
    const QList<QString> results_column_labels = results.get_column_labels();
    new_results->setHorizontalHeaderLabels(results_column_labels);

    return item;
}

void ConsoleWidget::set_current_scope(const QModelIndex &index) {
    scope_view->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);
}

void ConsoleWidget::refresh_scope(const QModelIndex &index) {
    if (!index.isValid()) {
        return;
    }

    scope_model->removeRows(0, scope_model->rowCount(index), index);

    QStandardItemModel *results_model = get_results_for_scope_item(index);
    results_model->removeRows(0, results_model->rowCount());

    // Emit item_fetched() so that user of console can
    // reload item's results
    emit item_fetched(index);
}

int ConsoleWidget::register_results_view(QTreeView *view, const QList<QString> &column_labels, const QList<int> &default_columns) {
    static int id_max = 0;
    id_max++;
    const int id = id_max;

    results_descriptions[id] = ResultsDescription(view, column_labels, default_columns);

    // TODO: check if also need to call adjustSize() when
    // changing between different results widgets.

    // NOTE: necessary to adjust size after adding widget
    // for correct sizing, idk why. Seems like a Qt
    // bug/quirk. Without this qstackwidget is too small in
    // the splitter.
    results_stacked_widget->addWidget(view);
    results_stacked_widget->adjustSize();

    // Hide non-default results view columns
    QHeaderView *header = view->header();
    for (int i = 0; i < header->count(); i++) {
        const bool hidden = !default_columns.contains(i);
        header->setSectionHidden(i, hidden);
    }

    connect(
        view, &QTreeView::activated,
        this, &ConsoleWidget::on_results_activated);
    connect(
        view, &QWidget::customContextMenuRequested,
        this, &ConsoleWidget::open_action_menu_as_context_menu);

    return id;
}

QList<QStandardItem *> ConsoleWidget::add_results_row(const QModelIndex &buddy, const QModelIndex &scope_parent) {
    if (!scope_parent.isValid()) {
        return QList<QStandardItem *>();
    }

    const QList<QStandardItem *> row =
    [=]() {
        QList<QStandardItem *> out;

        const int results_id = scope_parent.data(ConsoleRole_ResultsId).toInt();
        const ResultsDescription results = results_descriptions[results_id];
        const int column_count = results.get_column_count();

        for (int i = 0; i < column_count; i++) {
            const auto item = new QStandardItem();
            out.append(item);
        }

        return out;
    }();

    QStandardItemModel *results_model = get_results_for_scope_item(scope_parent);
    results_model->appendRow(row);

    // Set buddy data role for both this results item and
    // the buddy scope item
    if (buddy.isValid()) {
        const QModelIndex results_index = row[0]->index();

        // NOTE: MUST use QPersistentModelIndex because
        // QModelIndex's will become incorrect quickly
        results_model->setData(results_index, QPersistentModelIndex(buddy), ConsoleRole_Buddy);
        scope_model->setData(buddy, QPersistentModelIndex(results_index), ConsoleRole_Buddy);
    }

    row[0]->setData(false, ConsoleRole_IsScope);

    row[0]->setData(scope_parent, ConsoleRole_ScopeParent);

    return row;
}

void ConsoleWidget::sort_scope() {
    scope_model->sort(0, Qt::AscendingOrder);
}

void ConsoleWidget::set_description_bar_text(const QString &text) {
    description_bar->setText(text);
}

QList<QModelIndex> ConsoleWidget::get_selected_items() const {
    const QList<QModelIndex> all_indexes = focused_view->selectionModel()->selectedIndexes();

    QList<QModelIndex> indexes;

    for (const QModelIndex index : all_indexes) {
        // Need first column to access item data
        if (index.column() == 0) {
            indexes.append(index);
        }
    }

    return indexes;
}

QList<QModelIndex> ConsoleWidget::search_scope_by_role(int role, const QVariant &value) const {
    const QList<QModelIndex> matches = scope_model->match(scope_model->index(0, 0), role, value, -1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));

    return matches;
}

QModelIndex ConsoleWidget::get_current_scope_item() const {
    const QModelIndex index = scope_view->selectionModel()->currentIndex();

    return index;
}

int ConsoleWidget::get_current_results_count() const {
    const QModelIndex current_scope = get_current_scope_item();
    QStandardItemModel *current_results = get_results_for_scope_item(current_scope);
    const int results_count = current_results->rowCount();

    return results_count;
}

QStandardItem *ConsoleWidget::get_scope_item(const QModelIndex &scope_index) const {
    QStandardItem *item = scope_model->itemFromIndex(scope_index);

    return item;
}

QList<QStandardItem *> ConsoleWidget::get_results_row(const QModelIndex &results_index) const {
    QList<QStandardItem *> row;

    const QModelIndex scope_parent = results_index.data(ConsoleRole_ScopeParent).toModelIndex();
    QStandardItemModel *model = get_results_for_scope_item(scope_parent);

    for (int col = 0; col < model->columnCount(); col++) {
        QStandardItem *item = model->item(results_index.row(), col);
        row.append(item);
    }

    return row;
}

QTreeView *ConsoleWidget::get_scope_view() const {
    return scope_view;
}

QLabel *ConsoleWidget::get_description_bar() const {
    return description_bar;
}

QMenu *ConsoleWidget::get_action_menu() const {
    return action_menu;
}

QMenu *ConsoleWidget::get_navigation_menu() const {
    return navigation_menu;
}

QMenu *ConsoleWidget::get_view_menu() const {
    return view_menu;
}

void ConsoleWidget::on_current_scope_item_changed(const QModelIndex &current, const QModelIndex &previous) {
    // NOTE: technically this slot should never be called
    // with invalid current index
    if (!current.isValid()) {
        return;
    }

    // Move current to past, if current changed and is valid
    if (previous.isValid() && (previous != current)) {
        targets_past.append(QPersistentModelIndex(previous));
    }

    // When a new current is selected, a new future begins
    targets_future.clear();

    update_navigation_actions();

    // Switch to this item's results view
    const int results_id = current.data(ConsoleRole_ResultsId).toInt();
    const ResultsDescription results_description = results_descriptions[results_id];
    QTreeView *results_view = results_description.get_view();
    results_stacked_widget->setCurrentWidget(results_view);

    // Switch to this item's results model
    QStandardItemModel *results = get_results_for_scope_item(current);
    results_proxy_model->setSourceModel(results);
    results_view->setModel(results_proxy_model);

    // NOTE: technically (selection != expansion) but for our
    // purposes we consider it to be the same.
    fetch_scope(current);

    emit current_scope_item_changed(current);
}

void ConsoleWidget::on_scope_items_about_to_be_removed(const QModelIndex &parent, int first, int last) {
    const QList<QModelIndex> removed_scope_items =
    [=]() {
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

    // Remove and delete results models associated with
    // removed scope items
    for (const QModelIndex &index : removed_scope_items) {
        if (results_models.contains(index)) {
            QStandardItemModel *results = results_models.take(index);
            delete results;
        }
    }
}

void ConsoleWidget::on_results_activated(const QModelIndex &index) {
    const QModelIndex buddy = index.data(ConsoleRole_Buddy).toModelIndex();

    if (buddy.isValid()) {
        // Set associated scope item as current
        const QModelIndex scope_index = index.data(ConsoleRole_Buddy).toModelIndex();

        if (scope_index.isValid()) {
            scope_view->selectionModel()->setCurrentIndex(scope_index, QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);
        }
    } else {
        properties();
    }
}

// NOTE: this is the workaround required to know in which pane selected objects are located
void ConsoleWidget::on_focus_changed(QWidget *old, QWidget *now) {
    auto current_results_view = results_stacked_widget->currentWidget();
    const QList<QWidget *> views = {
        scope_view, current_results_view
    };

    for (QWidget *view : views) {
        QTreeView *view_casted = qobject_cast<QTreeView *>(view);

        if (now == view && view_casted != nullptr) {
            focused_view = view_casted;
        }
    }
}

void ConsoleWidget::open_action_menu_as_context_menu(const QPoint pos) {
    auto menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    add_actions_to_action_menu(menu);

    emit action_menu_about_to_open(menu, focused_view);

    const QPoint global_pos = focused_view->mapToGlobal(pos);
    menu->exec(global_pos);
}

void ConsoleWidget::on_action_menu_show() {
    add_actions_to_action_menu(action_menu);

    emit action_menu_about_to_open(action_menu, focused_view);
}

void ConsoleWidget::on_view_menu_show() {
    view_menu->clear();

    // NOTE: currently all results views are tree views, but
    // do this anyway for the future
    QWidget *current_results_widget = results_stacked_widget->currentWidget();
    QTreeView *current_results_view = qobject_cast<QTreeView *>(current_results_widget);
    if (current_results_view != nullptr) {
        view_menu->addAction(customize_columns_action);
    }
    
    emit view_menu_about_to_open(view_menu);
}

void ConsoleWidget::refresh() {
    const QModelIndex current_index = focused_view->selectionModel()->currentIndex();

    refresh_scope(current_index);
}

void ConsoleWidget::customize_columns() {
    const QModelIndex current_scope = get_current_scope_item();

    if (!current_scope.isValid()) {
        return;
    }

    const int current_results_id = current_scope.data(ConsoleRole_ResultsId).toInt();
    const ResultsDescription current_results = results_descriptions[current_results_id];

    auto dialog = new CustomizeColumnsDialog(current_results, this);
    dialog->open();
}

// TODO: for now properties are opened by user of console
// widget but in the future it's planned to move this stuff
// here, which will make this do more than just emit a
// signal.
void ConsoleWidget::properties() {
    emit properties_requested();
}

// Set target to parent of current target
void ConsoleWidget::navigate_up() {
    const QPersistentModelIndex old_current = QPersistentModelIndex(scope_view->currentIndex());

    if (!old_current.isValid()) {
        return;
    }

    const QModelIndex new_current = old_current.parent();

    // NOTE: parent of target can be invalid, if current is
    // head item in which case we've reached top of tree and
    // can't go up higher
    const bool can_go_up = (!new_current.isValid());
    if (!can_go_up) {
        scope_view->selectionModel()->setCurrentIndex(new_current, QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);
    }
}

// NOTE: for "back" and "forward" navigation, setCurrentIndex() triggers "current changed" slot which by default erases future history, so manually restore correct navigation state afterwards
void ConsoleWidget::navigate_back() {
    const QModelIndex old_current = scope_view->currentIndex();

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
    scope_view->selectionModel()->setCurrentIndex(new_current, QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);

    targets_past = saved_past;
    targets_future = saved_future;

    targets_past.removeLast();
    targets_future.prepend(old_current);

    update_navigation_actions();
}

void ConsoleWidget::navigate_forward() {
    const QModelIndex old_current = scope_view->currentIndex();
    
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
    scope_view->selectionModel()->setCurrentIndex(new_current, QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);

    targets_past = saved_past;
    targets_future = saved_future;

    targets_past.append(old_current);
    targets_future.removeFirst();

    update_navigation_actions();
}

void ConsoleWidget::on_start_drag(const QList<QModelIndex> &dropped_arg) {
    dropped = dropped_arg;
}

void ConsoleWidget::on_can_drop(const QModelIndex &target, bool *ok) {
    emit items_can_drop(dropped, target, ok);
}

void ConsoleWidget::on_drop(const QModelIndex &target) {
    emit items_dropped(dropped, target);
}

// NOTE: as long as this is called where appropriate (on every target change), it is not necessary to do any condition checks in navigation f-ns since the actions that call them will be disabled if they can't be done
void ConsoleWidget::update_navigation_actions() {
    navigate_back_action->setEnabled(!targets_past.isEmpty());
    navigate_forward_action->setEnabled(!targets_future.isEmpty());
}

void ConsoleWidget::add_actions_to_action_menu(QMenu *menu) {
    menu->clear();

    if (focused_view == scope_view) {
        menu->addAction(refresh_action);
    }

    menu->addAction(properties_action);
}

void ConsoleWidget::connect_to_drag_model(ConsoleDragModel *model) {
    connect(
        model, &ConsoleDragModel::start_drag,
        this, &ConsoleWidget::on_start_drag);
    connect(
        model, &ConsoleDragModel::can_drop,
        this, &ConsoleWidget::on_can_drop);
    connect(
        model, &ConsoleDragModel::drop,
        this, &ConsoleWidget::on_drop);
}

QStandardItemModel *ConsoleWidget::get_results_for_scope_item(const QModelIndex &index) const {
    if (results_models.contains(index)) {
        return results_models[index];
    } else {
        qDebug() << "get_results_for_scope_item() called with invalid index! Creating an empty model to avoid crash.";

        return new ConsoleDragModel();
    }
}

void ConsoleWidget::fetch_scope(const QModelIndex &index) {
    const bool was_fetched = index.data(ConsoleRole_WasFetched).toBool();

    if (!was_fetched) {
        scope_model->setData(index, true, ConsoleRole_WasFetched);

        emit item_fetched(index);
    }
}
