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

#ifndef TREE_STATE_MANAGER_H
#define TREE_STATE_MANAGER_H

#include <functional>

#include <QObject>
#include <QPair>
#include <QPersistentModelIndex>
#include <QQueue>
#include <QString>

#include "core/console_item_type.h"

class ConsoleWidget;
class QModelIndex;
class QTreeView;
class QAbstractItemModel;
class QAbstractProxyModel;

/**
 * @brief Saves and restores the state of the console tree.
 *
 * The manager persists expanded tree nodes and the current selection in
 * QSettings for a particular user/domain context. Tree restoration is
 * performed as a sequence of expansion tasks because parts of the source
 * model may be populated asynchronously. Pending restoration is resumed on
 * rowsInserted() notifications from the source model.
 */
class TreeStateManager final : public QObject {
    using SelectedItemData = QPair<ItemType, QString>;

    Q_OBJECT

public:
    /**
     * @brief Constructs a tree state manager.
     *
     * @param console Console used to access and select source-model items.
     * @param view Tree view whose expansion state is saved and restored.
     * @param domain Domain used as part of the settings context.
     * @param user User used as part of the settings context.
     * @param parent Parent QObject.
     */
    explicit TreeStateManager(ConsoleWidget *console, QTreeView *view,
        const QString &domain, const QString &user, QObject *parent);

    /**
     * @brief Changes the user/domain context used for persisted tree state.
     * @param domain New domain name.
     * @param user New user name.
     */
    void set_context(const QString &domain, const QString &user);

    /**
     * @brief Saves tree expansion state and the current selection.
     */
    void save();

    /**
     * @brief Restores tree expansion state and the current selection.
     *
     * Expansion tasks are processed sequentially. If the selected item is not
     * available when expansion finishes, selection restoration is retried when
     * new rows are inserted into the source model.
     */
    void restore();

private slots:
    /**
     * @brief Continues pending restoration after rows are inserted.
     *
     * During expansion restoration, unrelated insertions are ignored when a
     * specific parent is being awaited. After all expansion tasks are complete,
     * the same notification is used to retry selection restoration if needed.
     *
     * @param parent Parent index under which rows were inserted.
     * @param first First inserted row.
     * @param last Last inserted row.
     */
    void on_rows_inserted(const QModelIndex &parent, int first, int last);

private:
    /**
     * @brief Describes one sequential subtree expansion operation.
     */
    struct ExpandTask {
        QPersistentModelIndex root;       ///< Root used for item lookup.
        QQueue<QString> dn_queue;         ///< Item identifiers restored in order.
        int role = 0;                     ///< Model role used to match identifiers.
        ItemType item_type = ItemType_Unassigned; ///< Expected item type.
        std::function<void()> on_finished; ///< Optional completion callback.
    };

    ConsoleWidget *console_;
    QTreeView *view_;
    QString domain_;
    QString user_;
    QAbstractProxyModel *proxy_model_;
    QAbstractItemModel *source_model_;

    // Prefix and keys for tree state settings
    QString tree_state_prefix;
    const QString object_tree_key = "/objects";
    const QString policy_tree_key = "/policies";
    const QString policy_root_key = "/policy_root";
    const QString all_policies_folder_key = "/all_policies";
    const QString pso_tree_key = "/pso";
    const QString sites_tree_key = "/sites";
    const QString queries_tree_key = "/queries";
    const QString selected_item_key = "/selected_item";

    QQueue<ExpandTask> restore_task_queue_;
    bool restore_task_active_ = false;
    QPersistentModelIndex waiting_parent_; ///< Parent whose children are currently awaited.
    bool item_selection_restored_ = false; ///< Whether the saved current item was restored.

    /**
     * @brief Adds a subtree expansion task to the restore queue.
     * @param root Root index used for item lookup.
     * @param name_list Saved identifiers to restore.
     * @param role Model role containing the identifier.
     * @param item_type Expected item type.
     * @param on_finished Optional callback invoked after the task completes.
     */
    void enqueue_restore_task(const QModelIndex &root, const QStringList &name_list, int role,
        ItemType item_type, std::function<void()> on_finished = {});

    /** @brief Starts the next queued expansion task or restores selection. */
    void start_next_restore_task();

    /** @brief Continues processing the active expansion task. */
    void continue_restore_task();

    /** @brief Completes the active expansion task and starts the next one. */
    void finish_current_restore_task();

    /**
     * @brief Saves expanded nodes of an object subtree.
     * @param parent Root of the subtree.
     * @param key Settings key suffix used to store the state.
     */
    void save_object_subtree(const QModelIndex &parent, const QString &key);

    /** @brief Saves expansion state of the policy subtree. */
    void save_policy_subtree();

    /** @brief Saves expansion state of the password settings subtree. */
    void save_pso_subtree();

    /** @brief Saves expansion state of the query subtree. */
    void save_query_subtree();

    /**
     * @brief Collects the deepest expanded and fetched nodes below a parent.
     * @param parent Parent index to inspect.
     * @return Expanded source-model indexes representing saved subtree state.
     */
    QList<QModelIndex> expanded_index_list(const QModelIndex &parent);

    /** @brief Saves the current console selection. */
    void save_current_selected_item();

    /**
     * @brief Returns persistent data identifying the current console item.
     *
     * AD objects and policies are identified by DN, query items by path, and
     * special root/folder items by ItemType only.
     *
     * @return Pair containing the selected item type and its persistent name.
     */
    SelectedItemData selected_item_data();

    /**
     * @brief Builds an ordered queue of nodes that must be expanded.
     *
     * Missing ancestors between @p base_dn and each saved DN are inserted so
     * that lazy subtrees can be restored from parent to child.
     *
     * @param base_dn DN of the subtree root, or an empty string for non-DN roots.
     * @param dn_list Saved DNs to restore.
     * @return Ordered queue of identifiers to process.
     */
    QQueue<QString> prepare_restore_dn_queue(const QString &base_dn, const QStringList &dn_list);

    /** @brief Enqueues restoration of the domain object subtree. */
    void restore_object_subtree();

    /** @brief Restores policy-root state and enqueues policy subtree restoration. */
    void restore_policy_subtree();

    /** @brief Restores expansion state of the password settings subtree. */
    void restore_pso_subtree();

    /** @brief Enqueues restoration of the sites subtree. */
    void restore_sites_subtree();

    /** @brief Restores expansion state of saved query folders. */
    void restore_queries_subtree();

    /**
     * @brief Attempts to restore the saved current item.
     *
     * The result is recorded in item_selection_restored_. A failed attempt may
     * be retried by on_rows_inserted() after the expansion queue is exhausted.
     */
    void restore_current_selected_item();

    /**
     * @brief Restores selection of an object-tree item by DN.
     * @param dn Distinguished name of the saved object.
     */
    void restore_object_subtree_selection(const QString &dn);

    /**
     * @brief Restores selection of a policy or policy OU by DN.
     * @param dn Distinguished name of the saved policy item.
     * @param type Saved policy item type.
     */
    void restore_policy_subtree_selection(const QString &dn, ItemType type);

    /**
     * @brief Restores selection of a query item or folder by saved path.
     * @param name Saved query path.
     * @param type Saved query item type.
     */
    void restore_queries_subtree_selection(const QString &name, ItemType type);
};

#endif // TREE_STATE_MANAGER_H
