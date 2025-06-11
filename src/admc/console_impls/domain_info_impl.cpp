/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
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

#include "domain_info_impl.h"
#include "console_widget/console_widget.h"
#include "results_widgets/domain_info_results_widget/domain_info_results_widget.h"
#include "console_impls/object_impl.h"
#include "console_impls/policy_root_impl.h"
#include "console_impls/query_folder_impl.h"
#include "ad_interface.h"
#include "item_type.h"
#include "connection_options_dialog.h"
#include "fsmo/fsmo_dialog.h"
#include "utils.h"
#include "status.h"
#include "globals.h"
#include "managers/icon_manager.h"

#include <QSet>
#include <QAction>
#include <QModelIndex>
#include <QStandardItem>
#include <QIcon>


DomainInfoImpl::DomainInfoImpl(ConsoleWidget *console_arg): ConsoleImpl(console_arg) {
    domain_info_results_widget = new DomainInfoResultsWidget(console_arg);
    set_results_widget(domain_info_results_widget);

    edit_fsmo_action = new QAction(tr("Edit FSMO roles"), this);
    connection_options_action = new QAction(tr("Open connection options"), this);
    connect(edit_fsmo_action, &QAction::triggered, this, &DomainInfoImpl::open_fsmo_dialog);
    connect(connection_options_action, &QAction::triggered, this, &DomainInfoImpl::open_connection_options);
}

void DomainInfoImpl::selected_as_scope(const QModelIndex &index) {
    Q_UNUSED(index)
    domain_info_results_widget->update();
}

void DomainInfoImpl::refresh(const QList<QModelIndex> &index_list) {
    Q_UNUSED(index_list)

    AdInterface ad;
    load_domain_info_item(ad);
    domain_info_results_widget->update();
    console->clear_scope_tree();

    console->set_current_scope(console->domain_info_index());
    if (ad.is_connected()) {
        console_object_tree_init(console, ad);
        console_policy_tree_init(console);
        console_query_tree_init(console);
        console_tree_add_password_settings(console, ad);
    }
    console->expand_item(console->domain_info_index());
}

QList<QAction *> DomainInfoImpl::get_all_custom_actions() const {
    return {edit_fsmo_action, connection_options_action};
}

QSet<QAction *> DomainInfoImpl::get_custom_actions(const QModelIndex &index, const bool single_selection) const {
    Q_UNUSED(index)
    Q_UNUSED(single_selection)

    return {edit_fsmo_action, connection_options_action};
}

QSet<StandardAction> DomainInfoImpl::get_standard_actions(const QModelIndex &index, const bool single_selection) const {
    Q_UNUSED(index)
    Q_UNUSED(single_selection)

    return {StandardAction_Refresh};
}

QList<QString> DomainInfoImpl::column_labels() const {
    return {tr("Name")};
}

void DomainInfoImpl::load_domain_info_item(const AdInterface &ad) {
    QString dc = tr("Host not found");
    if (ad.is_connected()) {
        dc = ad.get_dc();
    }
    QStandardItem *domain_info_item = console->get_item(console->domain_info_index());
    domain_info_item->setText(tr("Active directory managment center [") + dc + ']');
    domain_info_item->setIcon(g_icon_manager->category_icon(ADMC_CATEGORY_DOMAIN_INFO_ITEM));
}

void DomainInfoImpl::open_fsmo_dialog() {
    AdInterface ad;
    if (!ad.is_connected()) {
        return;
    }

    auto dialog = new FSMODialog(ad, console);
    dialog->open();
    connect(dialog, &FSMODialog::master_changed, domain_info_results_widget, &DomainInfoResultsWidget::update_fsmo_roles);
}

void DomainInfoImpl::open_connection_options() {
    auto dialog = new ConnectionOptionsDialog(console);
    dialog->open();
    connect(dialog, &ConnectionOptionsDialog::host_changed,
            [this](const QString &host) {
        show_busy_indicator();
        console->refresh_scope(console->domain_info_index());
        hide_busy_indicator();
        g_status->add_message(tr("Connected to host ") + host, StatusType_Success);
    });
}
