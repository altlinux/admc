/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2026 BaseALT Ltd.
 * Copyright (C) 2026 Semyon Knyazev
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

#include "tree_state_manager.h"
#include "ui/widget/console/console_widget.h"
#include <QSettings>
#include "ad_config.h"
#include "core/globals.h"
#include "ui/console/object/console_object_operations.h"
#include "ui/console/policy_root_impl.h"
#include "ui/console/policy_impl.h"
#include "ui/console/policy_ou_impl.h"
#include "ui/console/all_policies_folder_impl.h"
#include "ui/console/query_folder_impl.h"
#include "ui/console/password_settings_impl.h"
#include <QTreeView>
#include "core/utils.h"
#include "ad_utils.h"
#include "core/settings.h"
#include <QAbstractProxyModel>
#include <QAbstractItemModel>
#include <utility>


TreeStateManager::TreeStateManager(ConsoleWidget *console, QTreeView *view, const QString &domain, const QString &user, QObject *parent) :
QObject(parent), console_(console), view_(view), domain_(domain), user_(user),
tree_state_prefix("tree_state/" + user + "/" + domain) {
    proxy_model_ = qobject_cast<QAbstractProxyModel*>(view_->model());
    source_model_ = proxy_model_ ? proxy_model_->sourceModel() : nullptr;

    connect(source_model_, &QAbstractItemModel::rowsInserted, this, &TreeStateManager::on_rows_inserted,
        Qt::QueuedConnection);
}

void TreeStateManager::set_context(const QString &domain, const QString &user) {
    domain_ = domain;
    user_ = user;
    tree_state_prefix = "tree_state/" + user + "/" + domain;
}

void TreeStateManager::save() {
    if (domain_.isEmpty() || user_.isEmpty() || !console_ || !view_ || !proxy_model_ || !source_model_) {
        return;
    }

    save_object_subtree(ConsoleObjectTreeOperations::get_domain_object_tree_root(console_),
        object_tree_key);
    save_policy_subtree();
    save_pso_subtree();
    save_object_subtree(ConsoleObjectTreeOperations::get_sites_container_tree_root(console_),
        sites_tree_key);
    save_query_subtree();
    save_current_selected_item();
}

void TreeStateManager::restore() {
    const bool context_match = g_adconfig->domain().toLower() == domain_ &&
                               g_adconfig->user() == user_;
    if (!proxy_model_ || !source_model_ || !console_ || !view_ || !context_match) {
        return;
    }

    item_selection_restored_ = false;

    restore_object_subtree();
    restore_policy_subtree();
    restore_pso_subtree();
    restore_sites_subtree();
    restore_queries_subtree();

    start_next_restore_task();
}

void TreeStateManager::on_rows_inserted(const QModelIndex &parent, int first, int last) {
    Q_UNUSED(first);
    Q_UNUSED(last);

    if (restore_task_active_) {
        if (waiting_parent_.isValid() && parent != waiting_parent_) {
            return;
        }

        continue_restore_task();
        return;
    }

    if (!restore_task_active_ && restore_task_queue_.isEmpty() &&
        !item_selection_restored_) {
        restore_current_selected_item();
    }
}

void TreeStateManager::enqueue_restore_task(const QModelIndex &root, const QStringList &name_list, int role,
        ItemType item_type, std::function<void()> on_finished) {
    if (!root.isValid()) {
        return;
    }

    ExpandTask task;
    task.root = root;
    task.role = role;
    task.item_type = item_type;
    task.on_finished = std::move(on_finished);

    switch (item_type) {
        case ItemType_Object:
            task.dn_queue = prepare_restore_dn_queue(
                root.data(ObjectRole_DN).toString(),
                name_list);
            break;

        case ItemType_PolicyOU:
            task.dn_queue = prepare_restore_dn_queue(
                g_adconfig->domain_dn(),
                name_list);
            break;

        default:
            task.dn_queue = prepare_restore_dn_queue(
                QString(),
                name_list);
            break;
    }

    restore_task_queue_.enqueue(std::move(task));
}

