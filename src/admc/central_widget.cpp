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

#include "central_widget.h"

#include "adldap.h"
#include "change_dc_dialog.h"
#include "console_actions.h"
#include "console_types/console_object.h"
#include "console_types/console_policy.h"
#include "console_types/console_query.h"
#include "console_widget/console_widget.h"
#include "console_widget/results_view.h"
#include "create_object_dialog.h"
#include "create_policy_dialog.h"
#include "create_query_folder_dialog.h"
#include "create_query_item_dialog.h"
#include "edit_query_folder_dialog.h"
#include "edit_query_item_dialog.h"
#include "editors/multi_editor.h"
#include "filter_dialog.h"
#include "find_object_dialog.h"
#include "globals.h"
#include "gplink.h"
#include "move_object_dialog.h"
#include "object_multi_properties_dialog.h"
#include "password_dialog.h"
#include "policy_results_widget.h"
#include "properties_dialog.h"
#include "rename_object_dialog.h"
#include "rename_policy_dialog.h"
#include "select_object_dialog.h"
#include "settings.h"
#include "status.h"
#include "utils.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QDebug>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QStack>
#include <QStandardItemModel>
#include <QTreeWidget>
#include <QVBoxLayout>

CentralWidget::CentralWidget()
: QWidget() {
    console_actions = new ConsoleActions(this);

    open_filter_action = new QAction(tr("&Filter objects"), this);
    dev_mode_action = new QAction(tr("Dev mode"), this);
    show_noncontainers_action = new QAction(tr("&Show non-container objects in Console tree"), this);

    filter_dialog = nullptr;
    open_filter_action->setEnabled(false);

    console = new ConsoleWidget();

    auto create_query_folder_dialog = new CreateQueryFolderDialog(console);
    auto edit_query_folder_dialog = new EditQueryFolderDialog(console);
    auto create_policy_dialog = new CreatePolicyDialog(console);
    auto rename_policy_dialog = new RenamePolicyDialog(console);

    auto policy_container_results = new ResultsView(this);
    policy_container_results->detail_view()->header()->setDefaultSectionSize(200);
    policy_container_results_id = console->register_results(policy_container_results, console_policy_header_labels(), console_policy_default_columns());

    policy_results_widget = new PolicyResultsWidget();
    policy_results_id = console->register_results(policy_results_widget);

    auto query_results = new ResultsView(this);
    query_results->detail_view()->header()->setDefaultSectionSize(200);
    console_query_folder_results_id = console->register_results(query_results, console_query_folder_header_labels(), console_query_folder_default_columns());

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);
    layout->addWidget(console);

    // Refresh head when settings affecting the filter
    // change. This reloads the model with an updated filter
    const BoolSettingSignal *advanced_features = g_settings->get_bool_signal(BoolSetting_AdvancedFeatures);
    connect(
        advanced_features, &BoolSettingSignal::changed,
        this, &CentralWidget::refresh_head);

    const BoolSettingSignal *show_non_containers = g_settings->get_bool_signal(BoolSetting_ShowNonContainersInConsoleTree);
    connect(
        show_non_containers, &BoolSettingSignal::changed,
        this, &CentralWidget::refresh_head);

    const BoolSettingSignal *dev_mode_signal = g_settings->get_bool_signal(BoolSetting_DevMode);
    connect(
        dev_mode_signal, &BoolSettingSignal::changed,
        this, &CentralWidget::refresh_head);

    g_settings->connect_toggle_widget(console->get_scope_view(), BoolSetting_ShowConsoleTree);
    g_settings->connect_toggle_widget(console->get_description_bar(), BoolSetting_ShowResultsHeader);

    g_settings->connect_action_to_bool_setting(dev_mode_action, BoolSetting_DevMode);
    g_settings->connect_action_to_bool_setting(show_noncontainers_action, BoolSetting_ShowNonContainersInConsoleTree);

    connect(
        open_filter_action, &QAction::triggered,
        this, &CentralWidget::open_filter);

    connect(
        console_actions->get(ConsoleAction_NewUser), &QAction::triggered,
        this, &CentralWidget::object_create_user);
    connect(
        console_actions->get(ConsoleAction_NewComputer), &QAction::triggered,
        this, &CentralWidget::object_create_computer);
    connect(
        console_actions->get(ConsoleAction_NewOU), &QAction::triggered,
        this, &CentralWidget::object_create_ou);
    connect(
        console_actions->get(ConsoleAction_NewGroup), &QAction::triggered,
        this, &CentralWidget::object_create_group);
    connect(
        console_actions->get(ConsoleAction_Delete), &QAction::triggered,
        this, &CentralWidget::object_delete);
    connect(
        console_actions->get(ConsoleAction_Rename), &QAction::triggered,
        this, &CentralWidget::object_rename);
    connect(
        console_actions->get(ConsoleAction_Move), &QAction::triggered,
        this, &CentralWidget::object_move);
    connect(
        console_actions->get(ConsoleAction_AddToGroup), &QAction::triggered,
        this, &CentralWidget::object_add_to_group);
    connect(
        console_actions->get(ConsoleAction_Enable), &QAction::triggered,
        this, &CentralWidget::object_enable);
    connect(
        console_actions->get(ConsoleAction_Disable), &QAction::triggered,
        this, &CentralWidget::object_disable);
    connect(
        console_actions->get(ConsoleAction_ResetPassword), &QAction::triggered,
        this, &CentralWidget::object_reset_password);
    connect(
        console_actions->get(ConsoleAction_Find), &QAction::triggered,
        this, &CentralWidget::object_find);
    connect(
        console_actions->get(ConsoleAction_EditUpnSuffixes), &QAction::triggered,
        this, &CentralWidget::object_edit_upn_suffixes);
    connect(
        console_actions->get(ConsoleAction_ChangeDC), &QAction::triggered,
        this, &CentralWidget::object_change_dc);

    connect(
        console_actions->get(ConsoleAction_PolicyCreate), &QAction::triggered,
        create_policy_dialog, &QDialog::open);
    connect(
        console_actions->get(ConsoleAction_PolicyAddLink), &QAction::triggered,
        this, &CentralWidget::policy_add_link);
    connect(
        console_actions->get(ConsoleAction_PolicyRename), &QAction::triggered,
        rename_policy_dialog, &QDialog::open);
    connect(
        console_actions->get(ConsoleAction_PolicyDelete), &QAction::triggered,
        this, &CentralWidget::policy_delete);

    connect(
        console_actions->get(ConsoleAction_QueryCreateFolder), &QAction::triggered,
        create_query_folder_dialog, &CreateQueryFolderDialog::open);
    connect(
        console_actions->get(ConsoleAction_QueryCreateItem), &QAction::triggered,
        this, &CentralWidget::query_create);
    connect(
        console_actions->get(ConsoleAction_QueryEditFolder), &QAction::triggered,
        edit_query_folder_dialog, &QDialog::open);
    connect(
        console_actions->get(ConsoleAction_QueryEditItem), &QAction::triggered,
        this, &CentralWidget::query_edit);
    connect(
        console_actions->get(ConsoleAction_QueryCutItemOrFolder), &QAction::triggered,
        this, &CentralWidget::query_cut);
    connect(
        console_actions->get(ConsoleAction_QueryCopyItemOrFolder), &QAction::triggered,
        this, &CentralWidget::query_copy);
    connect(
        console_actions->get(ConsoleAction_QueryPasteItemOrFolder), &QAction::triggered,
        this, &CentralWidget::query_paste);
    connect(
        console_actions->get(ConsoleAction_QueryDeleteItemOrFolder), &QAction::triggered,
        this, &CentralWidget::query_delete);
    connect(
        console_actions->get(ConsoleAction_QueryExport), &QAction::triggered,
        this, &CentralWidget::query_export);
    connect(
        console_actions->get(ConsoleAction_QueryImport), &QAction::triggered,
        this, &CentralWidget::query_import);

    connect(
        console, &ConsoleWidget::current_scope_item_changed,
        this, &CentralWidget::on_current_scope_changed);
    connect(
        console, &ConsoleWidget::results_count_changed,
        this, &CentralWidget::update_description_bar);
    connect(
        console, &ConsoleWidget::item_fetched,
        this, &CentralWidget::fetch_scope_node);
    connect(
        console, &ConsoleWidget::items_can_drop,
        this, &CentralWidget::on_items_can_drop);
    connect(
        console, &ConsoleWidget::items_dropped,
        this, &CentralWidget::on_items_dropped);
    connect(
        console, &ConsoleWidget::properties_requested,
        this, &CentralWidget::object_properties);
    connect(
        console, &ConsoleWidget::actions_changed,
        this, &CentralWidget::update_actions_visibility);
    connect(
        console, &ConsoleWidget::context_menu,
        this, &CentralWidget::context_menu);

    update_actions_visibility();
}

