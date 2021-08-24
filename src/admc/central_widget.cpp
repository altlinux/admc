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
#include "console_actions.h"
#include "console_types/console_object.h"
#include "console_types/console_policy.h"
#include "console_types/console_query.h"
#include "console_widget/console_widget.h"
#include "console_widget/results_view.h"
#include "create_policy_dialog.h"
#include "create_query_folder_dialog.h"
#include "create_query_item_dialog.h"
#include "edit_query_folder_dialog.h"
#include "edit_query_item_dialog.h"
#include "filter_dialog.h"
#include "globals.h"
#include "gplink.h"
#include "policy_results_widget.h"
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
#include <QProcess>

CentralWidget::CentralWidget(AdInterface &ad)
: QWidget() {
    console_actions = new ConsoleActions(this);

    open_filter_action = new QAction(tr("&Filter objects"), this);

    // NOTE: these actions are not connected here because
    // they need to be connected to a custom slot
    dev_mode_action = settings_make_action(SETTING_dev_mode, tr("Dev mode"), this);
    show_noncontainers_action = settings_make_action(SETTING_show_non_containers_in_console_tree, tr("&Show non-container objects in Console tree"), this);
    advanced_features_action = settings_make_action(SETTING_advanced_features, tr("Advanced features"), this);

    toggle_console_tree_action = settings_make_action(SETTING_advanced_features, tr("Console Tree"), this);
    toggle_description_bar_action = settings_make_action(SETTING_show_results_header, tr("Description Bar"), this);

    console = new ConsoleWidget();

    filter_dialog = new FilterDialog(this);
    auto create_query_folder_dialog = new CreateQueryFolderDialog(console);
    auto edit_query_folder_dialog = new EditQueryFolderDialog(console);
    auto create_policy_dialog = new CreatePolicyDialog(console);
    auto rename_policy_dialog = new RenamePolicyDialog(console);

    auto object_results = new ResultsView(this);
    console_object_results_id = console->register_results(object_results, console_object_header_labels(), console_object_default_columns());

    auto policy_container_results = new ResultsView(this);
    policy_container_results->detail_view()->header()->setDefaultSectionSize(200);
    policy_container_results_id = console->register_results(policy_container_results, console_policy_header_labels(), console_policy_default_columns());

    policy_results_widget = new PolicyResultsWidget();
    policy_results_id = console->register_results(policy_results_widget);

    auto query_results = new ResultsView(this);
    query_results->detail_view()->header()->setDefaultSectionSize(200);
    console_query_folder_results_id = console->register_results(query_results, console_query_folder_header_labels(), console_query_folder_default_columns());

    // NOTE: requires all results to be initialized
    console_object_tree_init(console, ad);
    console_policy_tree_init(console, ad);
    console_query_tree_init(console);

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);
    layout->addWidget(console);

    const QVariant console_widget_state = settings_get_variant(SETTING_console_widget_state);
    console->restore_state(console_widget_state);

    connect_object_actions(console, console_actions);

    connect(
        show_noncontainers_action, &QAction::toggled,
        this, &CentralWidget::on_show_non_containers);
    on_show_non_containers();
    
    connect(
        dev_mode_action, &QAction::toggled,
        this, &CentralWidget::on_dev_mode);
    on_dev_mode();

    connect(
        advanced_features_action, &QAction::toggled,
        this, &CentralWidget::on_advanced_features);
    on_dev_mode();

    connect(
        toggle_console_tree_action, &QAction::toggled,
        this, &CentralWidget::on_toggle_console_tree);
    on_toggle_console_tree();

    connect(
        toggle_description_bar_action, &QAction::toggled,
        this, &CentralWidget::on_toggle_description_bar);
    on_toggle_description_bar();

    connect(
        open_filter_action, &QAction::triggered,
        filter_dialog, &QDialog::open);
    connect(
        filter_dialog, &QDialog::accepted,
        this, &CentralWidget::refresh_head);

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
        console_actions->get(ConsoleAction_PolicyEdit), &QAction::triggered,
        this, &CentralWidget::policy_edit);

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
        console, &ConsoleWidget::actions_changed,
        this, &CentralWidget::on_actions_changed);

    // Set current scope to object head to load it
    console->set_current_scope(console_object_head()->index());
}

CentralWidget::~CentralWidget() {
    const QVariant state = console->save_state();
    settings_set_variant(SETTING_console_widget_state, state);
}

void CentralWidget::on_actions_changed() {
    const QList<QModelIndex> selected_list = console->get_selected_items();

    console_actions->update_actions_visibility(selected_list);
}

