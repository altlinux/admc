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

#include "panes.h"

#include "object_model.h"
#include "containers_proxy.h"
#include "advanced_view_proxy.h"
#include "object_menu.h"
#include "utils.h"
#include "ad_utils.h"
#include "details_dialog.h"
#include "ad_config.h"
#include "ad_interface.h"
#include "ad_object.h"
#include "filter.h"
#include "settings.h"
#include "filter_dialog.h"
#include "filter_widget/filter_widget.h"
#include "object_menu.h"
#include "panes_drag_model.h"

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

// TODO: currently showing refresh action always for all scope nodes. Could show it in results if node is in scope? Also could not show it if results isn't loaded?

QHash<int, QStandardItemModel *> scope_id_to_results;

Panes::Panes()
: QWidget()
{
    scope_model = new PanesDragModel(0, 1, this);

    scope_view = new QTreeView(this);
    scope_view->setHeaderHidden(true);
    scope_view->setExpandsOnDoubleClick(true);
    scope_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    scope_view->setContextMenuPolicy(Qt::CustomContextMenu);
    scope_view->setDragDropMode(QAbstractItemView::DragDrop);

    scope_view->setModel(scope_model);

    results_view = new QTreeView(this);
    results_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    results_view->header()->setSectionsMovable(true);
    results_view->setContextMenuPolicy(Qt::CustomContextMenu);
    results_view->setDragDropMode(QAbstractItemView::DragDrop);

    auto splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(scope_view);
    splitter->addWidget(results_view);
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
        this, &Panes::change_results_target);

    // TODO: not sure about this... Need to re-sort when items are renamed and maybe in other cases as well.
    connect(
        scope_model, &QAbstractItemModel::rowsInserted,
        [this]() {
            scope_view->sortByColumn(0, Qt::AscendingOrder);
        });

    connect(
        scope_view, &QTreeView::expanded,
        [=](const QModelIndex &index) {
            const bool fetched = index.data(Role_Fetched).toBool();
            if (!fetched) {
                fetch_scope_node(index);
            }
        });

    connect(
        qApp, &QApplication::focusChanged,
        this, &Panes::on_focus_changed);

    connect(
        scope_view, &QWidget::customContextMenuRequested,
        this, &Panes::open_context_menu);
    connect(
        results_view, &QWidget::customContextMenuRequested,
        this, &Panes::open_context_menu);

    connect(
        AD(), &AdInterface::object_added,
        this, &Panes::on_object_added);
    connect(
        AD(), &AdInterface::object_deleted,
        this, &Panes::on_object_deleted);
    connect(
        AD(), &AdInterface::object_changed,
        this, &Panes::on_object_changed);

    connect(
        scope_model, &QStandardItemModel::rowsAboutToBeRemoved,
        this, &Panes::on_scope_rows_about_to_be_removed);

    focused_view = scope_view;

    // Load head object
    const QString head_dn = AD()->domain_head();
    const AdObject head_object = AD()->search_object(head_dn);
    make_scope_item(scope_model->invisibleRootItem(), head_object);

    // Make head object current
    scope_view->selectionModel()->setCurrentIndex(scope_model->index(0, 0), QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);

    // Refresh head when settings affecting the filter
    // change. This reloads the model with an updated filter
    const BoolSettingSignal *advanced_view = SETTINGS()->get_bool_signal(BoolSetting_AdvancedView);
    connect(
        advanced_view, &BoolSettingSignal::changed,
        this, &Panes::refresh_head);

    const BoolSettingSignal *show_non_containers = SETTINGS()->get_bool_signal(BoolSetting_ShowNonContainersInContainersTree);
    connect(
        show_non_containers, &BoolSettingSignal::changed,
        this, &Panes::refresh_head);

    connect(
        filter_dialog, &QDialog::accepted,
        this, &Panes::refresh_head);
}

void Panes::refresh_head() {
    fetch_scope_node(scope_model->index(0, 0));
}

