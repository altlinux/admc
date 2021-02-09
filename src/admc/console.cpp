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

#include "console.h"

#include "object_model.h"
#include "object_menu.h"
#include "utils.h"
#include "ad_utils.h"
#include "properties_dialog.h"
#include "ad_config.h"
#include "ad_interface.h"
#include "ad_object.h"
#include "filter.h"
#include "settings.h"
#include "filter_dialog.h"
#include "filter_widget/filter_widget.h"
#include "object_menu.h"
#include "console_drag_model.h"
#include "menubar.h"

#include <QTreeView>
#include <QVBoxLayout>
#include <QSplitter>
#include <QDebug>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QApplication>
#include <QTreeWidget>
#include <QStack>
#include <QMenu>
#include <QLabel>

enum ScopeRole {
    ScopeRole_Id = Role_ObjectClass + 1,
    ScopeRole_Fetched = Role_ObjectClass + 2,
};

#define DUMMY_ITEM_ID -1

// TODO: currently showing refresh action always for all scope nodes. Could show it in results if node is in scope? Also could not show it if results isn't loaded?

bool object_should_be_in_scope(const QString &object_class);

QHash<int, QStandardItemModel *> scope_id_to_results;

Console::Console(MenuBar *menubar_arg)
: QWidget()
{
    menubar = menubar_arg;

    scope_model = new ConsoleDragModel(0, 1, this);

    scope_view = new QTreeView(this);
    scope_view->setHeaderHidden(true);
    scope_view->setExpandsOnDoubleClick(true);
    scope_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    scope_view->setContextMenuPolicy(Qt::CustomContextMenu);
    scope_view->setDragDropMode(QAbstractItemView::DragDrop);
    scope_view->setSortingEnabled(true);
    // NOTE: this makes it so that you can't drag drop between rows (even though name/description don't say anything about that)
    scope_view->setDragDropOverwriteMode(true);

    scope_view->setModel(scope_model);

    results_view = new QTreeView(this);
    results_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    results_view->header()->setSectionsMovable(true);
    results_view->setContextMenuPolicy(Qt::CustomContextMenu);
    results_view->setDragDropMode(QAbstractItemView::DragDrop);
    results_view->setSortingEnabled(true);
    results_view->sortByColumn(0, Qt::AscendingOrder);
    results_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    results_view->setDragDropOverwriteMode(true);

    SETTINGS()->setup_header_state(results_view->header(), VariantSetting_ResultsHeader);
    
    auto results_wrapper = new QWidget();

    results_header = new QWidget();

    results_header_label = new QLabel();

    auto header_layout = new QVBoxLayout();
    results_header->setLayout(header_layout);
    header_layout->setContentsMargins(0, 0, 0, 0);
    header_layout->setSpacing(0);
    header_layout->addWidget(results_header_label);

    auto results_layout = new QVBoxLayout();
    results_wrapper->setLayout(results_layout);
    results_layout->setContentsMargins(0, 0, 0, 0);
    results_layout->setSpacing(0);
    results_layout->addWidget(results_header);
    results_layout->addWidget(results_view);

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

    filter_dialog = new FilterDialog(this);

    connect(
        scope_view->selectionModel(), &QItemSelectionModel::currentChanged,
        this, &Console::on_current_scope_changed);

    // TODO: not sure about this... Need to re-sort when items are renamed and maybe in other cases as well.
    connect(
        scope_model, &QAbstractItemModel::rowsInserted,
        [this]() {
            scope_view->sortByColumn(0, Qt::AscendingOrder);
        });

    connect(
        scope_view, &QTreeView::expanded,
        [=](const QModelIndex &index) {
            const bool fetched = index.data(ScopeRole_Fetched).toBool();
            if (!fetched) {
                fetch_scope_node(index);
            }
        });

    connect(
        qApp, &QApplication::focusChanged,
        this, &Console::on_focus_changed);

    connect(
        scope_view, &QWidget::customContextMenuRequested,
        this, &Console::open_context_menu);
    connect(
        results_view, &QWidget::customContextMenuRequested,
        this, &Console::open_context_menu);

    connect(
        results_view, &QTreeView::doubleClicked,
        this, &Console::on_result_item_double_clicked);

    connect(
        AD(), &AdInterface::object_added,
        this, &Console::on_object_added);
    connect(
        AD(), &AdInterface::object_deleted,
        this, &Console::on_object_deleted);
    connect(
        AD(), &AdInterface::object_changed,
        this, &Console::on_object_changed);

    connect(
        scope_model, &QStandardItemModel::rowsAboutToBeRemoved,
        this, &Console::on_scope_rows_about_to_be_removed);

    focused_view = scope_view;

    // Refresh head when settings affecting the filter
    // change. This reloads the model with an updated filter
    const BoolSettingSignal *advanced_view = SETTINGS()->get_bool_signal(BoolSetting_AdvancedView);
    connect(
        advanced_view, &BoolSettingSignal::changed,
        this, &Console::refresh_head);

    const BoolSettingSignal *show_non_containers = SETTINGS()->get_bool_signal(BoolSetting_ShowNonContainersInConsoleTree);
    connect(
        show_non_containers, &BoolSettingSignal::changed,
        this, &Console::refresh_head);

    connect(
        filter_dialog, &QDialog::accepted,
        this, &Console::refresh_head);

    connect(
        menubar->up_one_level_action, &QAction::triggered,
        this, &Console::navigate_up);
    connect(
        menubar->back_action, &QAction::triggered,
        this, &Console::navigate_back);
    connect(
        menubar->forward_action, &QAction::triggered,
        this, &Console::navigate_forward);

    connect(
        menubar->action_menu, &QMenu::aboutToShow,
        [this]() {
            load_menu(menubar->action_menu);
        });

    const BoolSettingSignal *dev_mode_signal = SETTINGS()->get_bool_signal(BoolSetting_DevMode);
    connect(
        dev_mode_signal, &BoolSettingSignal::changed,
        this, &Console::refresh_head);

    SETTINGS()->connect_toggle_widget(scope_view, BoolSetting_ShowConsoleTree);
    SETTINGS()->connect_toggle_widget(results_header, BoolSetting_ShowResultsHeader);

    // Load head object
    const QString head_dn = AD()->domain_head();
    const AdObject head_object = AD()->search_object(head_dn);
    auto head_item = make_scope_item(head_object);
    scope_model->appendRow(head_item);

    // Make head object current
    scope_view->selectionModel()->setCurrentIndex(scope_model->index(0, 0), QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);
}