void TreeStateManager::start_next_restore_task() {
    if (restore_task_active_) {
        return;
    }

    if (restore_task_queue_.isEmpty()) {
        restore_current_selected_item();
        return;
    }

    waiting_parent_ = QPersistentModelIndex();
    restore_task_active_ = true;
    continue_restore_task();
}

void TreeStateManager::continue_restore_task() {
    if (!restore_task_active_) {
        return;
    }

    if (restore_task_queue_.isEmpty()) {
        restore_task_active_ = false;
        return;
    }

    ExpandTask &task = restore_task_queue_.head();

    if (task.dn_queue.isEmpty()) {
        finish_current_restore_task();
        return;
    }

    const QModelIndex idx = console_->search_item(
        task.root,
        task.role,
        task.dn_queue.head(),
        {task.item_type});

    if (!idx.isValid()) {
        // Wait if no corresponding model items are
        // still fetched
        return;
    }

    waiting_parent_ = idx;

    const QModelIndex proxy_idx = proxy_model_->mapFromSource(idx);
    if (!proxy_idx.isValid()) {
        return;
    }

    view_->setExpanded(proxy_idx, true);

    task.dn_queue.dequeue();

    if (task.dn_queue.isEmpty()) {
        finish_current_restore_task();
        return;
    }

    const bool fetched = idx.data(ConsoleRole_WasFetched).toBool();
    if (fetched) {
        continue_restore_task();
    }
}

void TreeStateManager::finish_current_restore_task() {
    if (restore_task_queue_.isEmpty()) {
        restore_task_active_ = false;
        waiting_parent_ = QPersistentModelIndex();
        return;
    }

    auto on_finished =
        std::move(restore_task_queue_.head().on_finished);

    restore_task_queue_.dequeue();
    restore_task_active_ = false;
    waiting_parent_ = QPersistentModelIndex();

    if (on_finished) {
        on_finished();
    }

    start_next_restore_task();
}

void TreeStateManager::save_object_subtree(const QModelIndex &parent, const QString &key) {
    if (!parent.isValid()) {
        return;
    }

    QStringList expanded_dn_list;

    const QModelIndex proxy_parent = proxy_model_->mapFromSource(parent);

    if (proxy_parent.isValid() && view_->isExpanded(proxy_parent)) {
        expanded_dn_list.append(parent.data(ObjectRole_DN).toString());

        const QList<QModelIndex> expanded_list =
            expanded_index_list(parent);

        expanded_dn_list.append(
            index_list_to_dn_list(expanded_list, ObjectRole_DN));
    }

    QSettings settings;
    settings.setValue(
        tree_state_prefix + key,
        expanded_dn_list);
}

void TreeStateManager::save_policy_subtree() {
    const QModelIndex policy_root = get_policy_tree_root(console_);
    if (!policy_root.isValid()) {
        return;
    }

    const QModelIndex proxy_policy_root =
        proxy_model_->mapFromSource(policy_root);

    const bool policy_root_expanded =
        proxy_policy_root.isValid() &&
        view_->isExpanded(proxy_policy_root);

    QSettings settings;

    settings.setValue(
        tree_state_prefix + policy_root_key,
        policy_root_expanded);

    auto expanded_list = expanded_index_list(policy_root);

    const QModelIndex all_policies_folder_idx =
        get_all_policies_folder_index(console_);

    const bool all_policies_expanded =
        all_policies_folder_idx.isValid() &&
        expanded_list.contains(all_policies_folder_idx);

    settings.setValue(
        tree_state_prefix + all_policies_folder_key,
        all_policies_expanded);

    if (all_policies_folder_idx.isValid()) {
        expanded_list.removeAll(all_policies_folder_idx);
    }

    QStringList expanded_dn_list =
        index_list_to_dn_list(
            expanded_list,
            PolicyOURole_DN);

    const QModelIndex domain_idx = console_->search_item(
            policy_root,
            PolicyOURole_DN,
            g_adconfig->domain_dn(),
            {ItemType_PolicyOU});

    if (domain_idx.isValid()) {
        const QModelIndex proxy_domain_idx =
            proxy_model_->mapFromSource(domain_idx);

        if (proxy_domain_idx.isValid() &&
            view_->isExpanded(proxy_domain_idx)) {

            const QString domain_dn =
                domain_idx.data(PolicyOURole_DN).toString();

            if (!domain_dn.isEmpty() &&
                !expanded_dn_list.contains(domain_dn)) {
                expanded_dn_list.prepend(domain_dn);
            }
        }
    }

    settings.setValue(
        tree_state_prefix + policy_tree_key,
        expanded_dn_list);
}

