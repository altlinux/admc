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
#include "ui_find_widget.h"

#include "adldap.h"
#include "console_impls/item_type.h"
#include "console_impls/object_impl.h"
#include "console_impls/query_folder_impl.h"
#include "console_impls/query_item_impl.h"
#include "console_widget/results_view.h"
#include "search_thread.h"
#include "settings.h"
#include "utils.h"
#include "globals.h"

#include <QMenu>
#include <QStandardItem>

FindWidget::FindWidget(QWidget *parent)
: QWidget(parent) {
    ui = new Ui::FindWidget();
    ui->setupUi(this);

    action_view_icons = new QAction(tr("Icons"));
    action_view_icons->setCheckable(true);
    action_view_list = new QAction(tr("List"));
    action_view_list->setCheckable(true);
    action_view_detail = new QAction(tr("Detail"));
    action_view_detail->setCheckable(true);
    action_customize_columns = new QAction(tr("Customize Columns"));
    action_toggle_description_bar = new QAction(tr("Description Bar"));
    action_toggle_description_bar->setCheckable(true);

    const ConsoleWidgetActions console_actions = [&]() {
        ConsoleWidgetActions out;

        out.view_icons = action_view_icons;
        out.view_list = action_view_list;
        out.view_detail = action_view_detail;
        out.toggle_description_bar = action_toggle_description_bar;

        // Use placeholders for unused actions
        out.navigate_up = new QAction();
        out.navigate_back = new QAction();
        out.navigate_forward = new QAction();
        out.refresh = new QAction();
        out.customize_columns = new QAction();
        out.toggle_console_tree = new QAction();

        return out;
    }();

    ui->console->set_actions(console_actions);

    object_impl = new ObjectImpl(g_adconfig, ui->console);
    ui->console->register_impl(ItemType_Object, object_impl);

    object_impl->set_find_action_enabled(false);
    object_impl->set_refresh_action_enabled(false);

    // NOTE: registering impl so that it supplies text to
    // the description bar
    // TODO: g_adconfig
    auto query_item_impl = new QueryItemImpl(g_adconfig, ui->console);
    ui->console->register_impl(ItemType_QueryItem, query_item_impl);

    // TODO: g_adconfig
    auto query_folder_impl = new QueryFolderImpl(g_adconfig, ui->console);
    ui->console->register_impl(ItemType_QueryFolder, query_folder_impl);

    ResultsView *query_results = query_item_impl->view();
    query_results->set_drag_drop_enabled(false);

    const QList<QStandardItem *> root_row = ui->console->add_scope_item(ItemType_QueryFolder, QModelIndex());
    QStandardItem *root_item = root_row[0];
    root_item->setData(true, QueryItemRole_IsRoot);

    const QList<QStandardItem *> row = ui->console->add_scope_item(ItemType_QueryItem, root_item->index());
    head_item = row[0];
    head_item->setText(tr("Find results"));

    ui->console->set_scope_view_visible(false);

    const QModelIndex head_index = head_item->index();
    ui->console->set_current_scope(head_index);

    connect(
        ui->find_button, &QPushButton::clicked,
        this, &FindWidget::find);
    connect(
        ui->clear_button, &QPushButton::clicked,
        this, &FindWidget::on_clear_button);

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
    delete ui;
}

void FindWidget::init(AdConfig *adconfig, const QList<QString> &class_list, const QList<QString> &selected_list, const QString &default_base) {
    ui->filter_widget->init(adconfig, class_list, selected_list);
    ui->select_base_widget->init(adconfig, default_base);
}

void FindWidget::set_buddy_console(ConsoleWidget *buddy_console) {
    object_impl->set_buddy_console(buddy_console);
}

QVariant FindWidget::save_console_state() const {
    const QVariant state = ui->console->save_state();

    return state;
}

void FindWidget::restore_console_state(const QVariant &state) {
    ui->console->restore_state(state);
}

void FindWidget::setup_action_menu(QMenu *menu) {
    ui->console->add_actions(menu);

    connect(
        menu, &QMenu::aboutToShow,
        ui->console, &ConsoleWidget::update_actions);
}

void FindWidget::setup_view_menu(QMenu *menu) {
    menu->addAction(action_view_icons);
    menu->addAction(action_view_list);
    menu->addAction(action_view_detail);
    menu->addSeparator();
    menu->addAction(action_customize_columns);
    menu->addAction(action_toggle_description_bar);
}

void FindWidget::clear_results() {
    const QModelIndex head_index = head_item->index();
    ui->console->delete_children(head_index);
}

void FindWidget::find() {
    // Prepare search args
    const QString filter = ui->filter_widget->get_filter();
    const QString base = ui->select_base_widget->get_base();
    const QList<QString> search_attributes = console_object_search_attributes();

    auto find_thread = new SearchThread(base, SearchScope_All, filter, search_attributes);

    connect(
        find_thread, &SearchThread::results_ready,
        this, &FindWidget::handle_find_thread_results);
    connect(
        this, &QObject::destroyed,
        find_thread, &SearchThread::stop);
    connect(
        ui->stop_button, &QPushButton::clicked,
        find_thread, &SearchThread::stop);
    connect(
        find_thread, &SearchThread::finished,
        this, &FindWidget::on_thread_finished);

    show_busy_indicator();

    // NOTE: disable find and clear buttons, do not want
    // those functions to be available while a search is in
    // progress
    ui->find_button->setEnabled(false);
    ui->clear_button->setEnabled(false);

    clear_results();

    find_thread->start();
}

void FindWidget::handle_find_thread_results(const QHash<QString, AdObject> &results) {
    const QModelIndex head_index = head_item->index();

    for (const AdObject &object : results) {
        const QList<QStandardItem *> row = ui->console->add_results_item(ItemType_Object, head_index);

        console_object_load(row, object);
    }
}

void FindWidget::on_thread_finished() {
    ui->find_button->setEnabled(true);
    ui->clear_button->setEnabled(true);

    hide_busy_indicator();
}

QList<QString> FindWidget::get_selected_dns() const {
    const QList<QString> out = get_selected_dn_list(ui->console, ItemType_Object, ObjectRole_DN);

    return out;
}

void FindWidget::on_clear_button() {
    ui->filter_widget->clear();
    clear_results();
}