void Console::refresh_head() {
    fetch_scope_node(scope_model->index(0, 0));
}

// When scope nodes are removed, need to delete data associate to them
// So delete scope node's results
// and delete scope node from navigation history, if it's there
void Console::on_scope_rows_about_to_be_removed(const QModelIndex &parent, int first, int last) {
    QStack<QStandardItem *> stack;

    for (int r = first; r <= last; r++) {
        const QModelIndex removed_index = scope_model->index(r, 0, parent);
        auto removed_item = scope_model->itemFromIndex(removed_index);
        stack.push(removed_item);
    }

    while (!stack.isEmpty()) {
        auto item = stack.pop();

        const int id = item->data(ScopeRole_Id).toInt();

        // NOTE: need to avoid processing dummy nodes!
        if (id == DUMMY_ITEM_ID) {
            continue;
        }

        // Remove scope node from navigation history (if it's there)
        targets_past.removeAll(id);
        targets_future.removeAll(id);

        // Remove results from hashmap and delete it
        if (scope_id_to_results.contains(id)) {
            QStandardItemModel *results = scope_id_to_results.take(id);
            delete results;
        }

        // Iterate through children
        for (int r = 0; r < item->rowCount(); r++) {
            auto child = item->child(r, 0);
            stack.push(child);
        }
    }

    // Update navigation since a node in history could've been removed
    update_navigation_actions();
}