void CentralWidget::go_online(AdInterface &ad) {
    // NOTE: filter dialog requires a connection to load
    // display strings from adconfig so create it here
    filter_dialog = new FilterDialog(this);
    connect(
        filter_dialog, &QDialog::accepted,
        this, &CentralWidget::refresh_head);
    open_filter_action->setEnabled(true);

    // NOTE: need to create object results here because it
    // requires header labels which come from ADCONFIG, so
    // need to be online
    auto object_results = new ResultsView(this);
    console_object_results_id = console->register_results(object_results, console_object_header_labels(), console_object_default_columns());

    console_object_tree_init(console, ad);
    console_policy_tree_init(console, ad);
    console_query_tree_init(console);

    console->set_current_scope(console_object_head()->index());
}

void CentralWidget::open_filter() {
    if (filter_dialog != nullptr) {
        filter_dialog->open();
    }
}

void CentralWidget::object_delete() {
    const QHash<QString, QPersistentModelIndex> selected = get_selected_dns_and_indexes();
    const QList<QString> deleted_objects = object_operation_delete(selected.keys(), this);

    console_object_delete(console, deleted_objects);
}

void CentralWidget::object_properties() {
    const QHash<QString, QPersistentModelIndex> targets = get_selected_dns_and_indexes();

    if (targets.size() == 1) {
        const QString target = targets.keys()[0];

        PropertiesDialog *dialog = PropertiesDialog::open_for_target(target);

        connect(
            dialog, &PropertiesDialog::applied,
            this, &CentralWidget::on_object_properties_applied);
    } else if (targets.size() > 1) {
        const QList<QString> class_list = [&]() {
            QSet<QString> out;

            for (const QPersistentModelIndex &index : targets.values()) {
                const QList<QString> this_class_list = index.data(ObjectRole_ObjectClasses).toStringList();
                const QString main_class = this_class_list.last();
                out.insert(main_class);
            }

            return out.toList();
        }();

        auto dialog = new ObjectMultiPropertiesDialog(targets.keys(), class_list);
        dialog->open();

        connect(
            dialog, &ObjectMultiPropertiesDialog::applied,
            this, &CentralWidget::on_object_properties_applied);
    }
}

