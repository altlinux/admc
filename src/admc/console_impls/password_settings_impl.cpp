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

#include "console_impls/password_settings_impl.h"

#include "ad_defines.h"
#include "adldap.h"
#include "console_impls/item_type.h"
#include "console_impls/object_impl/object_impl.h"
#include "console_impls/policy_impl.h"
#include "console_widget/results_view.h"
#include "create_dialogs/create_policy_dialog.h"
#include "fsmo/fsmo_utils.h"
#include "globals.h"
#include "object_impl/console_object_operations.h"
#include "results_widgets/pso_results_widget/pso_results_widget.h"
#include "status.h"
#include "utils.h"

#include <QAction>
#include <QList>
#include <QMessageBox>
#include <QStandardItem>

PasswordSettingsImpl::PasswordSettingsImpl(ConsoleWidget *console_arg)
: ConsoleImpl(console_arg) {
    set_results_widget(new PSOResultsWidget(console_arg));

    create_pso_action = new QAction(tr("Create password settings object"), this);

    connect(
        create_pso_action, &QAction::triggered,
        this, [this]() {
            const QString parent_dn = get_selected_target_dn(console, ItemType_PasswordSettings, ObjectRole_DN);
            ConsoleObjectTreeOperations::console_object_create({console}, CLASS_PSO, parent_dn);
        });
}

void PasswordSettingsImpl::fetch(const QModelIndex &index) {
    AdInterface ad;
    if (ad_failed(ad, console)) {
        return;
    }

    const QString base = g_adconfig->pso_container_dn();
    const SearchScope scope = SearchScope_Children;
    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_PSO_CONTAINER);
    const QList<QString> attributes = QList<QString>();
    const QHash<QString, AdObject> results = ad.search(base, scope, "", attributes);

    ConsoleObjectTreeOperations::add_objects_to_console(console, results.values(), index);
}

void PasswordSettingsImpl::refresh(const QList<QModelIndex> &index_list) {
    const QModelIndex index = index_list[0];

    console->delete_children(index);
    fetch(index);
}

QList<QAction *> PasswordSettingsImpl::get_all_custom_actions() const {
    QList<QAction *> out;

    out.append(create_pso_action);

    return out;
}

QSet<QAction *> PasswordSettingsImpl::get_custom_actions(const QModelIndex &index, const bool single_selection) const {
    UNUSED_ARG(index);
    UNUSED_ARG(single_selection);

    QSet<QAction *> out;

    out.insert(create_pso_action);

    return out;
}

QSet<StandardAction> PasswordSettingsImpl::get_standard_actions(const QModelIndex &index, const bool single_selection) const {
    UNUSED_ARG(index);
    UNUSED_ARG(single_selection);

    QSet<StandardAction> out;

    out.insert(StandardAction_Refresh);

    return out;
}

void password_settings_impl_add_objects(ConsoleWidget *console, const QList<AdObject> &object_list, const QModelIndex &parent) {
    if (!parent.isValid()) {
        return;
    }

    const bool parent_was_fetched = console_item_get_was_fetched(parent);
    if (!parent_was_fetched) {
        return;
    }

    for (const AdObject &object : object_list) {
        const QList<QStandardItem *> row = console->add_scope_item(ItemType_Object, parent);
        ConsoleObjectTreeOperations::console_object_load(row, object);
    }
}

QList<QString> PasswordSettingsImpl::column_labels() const {
    return ConsoleObjectTreeOperations::object_impl_column_labels();
}

QList<int> PasswordSettingsImpl::default_columns() const {
    return ConsoleObjectTreeOperations::object_impl_default_columns();
}