// Delete results attached to all removed scope nodes
// (including descendants!)
void Panes::on_scope_rows_about_to_be_removed(const QModelIndex &parent, int first, int last) {
    QStack<QStandardItem *> stack;

    for (int r = first; r <= last; r++) {
        const QModelIndex removed_index = scope_model->index(r, 0, parent);
        auto removed_item = scope_model->itemFromIndex(removed_index);
        stack.push(removed_item);
    }

    while (!stack.isEmpty()) {
        auto item = stack.pop();

        // NOTE: need to avoid processing dummy nodes! So
        // check that node was fetched
        const bool fetched = item->data(Role_Fetched).toBool();
        if (fetched) {
            // Remove results from hashmap and delete it
            const int id = item->data(Role_Id).toInt();
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
    }
}

// NOTE: responding to object changes/additions/deletions only in object part of the scope tree. Queries are left unupdated.

void Panes::on_object_deleted(const QString &dn) {
    const QString parent_dn = dn_get_parent(dn);

    const QList<QModelIndex> scope_parent_matches = scope_model->match(scope_model->index(0, 0), Role_DN, parent_dn, 1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));
    if (scope_parent_matches.isEmpty()) {
        return;
    }

    const QModelIndex scope_parent = scope_parent_matches[0];

    // Remove from results first(need object to still be in
    // scope tree to do this)
    const int scope_parent_id = scope_parent.data(Role_Id).toInt();
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

void Panes::on_object_added(const QString &dn) {
    // Add to scope
    const QString parent_dn = dn_get_parent(dn);
    const QList<QModelIndex> scope_parent_matches = scope_model->match(scope_model->index(0, 0), Role_DN, parent_dn, 1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));
    if (scope_parent_matches.isEmpty()) {
        return;
    }

    const QModelIndex scope_parent = scope_parent_matches[0];

    const AdObject object = AD()->search_object(dn);

    const bool fetched = scope_parent.data(Role_Fetched).toBool();

    // NOTE: only add if parent was fetched already. If parent wasn't fetched, then this new object will be added when parent is fetched.
    if (fetched) {
        QStandardItem *parent_item = scope_model->itemFromIndex(scope_parent);
        make_scope_item(parent_item, object);
    }

    // Add to results
    const int scope_parent_id = scope_parent.data(Role_Id).toInt();
    QStandardItemModel *results_model = scope_id_to_results.value(scope_parent_id, nullptr);
    if (results_model != nullptr) {
        make_results_row(results_model, object);
    }
}

void Panes::make_results_row(QStandardItemModel * model, const AdObject &object) {
    const QList<QStandardItem *> row = make_item_row(ADCONFIG()->get_columns().size());
    load_results_row(row, object);
    model->appendRow(row);
}

// NOTE: only updating object in results. Attribute changes don't matter to scope because it doesn't display any attributes, so only need to update results.
void Panes::on_object_changed(const QString &dn) {
    const QString parent_dn = dn_get_parent(dn);
    const QList<QModelIndex> scope_parent_matches = scope_model->match(scope_model->index(0, 0), Role_DN, parent_dn, 1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive | Qt::MatchWrap));
    if (scope_parent_matches.isEmpty()) {
        return;
    }

    const QModelIndex scope_parent = scope_parent_matches[0];
    const int scope_parent_id = scope_parent.data(Role_Id).toInt();
    QStandardItemModel *results_model = scope_id_to_results.value(scope_parent_id, nullptr);
    const QList<QModelIndex> results_index_matches = results_model->match(scope_parent, Role_DN, dn, 1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));

    if (!results_index_matches.isEmpty()) {
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
}

void Panes::load_menu(QMenu *menu) {
    QList<QString> targets;
    QList<QString> target_classes;

    const QList<QModelIndex> indexes = focused_view->selectionModel()->selectedIndexes();

    for (const QModelIndex index : indexes) {
        // Need first column to access item data
        if (index.column() != 0) {
            continue;
        }

        const QString dn = index.data(Role_DN).toString();
        const QString object_class = index.data(Role_ObjectClass).toString();

        targets.append(dn);
        target_classes.append(object_class);
    }

    add_object_actions_to_menu(menu, targets, target_classes, this);

    if (focused_view == scope_view) {
        const QModelIndex index = scope_view->selectionModel()->currentIndex();
        const bool was_fetched = index.data(Role_Fetched).toBool();

        if (was_fetched) {
            menu->addAction(tr("Refresh"),
                [=]() {
                    const QModelIndex current_index = scope_view->selectionModel()->currentIndex();
                    fetch_scope_node(current_index);

                });
        }
    }
}

void Panes::open_context_menu(const QPoint pos) {
    auto menu = new QMenu(this);
    load_menu(menu);
    exec_menu_from_view(menu, focused_view, pos);
}

