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

#include "find_widget.h"

#include "adldap.h"
#include "console_impls/object_impl.h"
#include "console_impls/item_type.h"
#include "console_impls/query_item_impl.h"
#include "filter_widget/filter_widget.h"
#include "filter_widget/select_base_widget.h"
#include "globals.h"
#include "search_thread.h"
#include "settings.h"
#include "utils.h"
#include "console_widget/console_widget.h"
#include "console_widget/results_view.h"
#include "console_impls/object_impl.h"

#include <QFormLayout>
#include <QFrame>
#include <QList>
#include <QPushButton>
#include <QModelIndex>
#include <QStandardItem>
#include <QMenu>

FindWidget::FindWidget(const QList<QString> classes, const QString &default_base)
: QWidget() {
    console = new ConsoleWidget(this);

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
    head_item = row[0];
    head_item->setText(tr("Find results"));

    console->set_scope_view_visible(false);

    const QVariant console_widget_state = settings_get_variant(SETTING_find_results_state);
    console->restore_state(console_widget_state);
    
    const QModelIndex head_index = head_item->index();
    console->set_current_scope(head_index);

    select_base_widget = new SelectBaseWidget(g_adconfig, default_base);

    filter_widget = new FilterWidget(g_adconfig, classes);

    find_button = new QPushButton(tr("Find"));
    find_button->setDefault(true);
    find_button->setObjectName("find_button");

    stop_button = new QPushButton(tr("Stop"));
    stop_button->setAutoDefault(false);

    auto filter_widget_frame = new QFrame();
    filter_widget_frame->setFrameStyle(QFrame::Raised);
    filter_widget_frame->setFrameShape(QFrame::Box);

    {
        auto select_base_layout_inner = new QFormLayout();
        select_base_layout_inner->addRow(tr("Search in:"), select_base_widget);

        auto select_base_layout = new QHBoxLayout();
        select_base_layout->addLayout(select_base_layout_inner);
        select_base_layout->addStretch();

        auto button_layout = new QHBoxLayout();
        button_layout->addWidget(find_button);
        button_layout->addWidget(stop_button);
        button_layout->addStretch();

        auto layout = new QVBoxLayout();
        filter_widget_frame->setLayout(layout);
        layout->addLayout(select_base_layout);
        layout->addWidget(filter_widget);
        layout->addLayout(button_layout);
    }

    {
        auto layout = new QHBoxLayout();
        setLayout(layout);
        layout->addWidget(filter_widget_frame);
        layout->addWidget(console);
    }

    // Keep filter widget compact, so that when user
    // expands find dialog horizontally, filter widget will
    // keep it's size, find results will get expanded
    filter_widget_frame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    console->setMinimumSize(500, 0);

    connect(
        find_button, &QPushButton::clicked,
        this, &FindWidget::find);

    // NOTE: need this for the case where dialog is closed
    // while a search is in progress. Without this busy
    // indicator stays on.
    connect(
        this, &QObject::destroyed,
        []() {
            hide_busy_indicator();
        });
}

FindWidget::~FindWidget() {
    const QVariant state = console->save_state();
    settings_set_variant(SETTING_find_results_state, state);
}

void FindWidget::add_actions(QMenu *action_menu, QMenu *view_menu) {
    console->add_actions(action_menu);

    view_menu->addAction(console->customize_columns_action());

    connect(
        action_menu, &QMenu::aboutToShow,
        console, &ConsoleWidget::update_actions);
}

void FindWidget::clear() {
    const QModelIndex head_index = head_item->index();
    console->delete_children(head_index);
}

void FindWidget::find() {
    // Prepare search args
    const QString filter = filter_widget->get_filter();
    const QString base = select_base_widget->get_base();
    const QList<QString> search_attributes = console_object_search_attributes();

    auto find_thread = new SearchThread(base, SearchScope_All, filter, search_attributes);

    connect(
        find_thread, &SearchThread::results_ready,
        this, &FindWidget::handle_find_thread_results);
    connect(
        this, &QObject::destroyed,
        find_thread, &SearchThread::stop);
    connect(
        stop_button, &QPushButton::clicked,
        find_thread, &SearchThread::stop);
    connect(
        find_thread, &SearchThread::finished,
        this, &FindWidget::on_thread_finished);

    show_busy_indicator();

    // NOTE: disable find button, otherwise another find
    // process can start while this one isn't finished!
    find_button->setEnabled(false);

    clear();

    find_thread->start();
}

void FindWidget::handle_find_thread_results(const QHash<QString, AdObject> &results) {
    const QModelIndex head_index = head_item->index();

    for (const AdObject &object : results) {
        const QList<QStandardItem *> row = console->add_results_item(ItemType_Object, head_index);

        console_object_load(row, object);
    }
}

void FindWidget::on_thread_finished() {
    find_button->setEnabled(true);

    hide_busy_indicator();
}

QList<QString> FindWidget::get_selected_dns() const {
    const QList<QString> out = get_selected_dn_list(console, ItemType_Object, ObjectRole_DN);

    return out;
}