// NOTE: this is the workaround required to know in which pane selected objects are located
void Console::on_focus_changed(QWidget *old, QWidget *now) {
    const QList<QTreeView *> views = {
        scope_view, results_view
    };
    for (auto view : views) {
        if (view == now) {
            focused_view = view;
            
            return;
        }
    }
}

void Console::on_current_scope_changed(const QModelIndex &current, const QModelIndex &) {
    if (!current.isValid()) {
        return;
    }

    // Fetch if needed
    const bool fetched = current.data(ScopeRole_Fetched).toBool();
    if (!fetched) {
        fetch_scope_node(current);
    }

    const int id = current.data(ScopeRole_Id).toInt();

    QStandardItemModel *results_model = scope_id_to_results[id];
    results_view->setModel(results_model);

    // Update header with new object counts when rows are added/removed
    connect(
        results_model, &QAbstractItemModel::rowsInserted,
        this, &Console::update_results_header);
    connect(
        results_model, &QAbstractItemModel::rowsRemoved,
        this, &Console::update_results_header);
    update_results_header();

    // Update navigation history
    // NOTE: by default, this handles the case where current changed due to user selecting a different node in the tree. So erase future history. For cases where current changed due to navigation browsing through history, the navigation f-ns will set correct navigation state after this slot is called.

    // Move current to past, if it is valid
    // NOTE: current target may be invalid, for example when whole model is refreshed or at startup
    const QModelIndex old_target_index = get_scope_node_from_id(targets_current);
    const bool old_target_is_valid = (old_target_index.isValid() && old_target_index != current);
    if (old_target_is_valid) {
        targets_past.append(targets_current);
    }

    targets_future.clear();

    targets_current = id;

    update_navigation_actions();
}

// NOTE: responding to object changes/additions/deletions only in object part of the scope tree. Queries are left unupdated.

void Console::on_object_deleted(const QString &dn) {
    const QString parent_dn = dn_get_parent(dn);

    const QList<QModelIndex> scope_parent_matches = scope_model->match(scope_model->index(0, 0), Role_DN, parent_dn, 1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));
    if (scope_parent_matches.isEmpty()) {
        return;
    }

    const QModelIndex scope_parent = scope_parent_matches[0];

    // Remove from results first(need object to still be in
    // scope tree to do this)
    const int scope_parent_id = scope_parent.data(ScopeRole_Id).toInt();
    QStandardItemModel *results_model = scope_id_to_results.value(scope_parent_id, nullptr);
    if (results_model != nullptr) {
        const QList<QModelIndex> results_index_matches = results_model->match(scope_model->index(0, 0), Role_DN, dn, 1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));

        if (!results_index_matches.isEmpty()) {
            const QModelIndex results_index = results_index_matches[0];
            results_model->removeRow(results_index.row(), results_index.parent());
        }
    }

    // Remove from scope
    const QList<QModelIndex> scope_index_matches = scope_model->match(scope_parent, Role_DN, dn, 1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));
    if (!scope_index_matches.isEmpty()) {
        const QModelIndex scope_index = scope_index_matches[0];
        scope_model->removeRow(scope_index.row(), scope_index.parent());
    }
}

void Console::on_object_added(const QString &dn) {
    // Find parent of object in scope tree (if it exists)
    const QModelIndex scope_parent =
    [=]() {
        const QString parent_dn = dn_get_parent(dn);
        const QList<QModelIndex> scope_parent_matches = scope_model->match(scope_model->index(0, 0), Role_DN, parent_dn, 1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));
        
        if (scope_parent_matches.isEmpty()) {
            return scope_parent_matches[0];
        } else {
            return QModelIndex();
        }
    }();

    const bool parent_not_loaded = (!scope_parent.isValid());
    if (parent_not_loaded) {
        return;
    }

    // NOTE: only need to add object to console if parent was fetched already. If parent wasn't fetched, then this new object will be added when parent is fetched.
    const bool parent_was_fetched = scope_parent.data(ScopeRole_Fetched).toBool();
    if (!parent_was_fetched) {
        return;
    }

    const AdObject object = AD()->search_object(dn);
    const QString object_class = object.get_string(ATTRIBUTE_OBJECT_CLASS);

    // Add object to scope
    const bool should_be_in_scope = object_should_be_in_scope(object_class);
    if (should_be_in_scope) {
        QStandardItem *parent_item = scope_model->itemFromIndex(scope_parent);
        auto object_item = make_scope_item(object);
        parent_item->appendRow(object_item);
    }

    // Add object to results
    const int scope_parent_id = scope_parent.data(ScopeRole_Id).toInt();
    QStandardItemModel *results_model = scope_id_to_results.value(scope_parent_id, nullptr);
    if (results_model != nullptr) {
        make_results_row(results_model, object);
    }
}

