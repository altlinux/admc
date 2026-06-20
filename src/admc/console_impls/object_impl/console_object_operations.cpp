#include "console_impls/object_impl/console_object_operations.h"
#include "console_impls/object_impl/object_impl.h"
#include "console_impls/item_type.h"
#include "globals.h"
#include "managers/icon_manager.h"
#include "console_impls/policy_ou_impl.h"
#include "console_impls/policy_root_impl.h"
#include "console_impls/query_folder_impl.h"
#include <QModelIndex>
#include "utils.h"
#include <QStandardItem>
#include "settings.h"
#include "ad_object.h"
#include "search_thread.h"
#include "create_dialogs/create_computer_dialog.h"
#include "create_dialogs/create_contact_dialog.h"
#include "create_dialogs/create_group_dialog.h"
#include "create_dialogs/create_ou_dialog.h"
#include "create_dialogs/create_shared_folder_dialog.h"
#include "create_dialogs/create_user_dialog.h"
#include "find_widgets/find_object_dialog.h"
#include "rename_dialogs/rename_group_dialog.h"
#include "rename_dialogs/rename_object_dialog.h"
#include "rename_dialogs/rename_other_dialog.h"
#include "rename_dialogs/rename_user_dialog.h"
#include "select_dialogs/select_container_dialog.h"
#include "select_dialogs/select_object_dialog.h"
#include "console_impls/find_object_impl.h"
#include "status.h"
#include "properties_widgets/properties_dialog.h"
#include "properties_widgets/properties_multi_dialog.h"
#include "create_dialogs/create_pso_dialog.h"
#include <QMessageBox>
#include "create_dialogs/create_site_dialog.h"
#include "create_dialogs/create_subnet_dialog.h"
#include "create_dialogs/create_sites_link_dialog.h"
#include "console_impls/object_impl/site_dn_attrs_updater.h"
#include "console_impls/object_impl/server_dn_attrs_updater.h"