void CentralWidget::object_rename() {
    const QString target = get_selected_dn();

    auto dialog = new RenameObjectDialog(target, this);
    dialog->open();

    connect(
        dialog, &RenameObjectDialog::accepted,
        [=]() {
            AdInterface ad;
            if (ad_failed(ad)) {
                return;
            }

            const QString old_dn = target;
            const QString new_dn = dialog->get_new_dn();
            const QString parent_dn = dn_get_parent(old_dn);
            console_object_move(console, ad, {old_dn}, {new_dn}, parent_dn);
        });
}

void CentralWidget::object_create_helper(const QString &object_class) {
    const QString parent_dn = get_selected_dn();

    auto dialog = new CreateObjectDialog(parent_dn, object_class, this);
    dialog->open();

    // NOTE: can't just add new object to console by adding
    // to selected index, because you can create an object
    // by using action menu of an object in a query tree.
    // Therefore need to search for parent in domain tree.
    connect(
        dialog, &CreateObjectDialog::accepted,
        [=]() {
            AdInterface ad;
            if (ad_failed(ad)) {
                return;
            }

            show_busy_indicator();

            const QList<QModelIndex> search_parent = console->search_items(console_object_head()->index(), ObjectRole_DN, parent_dn, ItemType_Object);

            if (search_parent.isEmpty()) {
                hide_busy_indicator();
                return;
            }

            const QModelIndex scope_parent_index = search_parent[0];
            const QString created_dn = dialog->get_created_dn();
            console_object_create(console, ad, {created_dn}, scope_parent_index);

            hide_busy_indicator();
        });
}

