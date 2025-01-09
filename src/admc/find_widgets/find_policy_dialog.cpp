/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#include "find_policy_dialog.h"
#include "find_policy_dialog_p.h"
#include "ui_find_policy_dialog.h"

#include "adldap.h"
#include "console_impls/find_policy_impl.h"
#include "console_impls/found_policy_impl.h"
#include "console_impls/item_type.h"
#include "globals.h"
#include "search_thread.h"
#include "settings.h"
#include "status.h"
#include "utils.h"

#include <QAction>
#include <QMenuBar>
#include <QStandardItem>

FindPolicyDialog::FindPolicyDialog(ConsoleWidget *buddy_console, QWidget *parent)
: QDialog(parent) {
    ui = new Ui::FindPolicyDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    auto menubar = new QMenuBar();
    layout()->setMenuBar(menubar);
    auto action_menu = menubar->addMenu(tr("&Action"));
    menubar->setContextMenuPolicy(Qt::PreventContextMenu);
    auto view_menu = menubar->addMenu(tr("&View"));

    ui->find_button->setDefault(true);

    // Fill search item combo
    const QList<SearchItem> search_item_list = {
        SearchItem_Name,
        SearchItem_GUID,
    };

    for (const SearchItem &search_item : search_item_list) {
        const QString search_item_string = [&]() {
            switch (search_item) {
                case SearchItem_Name: return tr("Name");
                case SearchItem_GUID: return tr("GUID");
            }

            return QString();
        }();

        ui->search_item_combo->addItem(search_item_string, (int) search_item);
    }

    // Fill condition combo
    const QList<Condition> condition_list = {
        Condition_Contains,
        Condition_Equals,
        Condition_StartsWith,
        Condition_EndsWith,
    };

    for (const Condition &condition : condition_list) {
        const QString condition_string = condition_to_display_string(condition);

        ui->condition_combo->addItem(condition_string, (int) condition);
    }

    // Setup console
    auto find_impl = new FindPolicyImpl(ui->console);
    ui->console->register_impl(ItemType_FindPolicy, find_impl);

    auto found_policy_impl = new FoundPolicyImpl(ui->console);
    ui->console->register_impl(ItemType_FoundPolicy, found_policy_impl);
    found_policy_impl->set_buddy_console(buddy_console);

    auto action_view_icons = new QAction(tr("&Icons"), this);
    action_view_icons->setCheckable(true);
    auto action_view_list = new QAction(tr("&List"), this);
    action_view_list->setCheckable(true);
    auto action_view_detail = new QAction(tr("&Detail"), this);
    action_view_detail->setCheckable(true);
    auto action_customize_columns = new QAction(tr("&Customize Columns"), this);
    auto action_toggle_description_bar = new QAction(tr("&Description Bar"), this);
    action_toggle_description_bar->setCheckable(true);

    const ConsoleWidgetActions console_actions = [&]() {
        ConsoleWidgetActions out;

        out.view_icons = action_view_icons;
        out.view_list = action_view_list;
        out.view_detail = action_view_detail;
        out.toggle_description_bar = action_toggle_description_bar;
        out.customize_columns = action_customize_columns;

        // Use placeholders for unused actions
        out.navigate_up = new QAction(this);
        out.navigate_back = new QAction(this);
        out.navigate_forward = new QAction(this);
        out.refresh = new QAction(this);
        out.toggle_console_tree = new QAction(this);

        return out;
    }();

    ui->console->set_actions(console_actions);

    ui->console->setup_menubar_action_menu(action_menu);

    view_menu->addAction(action_view_icons);
    view_menu->addAction(action_view_list);
    view_menu->addAction(action_view_detail);
    view_menu->addSeparator();
    view_menu->addAction(action_customize_columns);
    view_menu->addAction(action_toggle_description_bar);

    const QList<QStandardItem *> row = ui->console->add_scope_item(ItemType_FindPolicy, QModelIndex());
    head_item = row[0];
    head_item->setText(tr("Find results"));

    ui->console->set_scope_view_visible(false);

    const QModelIndex head_index = head_item->index();
    ui->console->set_current_scope(head_index);

    settings_setup_dialog_geometry(SETTING_find_policy_dialog_geometry, this);

    const QVariant console_state = settings_get_variant(SETTING_find_policy_dialog_console_state);
    ui->console->restore_state(console_state);

    connect(
        ui->add_button, &QAbstractButton::clicked,
        this, &FindPolicyDialog::add_filter);
    connect(
        ui->find_button, &QAbstractButton::clicked,
        this, &FindPolicyDialog::find);
    connect(
        ui->clear_button, &QAbstractButton::clicked,
        this, &FindPolicyDialog::clear_results);
}