void ConsoleObjectTreeOperations::console_object_move_and_rename(const QList<ConsoleWidget *> &console_list,
                                                      AdInterface &ad,
                                                      const QHash<QString, QString> &old_to_new_dn_map_arg,
                                                      const QString &new_parent_dn) {

    // NOTE: sometimes, some objects that are supposed
    // to be moved don't actually need to be. For
    // example, if an object were to be moved to an
    // object that is already it's current parent. If
    // we were to move them that would cause gui
    // glitches because that kind of move is not
    // considered in the update logic. Instead, we skip
    // these kinds of objects.
    const QHash<QString, QString> &old_to_new_dn_map = [&]() {
        QHash<QString, QString> out;

        for (const QString &old_dn : old_to_new_dn_map_arg.keys()) {
            const QString new_dn = old_to_new_dn_map_arg[old_dn];
            const bool dn_changed = (new_dn != old_dn);

            if (dn_changed) {
                out[old_dn] = new_dn;
            }
        }

        return out;
    }();

    const QList<QString> old_dn_list = old_to_new_dn_map.keys();
    const QList<QString> new_dn_list = old_to_new_dn_map.values();

    // NOTE: search for objects once here to reuse them
    // multiple times later
    const QHash<QString, AdObject> object_map = [&]() {
        QHash<QString, AdObject> out;

        for (const QString &dn : new_dn_list) {
            const AdObject object = ad.search_object(dn);
            out[dn] = object;
        }

        return out;
    }();

    auto apply_changes = [&old_to_new_dn_map, &old_dn_list, &new_parent_dn, &object_map](ConsoleWidget *target_console) {
        // For object tree, we add items representing
        // updated objects and delete old items. In the case
        // of move, this moves the items to their new
        // parent. In the case of rename, this updates
        // item's information about the object by recreating
        // it.

        // NOTE: delete old item AFTER adding new item
        // because: If old item is deleted first, then it's
        // possible for new parent to get selected (if they
        // are next to each other in scope tree). Then what
        // happens is that due to new parent being selected,
        // it gets fetched and loads new object. End result
        // is that new object is duplicated.
        const QModelIndex object_root = new_parent_dn.contains(g_adconfig->sites_container_dn()) ?
                    get_sites_container_tree_root(target_console) :
                    get_domain_object_tree_root(target_console);
        if (object_root.isValid()) {
            console_object_delete_dn_list(target_console, old_dn_list, object_root, ItemType_Object, ObjectRole_DN);

            const QModelIndex new_parent_obj_index = target_console->search_item(object_root, ObjectRole_DN, new_parent_dn, {ItemType_Object});
            add_objects_to_console(target_console, object_map.values(), new_parent_obj_index);
        }

        // For query tree, we don't move items or recreate
        // items but instead update items. Move doesn't make
        // sense because query tree doesn't represent the
        // object tree. Note that changing name or DN of
        // objects may mean that the parent query may become
        // invalid. For example, if query searches for all
        // objects with names starting with "foo" and we
        // rename an object in that query so that it doesn't
        // start with "foo" anymore. Since it is too
        // complicated to determine match criteria on the
        // client, and refreshing the query is too wasteful,
        // we instead mark queries with modified objects as
        // "out of date". It is then up to the user to
        // refresh the query if needed.
        const QModelIndex query_root = get_query_tree_root(target_console);
        if (query_root.isValid()) {
            // Find indexes of modified objects in query
            // tree
            const QHash<QString, QModelIndex> index_map = [&]() {
                QHash<QString, QModelIndex> out;

                for (const QString &old_dn : old_dn_list) {
                    const QList<QModelIndex> results = target_console->search_items(query_root, ObjectRole_DN, old_dn, {ItemType_Object});

                    for (const QModelIndex &index : results) {
                        out[old_dn] = index;
                    }
                }

                return out;
            }();

            for (const QString &old_dn : index_map.keys()) {
                const QModelIndex index = index_map[old_dn];

                // Mark parent query as "out of date". Icon
                // and tooltip will be restored after
                // refresh
                const QModelIndex query_index = index.parent();
                QStandardItem *item = target_console->get_item(query_index);
                item->setIcon(g_icon_manager->item_icon(ItemIcon_Warning_Indicator));
                item->setToolTip(QCoreApplication::translate("ObjectImpl", "Query may be out of date"));

                // Update item row
                const QList<QStandardItem *> row = target_console->get_row(index);
                const QString new_dn = old_to_new_dn_map[old_dn];
                const AdObject object = object_map[new_dn];
                console_object_load(row, object);
            }
        }

        // TODO: decrease code duplication
        // For find tree, we only reload the rows to
        // update name, and attributes (DN for example)
        const QModelIndex find_object_root = get_query_tree_root(target_console);
        if (find_object_root.isValid()) {
            // Find indexes of modified objects in find
            // tree
            const QHash<QString, QModelIndex> index_map = [&]() {
                QHash<QString, QModelIndex> out;

                for (const QString &old_dn : old_dn_list) {
                    const QList<QModelIndex> results = target_console->search_items(find_object_root, ObjectRole_DN, old_dn, {ItemType_Object});

                    for (const QModelIndex &index : results) {
                        out[old_dn] = index;
                    }
                }

                return out;
            }();

            for (const QString &old_dn : index_map.keys()) {
                const QModelIndex index = index_map[old_dn];

                // Update item row
                const QList<QStandardItem *> row = target_console->get_row(index);
                const QString new_dn = old_to_new_dn_map[old_dn];
                const AdObject object = object_map[new_dn];
                console_object_load(row, object);
            }
        }

        // Apply changes to policy tree
        const QModelIndex policy_root = get_policy_tree_root(target_console);
        if (policy_root.isValid()) {
            const QModelIndex new_parent_index = target_console->search_item(policy_root, PolicyOURole_DN, new_parent_dn, {ItemType_PolicyOU});

            if (new_parent_index.isValid()) {
                policy_ou_impl_add_objects_to_console(target_console, object_map.values(), new_parent_index);

                console_object_delete_dn_list(target_console, old_dn_list, policy_root, ItemType_PolicyOU, PolicyOURole_DN);
            }
        }
    };

    for (ConsoleWidget *console : console_list) {
        apply_changes(console);
    }

    // Fix dn attrs which contain site/server not actual DNs
    // after move/rename
    for (auto obj : object_map.values()) {
        if (obj.get_string(ATTRIBUTE_OBJECT_CLASS) == CLASS_SITE) {
            const QString new_dn = obj.get_dn();
            const QString old_dn = old_to_new_dn_map.key(new_dn, QString());
            SiteDnAttrsUpdater(old_dn).update_for_rename(ad, new_dn);
        } else if (obj.get_string(ATTRIBUTE_OBJECT_CLASS) == CLASS_SERVER) {
            const QString new_dn = obj.get_dn();
            const QString old_dn = old_to_new_dn_map.key(new_dn, QString());
            ServerDnAttrsUpdater(old_dn).update_for_move(ad, new_dn);
        } else {
            continue;
        }
    }
}

void ConsoleObjectTreeOperations::console_object_delete_dn_list(ConsoleWidget *console, const QList<QString> &dn_list, const QModelIndex &tree_root, const int type, const int dn_role) {
    for (const QString &dn : dn_list) {
        const QList<QModelIndex> index_list = console->search_items(tree_root, dn_role, dn, {type});
        const QList<QPersistentModelIndex> persistent_list = persistent_index_list(index_list);

        for (const QPersistentModelIndex &index : persistent_list) {
            console->delete_item(index);
        }
    }
}