void TreeStateManager::save_pso_subtree() {
    const QModelIndex pso_root_proxy_idx = proxy_model_->mapFromSource(
        get_password_settings_tree_root(console_));
    if (!pso_root_proxy_idx.isValid()) {
        return;
    }

    const bool expanded = view_->isExpanded(pso_root_proxy_idx);
    QSettings settings;
    const QString settings_key = tree_state_prefix + pso_tree_key;
    settings.setValue(settings_key, expanded);
}

void TreeStateManager::save_query_subtree() {
    const QModelIndex queries_root = get_query_tree_root(console_);
    if (!queries_root.isValid()) {
        return;
    }

    QSettings settings;
    QStringList query_path_list;

    const QModelIndex proxy_root = proxy_model_->mapFromSource(queries_root);
    if (!proxy_root.isValid() || !view_->isExpanded(proxy_root)) {
        settings.setValue(
            tree_state_prefix + queries_tree_key,
            query_path_list);
        return;
    }

    query_path_list.append(QueryFolderImpl::QUERY_ROOT);

    const auto expanded_list = expanded_index_list(queries_root);
    const QStringList folder_list =
        settings_get_hash(SETTING_query_folders).keys();
    for (const QModelIndex &idx : expanded_list) {
        const QString path = console_query_folder_path(idx, console_);
        if (path.isEmpty() || !folder_list.contains(path)) {
            continue;
        }
        query_path_list.append(path);
    }

    settings.setValue(
        tree_state_prefix + queries_tree_key,
        query_path_list);
}

QList<QModelIndex> TreeStateManager::expanded_index_list(const QModelIndex &parent) {
    QList<QModelIndex> out;

    const QModelIndex proxy_parent = proxy_model_->mapFromSource(parent);

    if (parent.isValid() &&
        (!proxy_parent.isValid() || !view_->isExpanded(proxy_parent))) {
        return out;
    }

    const int row_count = source_model_->rowCount(parent);

    for (int row = 0; row < row_count; ++row) {
        const QModelIndex idx = source_model_->index(row, 0, parent);
        if (!idx.isValid()) {
            continue;
        }
        const QModelIndex proxy_idx = proxy_model_->mapFromSource(idx);

        const bool is_fetched = idx.data(ConsoleRole_WasFetched).toBool();
        const bool is_expanded = proxy_idx.isValid() && view_->isExpanded(proxy_idx);
        const bool has_children = source_model_->hasChildren(idx);

        if (!is_fetched || !is_expanded || !has_children) {
            continue;
        }

        const QList<QModelIndex> child_expanded_idx_list = expanded_index_list(idx);
        if (!child_expanded_idx_list.isEmpty()) {
            out.append(child_expanded_idx_list);
        }
        else {
            out.append(idx);
        }
    }

    return out;
}

void TreeStateManager::save_current_selected_item() {
    SelectedItemData data = selected_item_data();
    QSettings settings;
    const QString settings_key = tree_state_prefix + selected_item_key;
    const QVariantList value{data.first, data.second};
    settings.setValue(settings_key, value);
}