void CentralWidget::object_move() {
    const QHash<QString, QPersistentModelIndex> targets = get_selected_dns_and_indexes();

    auto dialog = new MoveObjectDialog(targets.keys(), this);
    dialog->open();

    connect(
        dialog, &QDialog::accepted,
        [=]() {
            AdInterface ad;
            if (ad_failed(ad)) {
                return;
            }

            const QList<QString> old_dn_list = dialog->get_moved_objects();
            const QString new_parent_dn = dialog->get_selected();
            console_object_move(console, ad, old_dn_list, new_parent_dn);
        });
}

void CentralWidget::object_add_to_group() {
    const QList<QString> targets = get_selected_dns();
    object_operation_add_to_group(targets, this);
}

void CentralWidget::object_enable() {
    enable_disable_helper(false);
}

void CentralWidget::object_disable() {
    enable_disable_helper(true);
}

void CentralWidget::object_find() {
    const QList<QString> targets = get_selected_dns();

    if (targets.size() != 1) {
        return;
    }

    const QString target = targets[0];

    auto find_dialog = new FindObjectDialog(filter_classes, target, this);
    find_dialog->open();
}

void CentralWidget::object_reset_password() {
    const QString target = get_selected_dn();
    const auto password_dialog = new PasswordDialog(target, this);
    password_dialog->open();
}

void CentralWidget::object_create_user() {
    object_create_helper(CLASS_USER);
}

void CentralWidget::object_create_computer() {
    object_create_helper(CLASS_COMPUTER);
}

void CentralWidget::object_create_ou() {
    object_create_helper(CLASS_OU);
}

void CentralWidget::object_create_group() {
    object_create_helper(CLASS_GROUP);
}

void CentralWidget::object_edit_upn_suffixes() {
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    // Open editor for upn suffixes attribute of partitions
    // object
    const QString partitions_dn = g_adconfig->partitions_dn();
    const AdObject partitions_object = ad.search_object(partitions_dn);
    const QList<QByteArray> current_values = partitions_object.get_values(ATTRIBUTE_UPN_SUFFIXES);

    g_status()->display_ad_messages(ad, this);

    auto editor = new MultiEditor(ATTRIBUTE_UPN_SUFFIXES, current_values, this);
    editor->open();

    // When editor is accepted, update values of upn
    // suffixes
    connect(editor, &QDialog::accepted,
        [this, editor, partitions_dn]() {
            AdInterface ad2;
            if (ad_failed(ad2)) {
                return;
            }

            const QList<QByteArray> new_values = editor->get_new_values();

            ad2.attribute_replace_values(partitions_dn, ATTRIBUTE_UPN_SUFFIXES, new_values);
            g_status()->display_ad_messages(ad2, this);
        });
}

void CentralWidget::object_change_dc() {
    auto change_dc_dialog = new ChangeDCDialog(console_object_head(), this);
    change_dc_dialog->open();
}

void CentralWidget::policy_add_link() {
    const QList<QModelIndex> selected = console->get_selected_items();
    if (selected.size() == 0) {
        return;
    }

    auto dialog = new SelectObjectDialog({CLASS_OU}, SelectObjectDialogMultiSelection_Yes, this);

    QObject::connect(
        dialog, &SelectObjectDialog::accepted,
        [=]() {
            const QList<QString> gpos = [selected]() {
                QList<QString> out;

                for (const QModelIndex &index : selected) {
                    const QString gpo = index.data(PolicyRole_DN).toString();
                    out.append(gpo);
                }

                return out;
            }();

            const QList<QString> ou_list = dialog->get_selected();

            console_policy_add_link(console, gpos, ou_list, policy_results_widget);

            const QModelIndex current_scope = console->get_current_scope_item();
            policy_results_widget->update(current_scope);
        });

    dialog->open();
}

void CentralWidget::policy_delete() {
    const QHash<QString, QPersistentModelIndex> selected = get_selected_dns_and_indexes();

    if (selected.size() == 0) {
        return;
    }

    const bool confirmed = confirmation_dialog(tr("Are you sure you want to delete this policy and all of it's links?"), this);
    if (!confirmed) {
        return;
    }

    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    show_busy_indicator();

    for (const QPersistentModelIndex &index : selected) {
        const QString dn = index.data(PolicyRole_DN).toString();
        const bool success = ad.object_delete(dn);

        if (success) {
            // Remove deleted policy from console
            console->delete_item(index);

            // Remove links to delete policy
            const QString base = g_adconfig->domain_head();
            const SearchScope scope = SearchScope_All;
            const QString filter = filter_CONDITION(Condition_Contains, ATTRIBUTE_GPLINK, dn);
            const QList<QString> attributes = {ATTRIBUTE_GPLINK};
            const QHash<QString, AdObject> results = ad.search(base, scope, filter, attributes);

            for (const AdObject &object : results.values()) {
                const QString gplink_string = object.get_string(ATTRIBUTE_GPLINK);
                Gplink gplink = Gplink(gplink_string);
                gplink.remove(dn);

                const QString updated_gplink_string = gplink.to_string();
                ad.attribute_replace_string(object.get_dn(), ATTRIBUTE_GPLINK, updated_gplink_string);
            }
        }
    }

    hide_busy_indicator();

    g_status()->display_ad_messages(ad, this);
}