void ConsoleObjectTreeOperations::add_objects_to_console(ConsoleWidget *console, const QList<AdObject> &object_list, const QModelIndex &parent) {
    if (!parent.isValid()) {
        return;
    }

    // NOTE: don't add if parent wasn't fetched yet. If that
    // is the case then the object will be added naturally
    // when parent is fetched.
    const bool parent_was_fetched = console_item_get_was_fetched(parent);
    if (!parent_was_fetched) {
        return;
    }

    for (const AdObject &object : object_list) {
        if (object.is_empty())
            continue;
        const bool should_be_in_scope = [&]() {
            // NOTE: "containers" referenced here don't mean
            // objects with "container" object class.
            // Instead it means all the objects that can
            // have children(some of which are not
            // "container" class).
            const QString object_class = object.get_string(ATTRIBUTE_OBJECT_CLASS);
            const bool is_container = [=]() {
                const QList<QString> filter_containers = g_adconfig->get_filter_containers();

                return filter_containers.contains(object_class);
            }();

            const bool show_non_containers_ON = settings_get_variant(SETTING_show_non_containers_in_console_tree).toBool();
            const bool is_site_related = g_adconfig->get_site_related_classes().contains(object_class);

            return (is_container || show_non_containers_ON || is_site_related || object_class == CLASS_PSO);
        }();

        const QList<QStandardItem *> row = [&]() {
            if (should_be_in_scope) {
                return console->add_scope_item(ItemType_Object, parent);
            } else {
                return console->add_results_item(ItemType_Object, parent);
            }
        }();

        if (object.get_string(ATTRIBUTE_OBJECT_CLASS) == CLASS_SITE) {
            console->set_item_sort_index(row[0]->index(), 1);
        }

        console_object_load(row, object);
    }
}

void ConsoleObjectTreeOperations::add_objects_to_console_from_dn_list(ConsoleWidget *console, AdInterface &ad, const QList<QString> &dn_list, const QModelIndex &parent) {
    const QList<AdObject> object_list = [&]() {
        QList<AdObject> out;

        for (const QString &dn : dn_list) {
            const AdObject object = ad.search_object(dn);
            out.append(object);
        }

        return out;
    }();

    ConsoleObjectTreeOperations::add_objects_to_console(console, object_list, parent);
}

void ConsoleObjectTreeOperations::console_object_load(const QList<QStandardItem *> row, const AdObject &object) {
    // Load attribute columns
    for (int i = 0; i < g_adconfig->get_columns().count(); i++) {
        if (g_adconfig->get_columns().count() > row.size()) {
            break;
        }

        const QString attribute = g_adconfig->get_columns()[i];

        if (!object.contains(attribute)) {
            continue;
        }

        const QString display_value = [attribute, object]() {
            if (attribute == ATTRIBUTE_OBJECT_CLASS) {
                const QString object_class = object.get_string(attribute);

                if (object_class == CLASS_GROUP) {
                    const GroupScope scope = object.get_group_scope();
                    const QString scope_string = group_scope_string(scope);

                    const GroupType type = object.get_group_type();
                    const QString type_string = group_type_string_adjective(type);

                    return QString("%1 - %2").arg(type_string, scope_string);
                } else {
                    return g_adconfig->get_class_display_name(object_class);
                }
            } else {
                const QByteArray value = object.get_value(attribute);
                return attribute_display_value(attribute, value, g_adconfig);
            }
        }();

        row[i]->setText(display_value);
    }

    console_object_item_data_load(row[0], object);

    const bool cannot_move = object.get_system_flag(SystemFlagsBit_DomainCannotMove);

    for (auto item : row) {
        item->setDragEnabled(!cannot_move);
    }
}

void ConsoleObjectTreeOperations::console_object_item_data_load(QStandardItem *item, const AdObject &object) {
    item->setData(object.get_dn(), ObjectRole_DN);

    const QList<QString> object_classes = object.get_strings(ATTRIBUTE_OBJECT_CLASS);
    item->setData(QVariant(object_classes), ObjectRole_ObjectClasses);

    const QString object_category = object.get_string(ATTRIBUTE_OBJECT_CATEGORY);
    item->setData(object_category, ObjectRole_ObjectCategory);

    const bool cannot_move = object.get_system_flag(SystemFlagsBit_DomainCannotMove);
    item->setData(cannot_move, ObjectRole_CannotMove);

    const bool cannot_rename = object.get_system_flag(SystemFlagsBit_DomainCannotRename);
    item->setData(cannot_rename, ObjectRole_CannotRename);

    const bool cannot_delete = object.get_system_flag(SystemFlagsBit_CannotDelete);
    item->setData(cannot_delete, ObjectRole_CannotDelete);

    const bool account_disabled = object.get_account_option(AccountOption_Disabled, g_adconfig);
    item->setData(account_disabled, ObjectRole_AccountDisabled);

    console_object_item_load_icon(item, account_disabled);
}