QQueue<QString> TreeStateManager::prepare_restore_dn_queue(const QString &base_dn, const QStringList &dn_list) {
    QQueue<QString> prepared_queue;

    QSet<QString> prepared_dn_set =
        base_dn.isEmpty()
            ? QSet<QString>{}
            : QSet<QString>{base_dn};

    if (!base_dn.isEmpty() && dn_list.contains(base_dn)) {
        prepared_queue.enqueue(base_dn);
    }

    for (const QString &dn : dn_list) {
        if (prepared_dn_set.contains(dn) ||
            (!base_dn.isEmpty() && !dn.contains(base_dn))) {
            continue;
        }

        const QStringList dn_splitted = dn.split(',');

        const int base_dn_parts_count =
            base_dn.isEmpty()
                ? 0
                : base_dn.split(',').size();

        if (dn_splitted.size() <= base_dn_parts_count) {
            continue;
        }

        QString current_dn = base_dn;

        for (int i = dn_splitted.size() - base_dn_parts_count - 1;
            i >= 0;
            --i) {

            current_dn = current_dn.isEmpty()
            ? dn_splitted[i]
              : dn_splitted[i] + "," + current_dn;

            if (!prepared_dn_set.contains(current_dn)) {
                prepared_dn_set.insert(current_dn);
                prepared_queue.enqueue(current_dn);
            }
        }
    }

    return prepared_queue;
}

void TreeStateManager::restore_object_subtree() {
    const QModelIndex object_root =
        ConsoleObjectTreeOperations::get_domain_object_tree_root(console_);

    if (!object_root.isValid()) {
        return;
    }

    QSettings settings;
    const QStringList restore_dn_list = settings.value(
                    tree_state_prefix + object_tree_key).toStringList();
    if (restore_dn_list.isEmpty()) {
        return;
    }

    enqueue_restore_task(
        object_root,
        restore_dn_list,
        ObjectRole_DN,
        ItemType_Object);
}

void TreeStateManager::restore_policy_subtree() {
    const QModelIndex policy_root_idx = get_policy_tree_root(console_);
    if (!policy_root_idx.isValid()) {
        return;
    }

    QSettings settings;
    const bool policy_root_expanded = settings.value(
                    tree_state_prefix + policy_root_key,
                    false).toBool();
    if (!policy_root_expanded) {
        return;
    }

    const QModelIndex proxy_policy_root = proxy_model_->mapFromSource(policy_root_idx);
    if (proxy_policy_root.isValid()) {
        view_->setExpanded(proxy_policy_root, true);
    }

    const QStringList policy_ou_dn_list = settings.value(
                    tree_state_prefix + policy_tree_key).toStringList();
    const bool all_policies_expanded = settings.value(
                    tree_state_prefix + all_policies_folder_key).toBool();

    enqueue_restore_task(
        policy_root_idx,
        policy_ou_dn_list,
        PolicyOURole_DN,
        ItemType_PolicyOU,
        [this, all_policies_expanded]() {
            if (!all_policies_expanded) {
                return;
            }

            const QModelIndex all_policies_idx =
                get_all_policies_folder_index(console_);
            if (!all_policies_idx.isValid()) {
                return;
            }

            const QModelIndex proxy_idx =
                proxy_model_->mapFromSource(all_policies_idx);
            if (proxy_idx.isValid()) {
                view_->setExpanded(proxy_idx, true);
            }
        });
}

void TreeStateManager::restore_queries_subtree() {
    const QModelIndex queries_root = get_query_tree_root(console_);
    if (!queries_root.isValid()) {
        return;
    }

    QSettings settings;
    const QStringList path_list =
        settings.value(tree_state_prefix + queries_tree_key).toStringList();
    if (path_list.isEmpty() || !path_list.contains(QueryFolderImpl::QUERY_ROOT)) {
        return;
    }

    const QModelIndex proxy_root =
        proxy_model_->mapFromSource(queries_root);
    if (!proxy_root.isValid()) {
        return;
    }
    view_->setExpanded(proxy_root, true);

    for (const QString &path : path_list) {
        if (path == QueryFolderImpl::QUERY_ROOT) {
            continue;
        }

        const QString prefix = QueryFolderImpl::QUERY_ROOT + "/";
        if (!path.startsWith(prefix)) {
            continue;
        }

        const QString relative_path = path.mid(prefix.size());

        const QStringList parts = relative_path.split('/', Qt::SkipEmptyParts);

        QModelIndex current = queries_root;
        for (const QString &part : parts) {
            QModelIndex child_idx;

            for (int row = 0; row < source_model_->rowCount(current); ++row) {
                const QModelIndex child = source_model_->index(row, 0, current);
                if (console_item_get_type(child) != ItemType_QueryFolder) {
                    continue;
                }

                if (child.data(Qt::DisplayRole).toString() == part) {
                    child_idx = child;
                    break;
                }
            }

            if (!child_idx.isValid()) {
                break;
            }

            const QModelIndex proxy_idx = proxy_model_->mapFromSource(child_idx);
            if (proxy_idx.isValid()) {
                view_->setExpanded(proxy_idx, true);
            }

            current = child_idx;
        }
    }
}