void CentralWidget::query_create() {
    auto dialog = new CreateQueryItemDialog(console);
    dialog->open();
}

void CentralWidget::query_edit() {
    auto dialog = new EditQueryItemDialog(console);
    dialog->open();
}

void CentralWidget::query_delete() {
    const QList<QPersistentModelIndex> selected_indexes = persistent_index_list(console->get_selected_items());

    for (const QPersistentModelIndex &index : selected_indexes) {
        console->delete_item(index);
    }

    console_query_tree_save(console);
}

void CentralWidget::query_export() {
    console_query_export(console);
}

void CentralWidget::query_import() {
    console_query_import(console);
}

void CentralWidget::query_cut() {
    console_query_cut(console);
}

void CentralWidget::query_copy() {
    console_query_copy(console);
}

void CentralWidget::query_paste() {
    console_query_paste(console);
}

void CentralWidget::on_items_can_drop(const QList<QPersistentModelIndex> &dropped_list, const QPersistentModelIndex &target, bool *ok) {
    const bool dropped_contains_target = [&]() {
        for (const QPersistentModelIndex &dropped : dropped_list) {
            if (dropped == target) {
                return true;
            }
        }

        return false;
    }();

    if (dropped_contains_target) {
        return;
    }

    const ItemType target_type = (ItemType) target.data(ConsoleRole_Type).toInt();
    const QSet<ItemType> dropped_types = [&]() {
        QSet<ItemType> out;

        for (const QPersistentModelIndex &index : dropped_list) {
            const ItemType type = (ItemType) index.data(ConsoleRole_Type).toInt();
            out.insert(type);
        }

        return out;
    }();

    switch (target_type) {
        case ItemType_Unassigned: break;
        case ItemType_Object: {
            console_object_can_drop(dropped_list, target, dropped_types, ok);
            break;
        }
        case ItemType_PolicyRoot: break;
        case ItemType_Policy: {
            console_policy_can_drop(dropped_list, target, dropped_types, ok);
            break;
        }
        case ItemType_QueryRoot: {
            console_query_can_drop(dropped_list, target, dropped_types, ok);
            break;
        }
        case ItemType_QueryFolder: {
            console_query_can_drop(dropped_list, target, dropped_types, ok);
            break;
        }
        case ItemType_QueryItem: break;
        case ItemType_LAST: break;
    }
}

void CentralWidget::on_items_dropped(const QList<QPersistentModelIndex> &dropped_list, const QPersistentModelIndex &target) {
    const ItemType target_type = (ItemType) target.data(ConsoleRole_Type).toInt();
    const QSet<ItemType> dropped_types = [&]() {
        QSet<ItemType> out;

        for (const QPersistentModelIndex &index : dropped_list) {
            const ItemType type = (ItemType) index.data(ConsoleRole_Type).toInt();
            out.insert(type);
        }

        return out;
    }();

    switch (target_type) {
        case ItemType_Unassigned: break;
        case ItemType_Object: {
            console_object_drop(console, dropped_list, dropped_types, target, policy_results_widget);
            break;
        }
        case ItemType_PolicyRoot: break;
        case ItemType_Policy: {
            console_policy_drop(console, dropped_list, target, policy_results_widget);
            break;
        }
        case ItemType_QueryRoot: {
            console_query_drop(console, dropped_list, target);
            break;
        }
        case ItemType_QueryFolder: {
            console_query_drop(console, dropped_list, target);
            break;
        }
        case ItemType_QueryItem: break;
        case ItemType_LAST: break;
    }
}