void ConsoleObjectTreeOperations::console_object_item_load_icon(QStandardItem *item, bool disabled) {
    const QString category = dn_get_name(item->data(ObjectRole_ObjectCategory).toString());
    QIcon icon;

    if (item->data(ConsoleRole_Type).toInt() == ItemType_QueryItem) {
        icon = g_icon_manager->category_icon(ADMC_CATEGORY_QUERY_ITEM);
        item->setIcon(icon);
        return;
    }

    if (category == OBJECT_CATEGORY_PERSON) {
        icon = disabled ? g_icon_manager->item_icon(ItemIcon_Person_Blocked) :
                          g_icon_manager->item_icon(ItemIcon_Person);
        item->setIcon(icon);
        return;
    }
    else if (category == OBJECT_CATEGORY_COMPUTER) {
        icon = disabled ? g_icon_manager->item_icon(ItemIcon_Computer_Blocked) :
                          g_icon_manager->item_icon(ItemIcon_Computer);
        item->setIcon(icon);
        return;
    }
    else if (category == OBJECT_CATEGORY_GROUP) {
        icon = g_icon_manager->item_icon(ItemIcon_Group);
        item->setIcon(icon);
        return;
    }

    icon = g_icon_manager->category_icon(category);
    item->setIcon(icon);
}

void ConsoleObjectTreeOperations::console_object_search(ConsoleWidget *console, const QModelIndex &index, const QString &base, const SearchScope scope, const QString &filter, const QList<QString> &attributes) {
    auto search_id_matches = [](QStandardItem *item, SearchThread *thread) {
        const int id_from_item = item->data(MyConsoleRole_SearchThreadId).toInt();
        const int thread_id = thread->get_id();

        const bool match = (id_from_item == thread_id);

        return match;
    };

    QStandardItem *item = console->get_item(index);

    // Set icon to indicate that item is in "search" state
    item->setIcon(g_icon_manager->item_icon(ItemIcon_Search_Indicator));

    // NOTE: need to set this role to disable actions during
    // fetch
    item->setData(true, ObjectRole_Fetching);
    item->setDragEnabled(false);

    auto search_thread = new SearchThread(base, scope, filter, attributes);

    // NOTE: change item's search thread, this will be used
    // later to handle situations where a thread is started
    // while another is running
    item->setData(search_thread->get_id(), MyConsoleRole_SearchThreadId);

    const QPersistentModelIndex persistent_index = index;

    // NOTE: need to pass console as receiver object to
    // connect() even though we're using lambda as a slot.
    // This is to be able to define queuedconnection type,
    // because there's no connect() version with no receiver
    // which has a connection type argument.
    QObject::connect(
        search_thread, &SearchThread::results_ready,
        console,
        [=](const QHash<QString, AdObject> &results) {
            // NOTE: fetched index might become invalid for
            // many reasons, parent getting moved, deleted,
            // item at the index itself might get modified.
            // Since this slot runs in the main thread, it's
            // not possible for any catastrophic conflict to
            // happen, so it's enough to just stop the
            // search.
            if (!persistent_index.isValid()) {
                search_thread->stop();

                return;
            }

            QStandardItem *item_now = console->get_item(persistent_index);

            // NOTE: if another thread was started for this
            // item, abort this thread
            const bool thread_id_match = search_id_matches(item_now, search_thread);
            if (!thread_id_match) {
                search_thread->stop();

                return;
            }

            ConsoleObjectTreeOperations::add_objects_to_console(console, results.values(), persistent_index);
        },
        Qt::QueuedConnection);
    QObject::connect(
        search_thread, &SearchThread::finished,
        console,
        [=]() {
            if (!persistent_index.isValid()) {
                return;
            }

            g_status->display_ad_messages(search_thread->get_ad_messages(), console);
            search_thread_display_errors(search_thread, console);

            QStandardItem *item_now = console->get_item(persistent_index);

            // NOTE: if another thread was started for this
            // item, don't change item data. It will be
            // changed by that other thread.
            const bool thread_id_match = search_id_matches(item_now, search_thread);
            if (!thread_id_match) {
                return;
            }

            const bool is_disabled = item_now->data(ObjectRole_AccountDisabled).toBool();
            console_object_item_load_icon(item_now, is_disabled);

            item_now->setData(false, ObjectRole_Fetching);
            item_now->setDragEnabled(true);

            search_thread->deleteLater();
        },
        Qt::QueuedConnection);

    search_thread->start();
}

QList<QString> ConsoleObjectTreeOperations::object_impl_column_labels() {
    QList<QString> out;

    for (const QString &attribute : g_adconfig->get_columns()) {
        const QString attribute_display_name = g_adconfig->get_column_display_name(attribute);

        out.append(attribute_display_name);
    }

    return out;
}