// Update object in results by reloading it's row with
// updated attributes NOTE: only updating object in results.
// Attribute changes don't matter to scope because it
// doesn't display any attributes, so only need to update
// results.
void Console::on_object_changed(const QString &dn) {
    // Find parent of this object in scope tree
    const QString parent_dn = dn_get_parent(dn);
    const QList<QModelIndex> scope_parent_matches = scope_model->match(scope_model->index(0, 0), Role_DN, parent_dn, 1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive | Qt::MatchWrap));
    if (scope_parent_matches.isEmpty()) {
        return;
    }

    // Get results model attached to parent
    const QModelIndex scope_parent = scope_parent_matches[0];
    const int scope_parent_id = scope_parent.data(ScopeRole_Id).toInt();
    QStandardItemModel *results_model = scope_id_to_results.value(scope_parent_id, nullptr);
    if (results_model == nullptr) {
        return;
    }

    // Find object's row in results model
    const QList<QModelIndex> results_index_matches = results_model->match(scope_parent, Role_DN, dn, 1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));
    if (results_index_matches.isEmpty()) {
        return;
    }

    // Update object's row
    const QModelIndex results_index = results_index_matches[0];

    const QList<QStandardItem *> item_row =
    [=]() {
        QList<QStandardItem *> out;

        for (int c = 0; c < ADCONFIG()->get_columns().size(); c++) {
            const QModelIndex sibling_index = results_index.siblingAtColumn(c);
            QStandardItem *sibling_item = results_model->itemFromIndex(sibling_index);

            out.append(sibling_item);
        }

        return out;
    }();

    const AdObject object = AD()->search_object(dn);

    load_results_row(item_row, object);
}

// Set target to parent of current target
void Console::navigate_up() {
    const QModelIndex current_index = get_scope_node_from_id(targets_current);
    const QModelIndex new_target_index = current_index.parent();

    // NOTE: parent of target can be invalid, if current is
    // head node
    if (new_target_index.isValid()) {
        scope_view->selectionModel()->setCurrentIndex(new_target_index, QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);
    }
}

// NOTE: for "back" and "forward" navigation, setCurrentIndex() triggers "current changed" slot which by default erases future history, so manually restore correct navigation state afterwards
void Console::navigate_back() {
    targets_future.prepend(targets_current);
    auto new_current = targets_past.takeLast();

    auto saved_past = targets_past;
    auto saved_future = targets_future;

    const QModelIndex new_current_index = get_scope_node_from_id(new_current);
    scope_view->selectionModel()->setCurrentIndex(new_current_index, QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);

    targets_future = saved_future;
    targets_past = saved_past;
    targets_current = new_current;

    update_navigation_actions();
}

void Console::navigate_forward() {
    targets_past.append(targets_current);
    auto new_current = targets_future.takeFirst();

    auto saved_past = targets_past;
    auto saved_future = targets_future;

    const QModelIndex new_current_index = get_scope_node_from_id(new_current);
    scope_view->selectionModel()->setCurrentIndex(new_current_index, QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);

    targets_future = saved_future;
    targets_past = saved_past;
    targets_current = new_current;

    update_navigation_actions();
}