void CentralWidget::on_current_scope_changed() {
    const QModelIndex current_scope = console->get_current_scope_item();
    policy_results_widget->update(current_scope);

    update_description_bar();
}

void CentralWidget::on_object_properties_applied() {
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    const QList<QString> target_list = get_selected_dns();

    for (const QString &target : target_list) {
        const AdObject object = ad.search_object(target);

        const QList<QModelIndex> index_list = console->search_items(console_object_head()->index(), ObjectRole_DN, target, ItemType_Object);
        for (const QModelIndex &index : index_list) {
            const QList<QStandardItem *> row = console->get_row(index);
            console_object_load(row, object);
        }
    }

    g_status()->display_ad_messages(ad, this);

    update_actions_visibility();
}

void CentralWidget::refresh_head() {
    show_busy_indicator();

    console->refresh_scope(console_object_head()->index());

    hide_busy_indicator();
}

void CentralWidget::update_description_bar() {
    const QString text = [this]() {
        const QModelIndex current_scope = console->get_current_scope_item();
        const ItemType type = (ItemType) current_scope.data(ConsoleRole_Type).toInt();

        if (type == ItemType_Object) {
            const int results_count = console->get_current_results_count();
            QString out = tr("%n object(s)", "", results_count);

            const bool filtering_ON = filter_dialog->filtering_ON();
            if (filtering_ON) {
                out += tr(" [Filtering results]");
            }

            return out;
        } else {
            return QString();
        }
    }();

    console->set_description_bar_text(text);
}

void CentralWidget::add_actions_to_action_menu(QMenu *menu) {
    console_actions->add_to_menu(menu);

    menu->addSeparator();

    console->add_actions_to_action_menu(menu);
}

void CentralWidget::add_actions_to_navigation_menu(QMenu *menu) {
    console->add_actions_to_navigation_menu(menu);
}

void CentralWidget::add_actions_to_view_menu(QMenu *menu) {
    console->add_actions_to_view_menu(menu);

    menu->addSeparator();

    menu->addAction(open_filter_action);

    menu->addAction(show_noncontainers_action);

#ifdef QT_DEBUG
    menu->addAction(dev_mode_action);
#endif
}

void CentralWidget::fetch_scope_node(const QModelIndex &index) {
    const ItemType type = (ItemType) index.data(ConsoleRole_Type).toInt();

    if (type == ItemType_Object) {
        console_object_fetch(console, filter_dialog, index);
    } else if (type == ItemType_QueryItem) {
        console_query_item_fetch(console, index);
    }
}

void CentralWidget::enable_disable_helper(const bool disabled) {
    const QHash<QString, QPersistentModelIndex> targets = get_selected_dns_and_indexes();

    show_busy_indicator();

    const QList<QString> changed_objects = object_operation_set_disabled(targets.keys(), disabled, this);

    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    for (const QString &dn : changed_objects) {
        const QList<QModelIndex> index_list = console->search_items(console_object_head()->index(), ObjectRole_DN, dn, ItemType_Object);
        for (const QModelIndex &index : index_list) {
            QStandardItem *item = console->get_item(index);
            item->setData(disabled, ObjectRole_AccountDisabled);
        }
    }

    update_actions_visibility();

    hide_busy_indicator();
}

// First, hide all actions, then show whichever actions are
// appropriate for current console selection
void CentralWidget::update_actions_visibility() {
    // Figure out what kind of types of items are selected
    const QList<QModelIndex> selected_indexes = console->get_selected_items();

    console_actions->update_actions_visibility(selected_indexes);
}

// Get selected indexes mapped to their DN's
QHash<QString, QPersistentModelIndex> CentralWidget::get_selected_dns_and_indexes() {
    QHash<QString, QPersistentModelIndex> out;

    const QList<QModelIndex> indexes = console->get_selected_items();
    for (const QModelIndex &index : indexes) {
        const QString dn = index.data(ObjectRole_DN).toString();
        out[dn] = QPersistentModelIndex(index);
    }

    return out;
}

QList<QString> CentralWidget::get_selected_dns() {
    const QHash<QString, QPersistentModelIndex> selected = get_selected_dns_and_indexes();

    return selected.keys();
}

QString CentralWidget::get_selected_dn() {
    const QList<QString> dn_list = get_selected_dns();

    if (!dn_list.isEmpty()) {
        return dn_list[0];
    } else {
        return QString();
    }
}