QList<int> ConsoleObjectTreeOperations::object_impl_default_columns() {
    // By default show first 3 columns: name, class and
    // description
    return {0, 1, 2};
}

QList<QString> ConsoleObjectTreeOperations::console_object_search_attributes() {
    QList<QString> attributes;

    attributes += g_adconfig->get_columns();

    // NOTE: needed for loading group type/scope into "type"
    // column
    attributes += ATTRIBUTE_GROUP_TYPE;

    // NOTE: system flags are needed to disable
    // delete/move/rename for objects that can't do those
    // actions
    attributes += ATTRIBUTE_SYSTEM_FLAGS;

    attributes += ATTRIBUTE_USER_ACCOUNT_CONTROL;

    // NOTE: needed to know which icon to use for object
    attributes += ATTRIBUTE_OBJECT_CATEGORY;

    // NOTE: for context menu block inheritance checkbox
    attributes += ATTRIBUTE_GPOPTIONS;

    // NOTE: needed to know gpo status
    attributes += ATTRIBUTE_FLAGS;

    return attributes;
}

void ConsoleObjectTreeOperations::console_object_tree_init(ConsoleWidget *console, AdInterface &ad) {
    const QList<QStandardItem *> row = console->add_scope_item(ItemType_Object, console->domain_info_index());
    auto root = row[0];

    const QString top_dn = g_adconfig->domain_dn();
    const AdObject top_object = ad.search_object(top_dn);
    console_object_item_data_load(root, top_object);

    const QString domain = g_adconfig->domain().toLower();
    root->setText(domain);
    console->set_item_sort_index(row[0]->index(), 0);
}

QModelIndex ConsoleObjectTreeOperations::get_domain_object_tree_root(ConsoleWidget *console) {
    return get_object_tree_root(console, g_adconfig->domain_dn());
}

bool ConsoleObjectTreeOperations::console_object_deletion_dialog(ConsoleWidget *console, const QList<QModelIndex> &index_deleted_list) {
    QString main_message;
    if (index_deleted_list.size() == 1) {
        main_message = QCoreApplication::translate("ObjectImpl", "Are you sure you want to delete this object?");
    }
    else {
        main_message = QCoreApplication::translate("ObjectImpl", "Are you sure you want to delete these objects?");
    }

    QString contains_objects_message;
    int not_empty_containers_count = 0;
    for (QModelIndex index : index_deleted_list) {
        if (index.model()->hasChildren(index)) {
            ++not_empty_containers_count;
        }
    }
    if (not_empty_containers_count == 1 && index_deleted_list.size() == 1) {
        contains_objects_message = QCoreApplication::translate("ObjectImpl", " It contains other objects.");
    }
    else if (not_empty_containers_count >= 1) {
        contains_objects_message = QCoreApplication::translate("ObjectImpl", " Containers to be deleted contain other objects.");
    }

    const bool confirm_actions = settings_get_variant(SETTING_confirm_actions).toBool();
    if (not_empty_containers_count > 0 || confirm_actions) {
        QMessageBox::StandardButton answer = QMessageBox::question(console, QObject::tr("Confirm action"), main_message + contains_objects_message);
        return answer == QMessageBox::Yes;
    }
    return true;
}

void ConsoleObjectTreeOperations::console_tree_add_password_settings(ConsoleWidget *console, AdInterface &ad) {
    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_PSO_CONTAINER);
    auto search_results = ad.search(g_adconfig->domain_dn(), SearchScope_All, filter, {});
    const QString err = QObject::tr("Password settings container is not available");
    if (search_results.isEmpty() || search_results.values()[0].is_empty()) {
        g_status->add_message(err, StatusType_Info);
        return;
    }

    const int pso_container_sort_idx = 3;
    console_tree_add_root_child(console, search_results.values()[0], pso_container_sort_idx,
                                QObject::tr("Fine-grained password policies"));
}

QString ConsoleObjectTreeOperations::console_object_count_string(ConsoleWidget *console, const QModelIndex &index) {
    const int count = console->get_child_count(index);
    const QString out = QCoreApplication::translate("object_impl", "%n object(s)", "", count);

    return out;
}