// TODO: currently calling this when current scope changes, when object is deleted and when object is added. Might be some other weird unhandled cases?
void Console::update_results_header() {
    const QString label_text =
    [this]() {
        const QAbstractItemModel *results_model = results_view->model();
        if (results_model == nullptr) {
            return QString();
        }
        const int object_count = results_model->rowCount();
        const QString object_count_string = tr("%n object(s)", "", object_count);

        const QModelIndex parent_index = scope_view->selectionModel()->currentIndex();
        if (!parent_index.isValid()) {
            return QString();
        }
        const QString parent_dn = parent_index.data(Role_DN).toString();
        const QString parent_name = dn_get_name(parent_dn);

        return QString("%1: %2").arg(parent_name, object_count_string);
    }();

    results_header_label->setText(label_text);
}

void Console::load_menu(QMenu *menu) {
    menu->clear();

    QAction *insert_before_action = add_object_actions_to_menu(menu, focused_view, this, true);

    // Add refresh action if clicked on a fetched scope node
    if (focused_view == scope_view) {
        const QModelIndex index = scope_view->selectionModel()->currentIndex();
        const bool was_fetched = index.data(ScopeRole_Fetched).toBool();

        if (was_fetched) {
            QAction *refresh = menu->addAction(tr("Refresh"),
                [=]() {
                    const QModelIndex current_index = scope_view->selectionModel()->currentIndex();
                    fetch_scope_node(current_index);
                });

            menu->removeAction(refresh);
            menu->insertAction(insert_before_action, refresh);
        }
    }
}

void Console::open_context_menu(const QPoint pos) {
    auto menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    load_menu(menu);
    exec_menu_from_view(menu, focused_view, pos);
}

void Console::load_results_row(QList<QStandardItem *> row, const AdObject &object) {
    load_object_row(row, object);
}

void Console::make_results_row(QStandardItemModel * model, const AdObject &object) {
    const QList<QStandardItem *> row = make_item_row(ADCONFIG()->get_columns().size());
    load_results_row(row, object);
    model->appendRow(row);
}

// Load children of this item in scope tree
// and load results linked to this scope item
void Console::fetch_scope_node(const QModelIndex &index) {
    show_busy_indicator();

    // NOTE: remove old scope children (which might be a dummy child used for showing child indicator)
    scope_model->removeRows(0, scope_model->rowCount(index), index);

    const bool dev_mode = SETTINGS()->get_bool(BoolSetting_DevMode);

    //
    // Search object's children
    //
    const QString filter =
    [=]() {
        const QString user_filter = filter_dialog->filter_widget->get_filter();

        const QString is_container = is_container_filter();

        // NOTE: OR user filter with containers filter so that container objects are always shown, even if they are filtered out by user filter
        QString out = filter_OR({user_filter, is_container});

        out = add_advanced_view_filter(out);

        // OR filter with some dev mode object classes, so that they show up no matter what when dev mode is on
        if (dev_mode) {
            const QList<QString> schema_classes = {
                "classSchema",
                "attributeSchema",
                "displaySpecifier",
            };

            QList<QString> class_filters;
            for (const QString object_class : schema_classes) {
                const QString class_filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, object_class);
                class_filters.append(class_filter);
            }

            out = filter_OR({out, filter_OR(class_filters)});
        }

        return out;
    }();

    const QList<QString> search_attributes =
    []() {
        QList<QString> out;

        out += ADCONFIG()->get_columns();

        // NOTE: load_object_row() needs this for loading group type/scope
        out += ATTRIBUTE_GROUP_TYPE;

        return out;
    }();

    const QString dn = index.data(Role_DN).toString();

    QHash<QString, AdObject> search_results = AD()->search(filter, search_attributes, SearchScope_Children, dn);

    // Dev mode
    // NOTE: configuration and schema objects are hidden so that they don't show up in regular searches. Have to use search_object() and manually add them to search results.
    if (dev_mode) {
        const QString search_base = AD()->domain_head();
        const QString configuration_dn = AD()->configuration_dn();
        const QString schema_dn = AD()->schema_dn();

        if (dn == search_base) {
            search_results[configuration_dn] = AD()->search_object(configuration_dn);
        } else if (dn == configuration_dn) {
            search_results[schema_dn] = AD()->search_object(schema_dn);
        }
    }

    //
    // Load into scope
    //
    QList<QStandardItem *> rows;
    for (const AdObject object : search_results.values()) {
        const QString object_class = object.get_string(ATTRIBUTE_OBJECT_CLASS);
        const bool should_be_in_scope = object_should_be_in_scope(object_class);

        if (should_be_in_scope) {
            auto child = make_scope_item(object);
            rows.append(child);
        }
    }

    // NOTE: use appendRows() instead of appendRow() because appendRows() performs MUCH better
    QStandardItem *item = scope_model->itemFromIndex(index);
    item->appendRows(rows);

    //
    // Load into results
    //
    const int id = index.data(ScopeRole_Id).toInt();

    const bool need_to_create_results = (!scope_id_to_results.contains(id));
    if (need_to_create_results) {
        auto new_results = new ConsoleDragModel(this);
        new_results->setHorizontalHeaderLabels(object_model_header_labels());
        scope_id_to_results[id] = new_results;
    }

    QStandardItemModel *results = scope_id_to_results[id];

    // Clear old results
    results->removeRows(0, results->rowCount());

    for (const AdObject object : search_results.values()) {
        make_results_row(results, object);
    }

    scope_model->setData(index, true, ScopeRole_Fetched);

    hide_busy_indicator();
}