FindPolicyDialog::~FindPolicyDialog() {
    const QVariant console_state = ui->console->save_state();
    settings_set_variant(SETTING_find_policy_dialog_console_state, console_state);

    delete ui;
}

void FindPolicyDialog::add_filter() {
    const QString filter = [&]() {
        const QString attribute = [&]() -> QString {
            const SearchItem search_item = (SearchItem) ui->search_item_combo->currentData().toInt();
            switch (search_item) {
                case SearchItem_Name: return ATTRIBUTE_DISPLAY_NAME;
                case SearchItem_GUID: return ATTRIBUTE_CN;
            }

            return QString();
        }();

        const Condition condition = (Condition) ui->condition_combo->currentData().toInt();

        const QString value = ui->value_edit->text();

        const QString out = filter_CONDITION(condition, attribute, value);

        return out;
    }();

    const QString filter_display = [&]() {
        const QString search_item_display = [&]() -> QString {
            const SearchItem search_item = (SearchItem) ui->search_item_combo->currentData().toInt();
            switch (search_item) {
                case SearchItem_Name: return tr("Name");
                case SearchItem_GUID: return tr("GUID");
            }

            return QString();
        }();

        const QString condition_display = [&]() {
            const Condition condition = (Condition) ui->condition_combo->currentData().toInt();
            const QString out = condition_to_display_string(condition);

            return out;
        }();

        const QString value = ui->value_edit->text();

        const QString out = QString("%1 %2: \"%3\"").arg(search_item_display, condition_display, value);

        return out;
    }();

    auto item = new QListWidgetItem();
    item->setText(filter_display);
    item->setData(Qt::UserRole, filter);
    ui->filter_list->addItem(item);

    ui->value_edit->clear();
}

void FindPolicyDialog::find() {
    const QString base = g_adconfig->policies_dn();
    const QString filter = [this]() {
        QList<QString> filter_string_list;

        for (int i = 0; i < ui->filter_list->count(); i++) {
            const QListWidgetItem *item = ui->filter_list->item(i);
            const QString this_filter = item->data(Qt::UserRole).toString();

            filter_string_list.append(this_filter);
        }

        const QString out = filter_AND(filter_string_list);

        return out;
    }();
    const QList<QString> search_attributes = QList<QString>();

    auto search_thread = new SearchThread(base, SearchScope_Children, filter, search_attributes);

    connect(
        search_thread, &SearchThread::results_ready,
        this, &FindPolicyDialog::handle_search_thread_results);
    connect(
        this, &QObject::destroyed,
        search_thread, &SearchThread::stop);
    connect(
        ui->stop_button, &QPushButton::clicked,
        search_thread, &SearchThread::stop);
    connect(
        search_thread, &SearchThread::finished,
        this,
        [this, search_thread]() {
            g_status->display_ad_messages(search_thread->get_ad_messages(), this);
            search_thread_display_errors(search_thread, this);

            ui->find_button->setEnabled(true);
            ui->clear_button->setEnabled(true);

            hide_busy_indicator();

            search_thread->deleteLater();
        });

    show_busy_indicator();

    // NOTE: disable find and clear buttons, do not want
    // those functions to be available while a search is in
    // progress
    ui->find_button->setEnabled(false);
    ui->clear_button->setEnabled(false);

    clear_results();

    search_thread->start();
}

void FindPolicyDialog::handle_search_thread_results(const QHash<QString, AdObject> &results) {
    const QModelIndex head_index = head_item->index();

    for (const AdObject &object : results.values()) {
        const QList<QStandardItem *> row = ui->console->add_results_item(ItemType_FoundPolicy, head_index);

        found_policy_impl_load(row, object);
    }
}

void FindPolicyDialog::clear_results() {
    const QModelIndex head_index = head_item->index();
    ui->console->delete_children(head_index);
}