void CentralWidget::policy_add_link() {
    const QList<QModelIndex> selected = console->get_selected_items();
    if (selected.size() == 0) {
        return;
    }

    auto dialog = new SelectObjectDialog({CLASS_OU}, SelectObjectDialogMultiSelection_Yes, this);
    dialog->setWindowTitle(tr("Add Link"));

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
        const bool success = ad.gpo_delete(dn);

        // NOTE: object may get deleted successfuly but
        // deleting GPT fails which makes gpo_delete() fail
        // as a whole, but we still want to remove gpo from
        // the console in that case
        const AdObject gpo_object = ad.search_object(dn);
        const bool object_deleted = gpo_object.is_empty();

        if (success || object_deleted) {
            console->delete_item(index);
        }
    }

    hide_busy_indicator();

    g_status()->display_ad_messages(ad, this);
}

void CentralWidget::policy_edit() {
    const QString dn = get_selected_dn();

    const QString filesys_path = [&]() {
        AdInterface ad;
        if (ad_failed(ad)) {
            return QString();
        }

        const AdObject object = ad.search_object(dn);
        const QString out = object.get_string(ATTRIBUTE_GPC_FILE_SYS_PATH);

        return out;
    }();

    auto process = new QProcess(this);
    process->setProgram("gpui");

    const QList<QString> args = {
        dn,
        filesys_path,
    };
    process->setArguments(args);

    connect(
        process, &QProcess::errorOccurred,
        [this](QProcess::ProcessError error) {
            const bool failed_to_start = (error == QProcess::FailedToStart);

            if (failed_to_start) {
                const QString error_text = "Failed to start gpui. Check that it's installed.";
                qDebug() << error_text;
                g_status()->add_message(error_text, StatusType_Error);
                error_log({error_text}, this);
            }
        });

    process->start(QIODevice::ReadOnly);
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

void CentralWidget::on_show_non_containers() {
    settings_set_bool(SETTING_show_non_containers_in_console_tree, show_noncontainers_action->isChecked());

    refresh_head();
}

void CentralWidget::on_dev_mode() {
    settings_set_bool(SETTING_dev_mode, dev_mode_action->isChecked());

    refresh_head();
}

void CentralWidget::on_advanced_features() {
    settings_set_bool(SETTING_advanced_features, advanced_features_action->isChecked());

    refresh_head();
}

void CentralWidget::on_toggle_console_tree() {
    const bool visible = toggle_console_tree_action->isChecked();
    
    settings_set_bool(SETTING_advanced_features, visible);
    console->get_scope_view()->setVisible(visible);
}

void CentralWidget::on_toggle_description_bar() {
    const bool visible = toggle_description_bar_action->isChecked();
    
    settings_set_bool(SETTING_advanced_features, visible);
    console->get_description_bar()->setVisible(visible);
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

        const QString object_count_text = [&]() {
            const int results_count = console->get_current_results_count();
            const QString out = tr("%n object(s)", "", results_count);

            return out;
        }();

        if (type == ItemType_Object) {
            QString out = object_count_text;

            const bool filtering_ON = filter_dialog->filtering_ON();
            if (filtering_ON) {
                out += tr(" [Filtering enabled]");
            }

            return out;
        } else if (type == ItemType_QueryItem) {
            return object_count_text;
        } else {
            return QString();
        }
    }();

    console->set_description_bar_text(text);
}

void CentralWidget::add_actions(QMenu *action_menu, QMenu *navigation_menu, QMenu *view_menu, QMenu *preferences_menu, QToolBar *toolbar) {
    console_actions->add_to_menu(action_menu);

    action_menu->addSeparator();

    console->add_actions(action_menu, navigation_menu, view_menu, toolbar);

    view_menu->addSeparator();

    view_menu->addAction(open_filter_action);

    view_menu->addAction(show_noncontainers_action);

#ifdef QT_DEBUG
    view_menu->addAction(dev_mode_action);
#endif

    preferences_menu->addAction(advanced_features_action);
    preferences_menu->addAction(toggle_console_tree_action);
    preferences_menu->addAction(toggle_description_bar_action);
}

void CentralWidget::fetch_scope_node(const QModelIndex &index) {
    const ItemType type = (ItemType) index.data(ConsoleRole_Type).toInt();

    if (type == ItemType_Object) {
        console_object_fetch(console, filter_dialog->get_filter(), index);
    } else if (type == ItemType_QueryItem) {
        console_query_item_fetch(console, index);
    } else if (type == ItemType_PolicyRoot) {
        console_policy_root_fetch(console);
    }
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