void TreeStateManager::restore_pso_subtree() {
    const QModelIndex pso_root = get_password_settings_tree_root(console_);
    if (!pso_root.isValid()) {
        return;
    }

    QSettings settings;
    const bool expanded = settings.value(tree_state_prefix + pso_tree_key).toBool();
    const QModelIndex pso_root_proxy = proxy_model_->mapFromSource(pso_root);
    if (pso_root_proxy.isValid() && expanded) {
        view_->setExpanded(pso_root_proxy, true);
    }
}

void TreeStateManager::restore_sites_subtree() {
    const QModelIndex sites_subtree_root = ConsoleObjectTreeOperations::get_sites_container_tree_root(console_);
    if (!sites_subtree_root.isValid()) {
        return;
    }

    QSettings settings;
    const QStringList dn_list_to_restore = settings.value(tree_state_prefix + sites_tree_key).toStringList();
    if (dn_list_to_restore.isEmpty()) {
        return;
    }

    enqueue_restore_task(sites_subtree_root, dn_list_to_restore, ObjectRole_DN, ItemType_Object);
}

void TreeStateManager::restore_current_selected_item() {
    QSettings settings;
    const QVariant selected_item_variant =
        settings.value(tree_state_prefix + selected_item_key);

    if (selected_item_variant.isNull()) {
        item_selection_restored_ = true;
        return;
    }

    const QVariantList selected_item_var_list =
        selected_item_variant.toList();

    if (selected_item_var_list.size() < 2) {
        item_selection_restored_ = true;
        return;
    }

    const ItemType type =
        static_cast<ItemType>(selected_item_var_list[0].toInt());
    const QString name_data =
        selected_item_var_list[1].toString();

    switch (type) {
        case ItemType_Object:
            restore_object_subtree_selection(name_data);
            break;
        case ItemType_PolicyOU:
        case ItemType_Policy:
            restore_policy_subtree_selection(name_data, type);
            break;
        case ItemType_QueryFolder:
        case ItemType_QueryItem:
            restore_queries_subtree_selection(name_data, type);
            break;
        case ItemType_AllPoliciesFolder:
        {
            const QModelIndex all_policies_idx =
                get_all_policies_folder_index(console_);
            if (all_policies_idx.isValid()) {
                console_->set_current_scope(all_policies_idx);
            }
            break;
        }
        case ItemType_PolicyRoot:
        {
            const QModelIndex policy_root =
                get_policy_tree_root(console_);
            if (policy_root.isValid()) {
                console_->set_current_scope(policy_root);
            }
            break;
        }
        default:
        {
            const QModelIndex domain_idx =
                console_->domain_info_index();

            if (domain_idx.isValid()) {
                console_->set_current_scope(domain_idx);
            }
            break;
        }
    }

    const SelectedItemData current_item =
        selected_item_data();
    item_selection_restored_ = current_item.first == type &&
        current_item.second == name_data;
}