QStandardItem *Console::make_scope_item(const AdObject &object) {
    auto item = new QStandardItem();
    item->setData(false, ScopeRole_Fetched);

    // NOTE: add fake child to new items, so that the child indicator is shown while they are childless until they are fetched
    auto dummy_item = new QStandardItem();
    dummy_item->setData(DUMMY_ITEM_ID, ScopeRole_Id);
    item->appendRow(dummy_item);
    
    const QString dn = object.get_dn();
    item->setData(dn, Role_DN);
    
    const QString name = dn_get_name(dn);
    item->setText(name);

    const QString object_class = object.get_string(ATTRIBUTE_OBJECT_CLASS);
    item->setData(object_class, Role_ObjectClass);

    static int id_max = 0;
    const int id = id_max;
    id_max++;
    item->setData(id, ScopeRole_Id);

    const QIcon icon = object.get_icon();
    item->setIcon(icon);

    return item;
}

// NOTE: as long as this is called where appropriate (on every target change), it is not necessary to do any condition checks in navigation f-ns since the actions that call them will be disabled if they can't be done
void Console::update_navigation_actions() {
    menubar->back_action->setEnabled(!targets_past.isEmpty());
    menubar->forward_action->setEnabled(!targets_future.isEmpty());
}

QModelIndex Console::get_scope_node_from_id(const int id) const {
    const QList<QModelIndex> matches = scope_model->match(scope_model->index(0, 0), ScopeRole_Id, id, 1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));
    if (!matches.isEmpty()) {
        return matches[0];
    } else {
        return QModelIndex();
    }
}

void Console::on_result_item_double_clicked(const QModelIndex &index)
{
    const QString dn = index.data(Role_DN).toString();
    const QString object_class = index.data(Role_ObjectClass).toString();
    const bool should_be_in_scope = object_should_be_in_scope(object_class);

    if (should_be_in_scope) {
        // Find the scope item that represents this object
        // and make it the current item of scope tree.
        const QList<QModelIndex> scope_index_matches = scope_model->match(scope_model->index(0, 0), Role_DN, dn, 1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));
        if (!scope_index_matches.empty()) {
            auto current_index = scope_index_matches[0];
            scope_view->selectionModel()->setCurrentIndex(current_index, QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);
        }
    } else {
        PropertiesDialog::open_for_target(dn);
    }
}

// NOTE: "containers" referenced here don't mean objects
// with "container" object class. Instead it means all the
// objects that can have children(some of which are not
// "container" class).
bool object_should_be_in_scope(const QString &object_class) {
    const bool show_non_containers_ON = SETTINGS()->get_bool(BoolSetting_ShowNonContainersInConsoleTree);

    const QList<QString> filter_containers = ADCONFIG()->get_filter_containers();

    return (filter_containers.contains(object_class) || show_non_containers_ON);
}
