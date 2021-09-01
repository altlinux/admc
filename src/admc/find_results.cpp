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

#include "find_results.h"

#include "console_widget/console_widget.h"
#include "console_widget/results_view.h"
#include "console_impls/object_impl.h"
#include "settings.h"
#include "utils.h"
#include "item_type.h"
#include "policy_results_widget.h"

#include "console_impls/query_item_impl.h"

#include <QMenu>
#include <QVBoxLayout>
#include <QStandardItem>

FindResults::FindResults()
: QWidget() {
    console = new ConsoleWidget(this);

    // TODO: deal with filter_dialog and
    // policy_results_widget args. They are not used here
    // but necessary for the ctor
    auto object_impl = new ObjectImpl(console);
    console->register_impl(ItemType_Object, object_impl);

    object_impl->set_find_action_enabled(false);
    object_impl->set_refresh_action_enabled(false);

    // NOTE: registering impl so that it supplies text to
    // the description bar
    auto query_item_impl = new QueryItemImpl(console);
    console->register_impl(ItemType_QueryItem, query_item_impl);

    query_item_impl->view()->set_drag_drop_enabled(false);

    const QList<QStandardItem *> row = console->add_scope_item(ItemType_QueryItem, QModelIndex());
    QStandardItem *main_item = row[0];
    main_item->setText(tr("Find results"));

    head_index = main_item->index();

    console->set_scope_view_visible(false);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(console);

    const QVariant console_widget_state = settings_get_variant(SETTING_find_results_state);
    console->restore_state(console_widget_state);
    
    console->set_current_scope(main_item->index());
}

FindResults::~FindResults() {
    const QVariant state = console->save_state();
    settings_set_variant(SETTING_find_results_state, state);
}

void FindResults::add_actions(QMenu *action_menu, QMenu *view_menu) {
    console->connect_to_action_menu(action_menu);

    view_menu->addAction(console->customize_columns_action());
}

void FindResults::clear() {
    console->delete_children(head_index);
}

void FindResults::load(const QHash<QString, AdObject> &results) {
    for (const AdObject &object : results) {
        const QList<QStandardItem *> row = console->add_results_item(ItemType_Object, head_index);

        console_object_load(row, object);
    }
}

// TODO: get from console
QList<QList<QStandardItem *>> FindResults::get_selected_rows() const {
    // const QList<QModelIndex> selected_indexes = view->get_selected_indexes();

    QList<QList<QStandardItem *>> out;

    // for (const QModelIndex row_index : selected_indexes) {
    //     const int row = row_index.row();

    //     QList<QStandardItem *> row_copy;

    //     for (int col = 0; col < model->columnCount(); col++) {
    //         QStandardItem *item = model->item(row, col);
    //         QStandardItem *item_copy = item->clone();
    //         row_copy.append(item_copy);
    //     }

    //     out.append(row_copy);
    // }

    return out;
}