void ConsoleObjectTreeOperations::console_object_create(const QList<ConsoleWidget *> &console_list, const QString &object_class, const QString &parent_dn) {
    AdInterface ad;
    if (ad_failed(ad, console_list[0])) {
        return;
    }

    // NOTE: creating dialogs here instead of directly
    // in "on_new_x" slots looks backwards but it's
    // necessary to avoid even more code duplication
    // due to having to pass "ad" and "parent_dn" args
    // to dialog ctors
    CreateObjectDialog *dialog = create_dialog(object_class, ad, parent_dn, console_list[0]);

    if (dialog == nullptr) {
        return;
    }

    dialog->open();

    QObject::connect(
        dialog, &QDialog::accepted,
        console_list[0],
        [console_list, dialog, parent_dn, object_class]() {   // TODO: Refactor that lambda madness
            AdInterface ad_inner;
            if (ad_failed(ad_inner, console_list[0])) {
                return;
            }

            show_busy_indicator();

            const QString created_dn = dialog->get_created_dn();

            // NOTE: we don't just use currently selected index as
            // parent to add new object onto, because you can create
            // an object in a query tree or find dialog. Therefore
            // need to search for index of parent object in domain
            // tree.
            auto apply_changes = [&](ConsoleWidget *target_console) {
                if (object_class == CLASS_SITE) {
                    const QModelIndex site_root = get_sites_container_tree_root(target_console);
                    add_objects_to_console_from_dn_list(target_console, ad_inner, {created_dn}, site_root);
                    return;
                }
                else if (object_class == CLASS_SITE_LINK ||
                           object_class == CLASS_SITE_LINK_BRIDGE || object_class == CLASS_SUBNET) {
                    const QModelIndex site_root = get_sites_container_tree_root(target_console);
                    const QModelIndex parent_index = target_console->search_item(site_root, ObjectRole_DN, parent_dn, {ItemType_Object});
                    if (parent_index.isValid()) {
                        add_objects_to_console_from_dn_list(target_console, ad_inner, {created_dn}, parent_index);
                    }
                    return;
                }

                if (object_class == CLASS_PSO) {
                    const QModelIndex pso_root = get_pso_container_tree_root(target_console);
                    add_objects_to_console_from_dn_list(target_console, ad_inner, {created_dn}, pso_root);
                    if (console_item_get_was_fetched(pso_root)) {
                        target_console->refresh_scope(pso_root);
                    }
                }

                const QModelIndex object_root = get_domain_object_tree_root(target_console);
                if (object_root.isValid()) {
                    const QModelIndex parent_index = target_console->search_item(object_root, ObjectRole_DN, parent_dn, {ItemType_Object});

                    if (parent_index.isValid()) {
                        add_objects_to_console_from_dn_list(target_console, ad_inner, {created_dn}, parent_index);
                    }
                }

                // NOTE: changes are not applied to
                // find tree because creation of
                // objects shouldn't affect find
                // results. If it does affect them by
                // creating a new object that fits
                // search criteria, then find dialog
                // can just be considered out of date
                // and should be updated

                // Apply changes to policy tree
                const QModelIndex policy_root = get_policy_tree_root(target_console);
                if (policy_root.isValid() && object_class == CLASS_OU) {
                    const QModelIndex parent_ou_index = target_console->search_item(policy_root, PolicyOURole_DN, parent_dn, {ItemType_PolicyOU});

                    if (parent_ou_index.isValid()) {
                        policy_ou_impl_add_objects_from_dns(target_console, ad_inner, {created_dn}, parent_ou_index);
                    }
                }
            };

            for (ConsoleWidget *console : console_list) {
                apply_changes(console);
            }

            hide_busy_indicator();
        });
}


void ConsoleObjectTreeOperations::console_object_rename(const QList<ConsoleWidget *> &console_list, const QList<QModelIndex> &index_list, const int dn_role, const QString &object_class) {
    AdInterface ad;
    if (ad_failed(ad, console_list[0])) {
        return;
    }

    const QString old_dn = [&]() {
        if (!index_list.isEmpty()) {
            const QModelIndex index = index_list[0];
            const QString out = index.data(dn_role).toString();

            return out;
        } else {
            return QString();
        }
    }();

    RenameObjectDialog *dialog = [&]() -> RenameObjectDialog * {
        const bool is_user = (object_class == CLASS_USER);
        const bool is_group = (object_class == CLASS_GROUP);

        if (is_user) {
            return new RenameUserDialog(ad, old_dn, console_list[0]);
        } else if (is_group) {
            return new RenameGroupDialog(ad, old_dn, console_list[0]);
        } else {
            return new RenameOtherDialog(ad, old_dn, console_list[0]);
        }
    }();

    dialog->open();

    QObject::connect(
        dialog, &QDialog::accepted,
        console_list[0],
        [console_list, dialog, old_dn]() {
            AdInterface ad_inner;
            if (ad_failed(ad_inner, console_list[0])) {
                return;
            }

            const QString new_dn = dialog->get_new_dn();
            const QString parent_dn = dn_get_parent(old_dn);

            ConsoleObjectTreeOperations::console_object_move_and_rename(console_list, ad_inner, {{old_dn, new_dn}}, parent_dn);
        });
    }