void TreeStateManager::restore_object_subtree_selection(const QString &dn) {
    const QList<QPair<QString, QModelIndex>> subtrees = {
        {
            g_adconfig->pso_container_dn(),
            get_password_settings_tree_root(console_)
        },
        {
            g_adconfig->sites_container_dn(),
            ConsoleObjectTreeOperations::get_sites_container_tree_root(console_)
        },
        {
            g_adconfig->domain_dn(),
            ConsoleObjectTreeOperations::get_domain_object_tree_root(console_)
        }
    };

    for (const auto &subtree : subtrees) {
        const QString &subtree_dn = subtree.first;
        const QModelIndex &subtree_root = subtree.second;

        if (dn != subtree_dn && !dn.endsWith("," + subtree_dn)) {
            continue;
        }

        if (!subtree_root.isValid()) {
            continue;
        }

        if (dn == subtree_dn) {
            console_->set_current_scope(subtree_root);
            return;
        }

        const QModelIndex idx = console_->search_item(
            subtree_root,
            ObjectRole_DN,
            dn,
            {ItemType_Object});

        if (idx.isValid()) {
            console_->set_current_scope(idx);
        }

        return;
    }
}

void TreeStateManager::restore_policy_subtree_selection(const QString &dn, ItemType type) {
    const int role = type == ItemType_PolicyOU
                         ? static_cast<int>(PolicyOURole_DN)
                         : static_cast<int>(PolicyRole_DN);
    const QModelIndex policy_root =
        get_policy_tree_root(console_);

    if (!policy_root.isValid()) {
        return;
    }

    const QModelIndex selected_policy_idx =
        console_->search_item(
            policy_root,
            role,
            dn,
            {type});
    if (selected_policy_idx.isValid()) {
        console_->set_current_scope(selected_policy_idx);
    }
}

void TreeStateManager::restore_queries_subtree_selection(const QString &name, ItemType type) {
    const QModelIndex queries_root =
        get_query_tree_root(console_);
    if (!queries_root.isValid()) {
        return;
    }
    if (type == ItemType_QueryFolder &&
        name.isEmpty()) {
        console_->set_current_scope(queries_root);
        return;
    }
    const QString prefix =
        QueryFolderImpl::QUERY_ROOT + "/";
    if (!name.startsWith(prefix)) {
        return;
    }
    const QString relative_path =
        name.mid(prefix.size());

    const QStringList parts =
        relative_path.split('/', Qt::SkipEmptyParts);

    QModelIndex current = queries_root;

    for (int i = 0; i < parts.size(); ++i) {
        QModelIndex child_idx;

        const ItemType expected_type =
            i == parts.size() - 1
                ? type
                : ItemType_QueryFolder;

        for (int row = 0; row < source_model_->rowCount(current); ++row) {
            const QModelIndex child = source_model_->index(row, 0, current);

            if (console_item_get_type(child) != expected_type) {
                continue;
            }

            if (child.data(Qt::DisplayRole).toString() == parts[i]) {
                child_idx = child;
                break;
            }
        }
        if (!child_idx.isValid()) {
            return;
        }
        current = child_idx;
    }
    if (current.isValid() &&
        console_item_get_type(current) == type) {
        console_->set_current_scope(current);
    }
}

TreeStateManager::SelectedItemData TreeStateManager::selected_item_data() {
        const QModelIndex selected_idx = console_->get_current_scope_item();
        if (!selected_idx.isValid()) {
            return SelectedItemData{ItemType_Unassigned, QString()};
        }

        const ItemType item_type =
            static_cast<ItemType>(console_item_get_type(selected_idx));
        QString name_data;
        switch (item_type) {
            case ItemType_Object:
                name_data = selected_idx.data(ObjectRole_DN).toString();
                break;
            case ItemType_PolicyOU:
                name_data = selected_idx.data(PolicyOURole_DN).toString();
                break;
            case ItemType_Policy:
                name_data = selected_idx.data(PolicyRole_DN).toString();
                break;
            case ItemType_QueryFolder:
            {
                const QModelIndex queries_root = get_query_tree_root(console_);
                if (selected_idx != queries_root) {
                    name_data = console_query_folder_path(selected_idx, console_);
                }
                break;
            }
            case ItemType_QueryItem:
                name_data = console_query_folder_path(selected_idx, console_);
                break;
            case ItemType_PolicyRoot:
            case ItemType_AllPoliciesFolder:
            default:
                break;
        }

        return SelectedItemData{item_type, name_data};
}