// NOTE: this is the workaround required to know in which pane selected objects are located
void Panes::on_focus_changed(QWidget *old, QWidget *now) {
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

void Panes::change_results_target(const QModelIndex &current, const QModelIndex &) {
    if (!current.isValid()) {
        return;
    }
    
    // Fetch if needed
    const bool fetched = current.data(Role_Fetched).toBool();
    if (!fetched) {
        fetch_scope_node(current);
    }

    const int id = scope_model->data(current, Role_Id).toInt();
    QStandardItemModel *results_model = scope_id_to_results[id];

    results_view->setModel(results_model);
}

void Panes::load_results_row(QList<QStandardItem *> row, const AdObject &object) {
    load_attributes_row(row, object);
    row[0]->setData(object.get_dn(), Role_DN);
    row[0]->setData(object.get_string(ATTRIBUTE_OBJECT_CLASS), Role_ObjectClass);
}

// TODO: should expand node when fetching or no? it seems that in all cases i can think of the node is already expanded.

// Load children of this item in scope tree
// and load results linked to this scope item
void Panes::fetch_scope_node(const QModelIndex &index) {
    show_busy_indicator();

    // NOTE: remove old scope children (which might be a dummy child used for showing child indicator)
    scope_model->removeRows(0, scope_model->rowCount(index), index);

    //
    // Search object's children
    //
    const QString filter =
    [=]() {
        const QString user_filter = filter_dialog->filter_widget->get_filter();

        const QString is_container =
        []() {
            const QList<QString> accepted_classes = ADCONFIG()->get_filter_containers();

            QList<QString> class_filters;
            for (const QString object_class : accepted_classes) {
                const QString class_filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, object_class);
                class_filters.append(class_filter);
            }

            return filter_OR(class_filters);
        }();

        const QString is_not_advanced_view_only = filter_CONDITION(Condition_NotEquals, ATTRIBUTE_SHOW_IN_ADVANCED_VIEW_ONLY, "true");

        QString out;
        
        // NOTE: OR user filter with containers filter so that container objects are always shown, even if they are filtered out by user filter
        out = filter_OR({user_filter, is_container});

        // Hide advanced view only" objects if advanced view setting is off
        const bool advanced_view_OFF = !SETTINGS()->get_bool(BoolSetting_AdvancedView);
        if (advanced_view_OFF) {
            out = filter_AND({out, is_not_advanced_view_only});
        }

        return out;
    }();

    const QList<QString> search_attributes = ADCONFIG()->get_columns();

    const QString dn = index.data(Role_DN).toString();

    const QHash<QString, AdObject> search_results = AD()->search(filter, search_attributes, SearchScope_Children, dn);

    //
    // Load into scope
    //
    QStandardItem *item = scope_model->itemFromIndex(index);
    const QList<QString> container_classes = ADCONFIG()->get_filter_containers();
    const bool show_non_containers_ON = SETTINGS()->get_bool(BoolSetting_ShowNonContainersInContainersTree);
    for (const AdObject object : search_results.values()) {
        const bool is_container =
        [=]() {
            const QString object_class = object.get_string(ATTRIBUTE_OBJECT_CLASS);

            return container_classes.contains(object_class);
        }();

        if (is_container || show_non_containers_ON) {
            make_scope_item(item, object);
        }
    }

    //
    // Load into results
    //
    const int id = index.data(Role_Id).toInt();

    const bool need_to_create_results = (!scope_id_to_results.contains(id));
    if (need_to_create_results) {
        auto new_results = new PanesDragModel(this);
        new_results->setHorizontalHeaderLabels(object_model_header_labels());
        scope_id_to_results[id] = new_results;
    }

    QStandardItemModel *results = scope_id_to_results[id];

    // Clear old results
    results->removeRows(0, results->rowCount());

    for (const AdObject object : search_results.values()) {
        make_results_row(results, object);
    }

    scope_model->setData(index, true, Role_Fetched);

    hide_busy_indicator();
}

void Panes::make_scope_item(QStandardItem *parent, const AdObject &object) {
    auto item = new QStandardItem();
    item->setData(false, Role_Fetched);

    // NOTE: add fake child to new items, so that the child indicator is shown while they are childless until they are fetched
    item->appendRow(new QStandardItem());
    
    const QString dn = object.get_dn();
    item->setData(dn, Role_DN);
    
    const QString name = dn_get_name(dn);
    item->setText(name);

    const QString object_class = object.get_string(ATTRIBUTE_OBJECT_CLASS);
    item->setData(object_class, Role_ObjectClass);

    static int id_max = 0;
    const int id = id_max;
    id_max++;
    item->setData(id, Role_Id);

    const QIcon icon = object.get_icon();
    item->setIcon(icon);

    parent->appendRow(item);
}