void ConsoleObjectTreeOperations::console_object_delete(const QList<ConsoleWidget *> &console_list, const QList<QModelIndex> &index_list, const int dn_role) {
    const bool confirmed = console_object_deletion_dialog(console_list[0], index_list);
    if (!confirmed) {
        return;
    }

    AdInterface ad;
    if (ad_failed(ad, console_list[0])) {
        return;
    }

    show_busy_indicator();

    QList<QString> deleted_list;

    for (const QModelIndex &idx : index_list) {
        const QString target_dn = idx.data(dn_role).toString();
        const QString obj_class = idx.data(ObjectRole_ObjectClasses).toStringList().last();

        const bool success = ad.object_delete(target_dn);
        if (success) {
            if (obj_class == CLASS_SITE) {
                SiteDnAttrsUpdater(target_dn).update_for_delete(ad);
            } else if (obj_class == CLASS_SERVER) {
                ServerDnAttrsUpdater(target_dn).update_for_delete(ad);
            }

            deleted_list.append(target_dn);
        }
    }


    auto apply_changes = [&deleted_list](ConsoleWidget *target_console) {
        const QList<QModelIndex> root_list = {
            get_domain_object_tree_root(target_console),
            get_pso_container_tree_root(target_console),
            get_sites_container_tree_root(target_console),
            get_query_tree_root(target_console),
            get_find_object_root(target_console),
        };

        for (const QModelIndex &root : root_list) {
            if (root.isValid()) {
                ConsoleObjectTreeOperations::console_object_delete_dn_list(target_console, deleted_list, root, ItemType_Object, ObjectRole_DN);
            }
        }

        const QModelIndex policy_root = get_policy_tree_root(target_console);
        if (policy_root.isValid()) {
            ConsoleObjectTreeOperations::console_object_delete_dn_list(target_console, deleted_list, policy_root, ItemType_PolicyOU, PolicyOURole_DN);
        }
    };

    for (ConsoleWidget *console : console_list) {
        apply_changes(console);
    }

    hide_busy_indicator();

    g_status->display_ad_messages(ad, console_list[0]);
}

void ConsoleObjectTreeOperations::console_object_properties(const QList<ConsoleWidget *> &console_list, const QList<QModelIndex> &index_list, const int dn_role, const QList<QString> &class_list) {
    AdInterface ad;
    if (ad_failed(ad, console_list[0])) {
        return;
    }

    const QList<QString> dn_list = index_list_to_dn_list(index_list, dn_role);

    auto on_object_properties_applied = [console_list, dn_list]() {
        AdInterface ad2;
        if (ad_failed(ad2, console_list[0])) {
            return;
        }

        const QList<AdObject> object_list = [&]() {
            QList<AdObject> out;

            for (const QString &dn : dn_list) {
                const AdObject object = ad2.search_object(dn);

                // TODO: band-aid for the situations
                // where properties dialog interacts
                // with deleted objects. Bad stuff can
                // still happen if properties is opened
                // while object exists, then object is
                // deleted and properties is applied.
                // Remove this or improve it when you
                // tackle this issue head-on.
                if (object.is_empty()) {
                    continue;
                }

                out.append(object);
            }

            return out;
        }();

        auto apply_changes = [&object_list](ConsoleWidget *target_console) {
            auto apply_changes_to_branch = [&](const QModelIndex &root_index, const int item_type, const int update_dn_role) {
                if (!root_index.isValid()) {
                    return;
                }

                for (const AdObject &object : object_list) {
                    const QString dn = object.get_dn();
                    const QModelIndex object_index = target_console->search_item(root_index, update_dn_role, dn, {item_type});

                    if (object_index.isValid()) {
                        const QList<QStandardItem *> object_row = target_console->get_row(object_index);
                        console_object_load(object_row, object);
                    }
                }
            };

            const QModelIndex object_root = get_domain_object_tree_root(target_console);
            const QModelIndex query_root = get_query_tree_root(target_console);
            const QModelIndex policy_root = get_policy_tree_root(target_console);
            const QModelIndex find_object_root = get_find_object_root(target_console);

            apply_changes_to_branch(object_root, ItemType_Object, ObjectRole_DN);
            apply_changes_to_branch(query_root, ItemType_Object, ObjectRole_DN);
            apply_changes_to_branch(find_object_root, ItemType_Object, ObjectRole_DN);

            // Apply to policy branch
            if (policy_root.isValid()) {
                for (const AdObject &object : object_list) {
                    const QString dn = object.get_dn();
                    const QModelIndex object_index = target_console->search_item(policy_root, PolicyOURole_DN, dn, {ItemType_PolicyOU});

                    if (object_index.isValid()) {
                        const QList<QStandardItem *> object_row = target_console->get_row(object_index);

                        policy_ou_impl_load_row(object_row, object);
                    }
                }
            }
        };

        for (ConsoleWidget *console : console_list) {
            apply_changes(console);
        }

        g_status->display_ad_messages(ad2, console_list[0]);
    };

    if (dn_list.size() == 1) {
        const QString dn = dn_list[0];

        bool dialog_is_new;
        PropertiesDialog *dialog = PropertiesDialog::open_for_target(ad, dn, &dialog_is_new, console_list[0]);

        if (dialog_is_new) {
            QObject::connect(
                dialog, &PropertiesDialog::applied,
                console_list[0], on_object_properties_applied);

            QObject::connect(
                dialog, &PropertiesDialog::applied,
                [console_list]()
                    {console_list[0]->update_current_item_results_widget();});
        }
    } else if (dn_list.size() > 1) {
        auto dialog = new PropertiesMultiDialog(ad, dn_list, class_list);
        dialog->open();

        QObject::connect(
            dialog, &PropertiesMultiDialog::applied,
            console_list[0], on_object_properties_applied);
    }
}

void ConsoleObjectTreeOperations::console_tree_add_root_child(ConsoleWidget *console, AdObject &obj, int sort_idx,
                                                              const QString& custom_object_name) {
    const QList<QStandardItem *> row = console->add_scope_item(ItemType_Object, console->domain_info_index());
    console_object_item_data_load(row[0], obj);
    if (custom_object_name.isEmpty()) {
        row[0]->setText(obj.get_string(ATTRIBUTE_NAME));
    } else {
        row[0]->setText(custom_object_name);
    }
    console->set_item_sort_index(row[0]->index(), sort_idx);
}

void ConsoleObjectTreeOperations::console_tree_add_sites_container(ConsoleWidget *console, AdInterface &ad) {
    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_SITES_CONTAINER);
    auto search_results = ad.search(g_adconfig->configuration_dn(), SearchScope_All, filter, {});
    const QString err = QObject::tr("Sites container is not available");
    if (search_results.isEmpty() || search_results.values()[0].is_empty()) {
        g_status->add_message(err, StatusType_Info);
        return;
    }

    const int sites_container_sort_idx = 4;
    console_tree_add_root_child(console, search_results.values()[0], sites_container_sort_idx);
}

CreateObjectDialog *ConsoleObjectTreeOperations::create_dialog(const QString &object_class, AdInterface &ad, const QString &parent_dn, ConsoleWidget *parent) {

    if (object_class == CLASS_USER)
            return new CreateUserDialog(ad, parent_dn, CLASS_USER, parent);
    if (object_class == CLASS_GROUP)
        return new CreateGroupDialog(parent_dn, parent);
    if (object_class == CLASS_COMPUTER)
        return new CreateComputerDialog(parent_dn, parent);
    if (object_class == CLASS_OU)
        return new CreateOUDialog(parent_dn, parent);
    if (object_class == CLASS_SHARED_FOLDER)
        return new CreateSharedFolderDialog(parent_dn, parent);
    if (object_class == CLASS_INET_ORG_PERSON)
        return new CreateUserDialog(ad, parent_dn, CLASS_INET_ORG_PERSON, parent);
    if (object_class == CLASS_CONTACT)
        return new CreateContactDialog(parent_dn, parent);
    if (object_class == CLASS_PSO)
        return new CreatePSODialog(parent_dn, parent);
    if (object_class == CLASS_SITE)
        return new CreateSiteDialog(ad, parent);
    if (object_class == CLASS_SUBNET)
        return new CreateSubnetDialog(ad, parent_dn, parent);
    if (object_class == CLASS_SITE_LINK)
        return new CreateSitesLinkDialog(ad, SitesLinkType::Link, parent_dn, parent);
    if (object_class == CLASS_SITE_LINK_BRIDGE)
        return new CreateSitesLinkDialog(ad, SitesLinkType::Bridge, parent_dn, parent);

    return nullptr;
}

QModelIndex ConsoleObjectTreeOperations::get_object_tree_root(ConsoleWidget *console, const QString &root_obj_dn) {
    const QModelIndex console_root = console->domain_info_index();
    const QList<QModelIndex> search_results = console->search_items(console_root, ObjectRole_DN, root_obj_dn, {ItemType_Object});

    if (!search_results.isEmpty()) {
        for (const QModelIndex &index : search_results) {
            const QModelIndex parent = index.parent();

            // Match only with the tree root domain info parent item
            if (parent == console->domain_info_index()) {
                return index;
            }
        }

        return QModelIndex();
    } else {
        return QModelIndex();
    }
}

QModelIndex ConsoleObjectTreeOperations::get_sites_container_tree_root(ConsoleWidget *console) {
    return get_object_tree_root(console, g_adconfig->sites_container_dn());
}

QModelIndex ConsoleObjectTreeOperations::get_pso_container_tree_root(ConsoleWidget *console) {
    return get_object_tree_root(console, g_adconfig->pso_container_dn());
}
