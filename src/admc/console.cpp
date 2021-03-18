/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
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

#include "console.h"

#include "object_model.h"
#include "object_menu.h"
#include "utils.h"
#include "ad_utils.h"
#include "properties_dialog.h"
#include "ad_config.h"
#include "ad_interface.h"
#include "ad_object.h"
#include "filter.h"
#include "settings.h"
#include "filter_dialog.h"
#include "filter_widget/filter_widget.h"
#include "object_menu.h"
#include "status.h"
#include "object_drag.h"
#include "console_drag_model.h"
#include "console_widget.h"
#include "rename_dialog.h"
#include "select_container_dialog.h"
#include "create_dialog.h"
#include "results_view.h"

#include <QDebug>
#include <QAbstractItemView>
#include <QVBoxLayout>
#include <QSplitter>
#include <QDebug>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QApplication>
#include <QTreeWidget>
#include <QStack>
#include <QMenu>
#include <QLabel>
#include <QSortFilterProxyModel>

bool object_should_be_in_scope(const AdObject &object);

Console::Console()
: QWidget()
{
    rename_action = new QAction(tr("&Rename"));
    move_action = new QAction(tr("&Move"));
    open_filter_action = new QAction(tr("&Filter objects"));
    advanced_view_action = new QAction(tr("&Advanced View"));
    advanced_view_action = new QAction(tr("&Advanced View"));
    dev_mode_action = new QAction(tr("Dev mode"));
    show_noncontainers_action = new QAction(tr("&Show non-container objects in Console tree"));

    filter_dialog = nullptr;

    console_widget = new ConsoleWidget();

    object_results = new ResultsView(this);
    // TODO: not sure how to do this. View headers dont even
    // have sections until their models are loaded.
    // SETTINGS()->setup_header_state(object_results->header(),
    // VariantSetting_ResultsHeader);
    
    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);
    layout->addWidget(console_widget);

    // Refresh head when settings affecting the filter
    // change. This reloads the model with an updated filter
    const BoolSettingSignal *advanced_view = SETTINGS()->get_bool_signal(BoolSetting_AdvancedView);
    connect(
        advanced_view, &BoolSettingSignal::changed,
        this, &Console::refresh_head);

    const BoolSettingSignal *show_non_containers = SETTINGS()->get_bool_signal(BoolSetting_ShowNonContainersInConsoleTree);
    connect(
        show_non_containers, &BoolSettingSignal::changed,
        this, &Console::refresh_head);

    const BoolSettingSignal *dev_mode_signal = SETTINGS()->get_bool_signal(BoolSetting_DevMode);
    connect(
        dev_mode_signal, &BoolSettingSignal::changed,
        this, &Console::refresh_head);

    SETTINGS()->connect_toggle_widget(console_widget->get_scope_view(), BoolSetting_ShowConsoleTree);
    SETTINGS()->connect_toggle_widget(console_widget->get_description_bar(), BoolSetting_ShowResultsHeader);

    SETTINGS()->connect_action_to_bool_setting(advanced_view_action, BoolSetting_AdvancedView);
    SETTINGS()->connect_action_to_bool_setting(dev_mode_action, BoolSetting_DevMode);
    SETTINGS()->connect_action_to_bool_setting(show_noncontainers_action, BoolSetting_ShowNonContainersInConsoleTree);

    connect(
        open_filter_action, &QAction::triggered,
        this, &Console::open_filter);
    connect(
        rename_action, &QAction::triggered,
        this, &Console::rename);
    connect(
        move_action, &QAction::triggered,
        this, &Console::move);
    connect(
        console_widget, &ConsoleWidget::current_scope_item_changed,
        this, &Console::update_description_bar);
    connect(
        console_widget, &ConsoleWidget::action_menu_about_to_open,
        this, &Console::on_action_menu_about_to_open);
    connect(
        console_widget, &ConsoleWidget::view_menu_about_to_open,
        this, &Console::on_view_menu_about_to_open);
    connect(
        console_widget, &ConsoleWidget::item_fetched,
        this, &Console::fetch_scope_node);
    connect(
        console_widget, &ConsoleWidget::items_can_drop,
        this, &Console::on_items_can_drop);
    connect(
        console_widget, &ConsoleWidget::items_dropped,
        this, &Console::on_items_dropped);
    connect(
        console_widget, &ConsoleWidget::properties_requested,
        this, &Console::on_properties_requested);
}

void Console::go_online(AdInterface &ad) {
    // NOTE: filter dialog requires a connection to load
    // display strings from adconfig so create it here
    filter_dialog = new FilterDialog(this);
    connect(
        filter_dialog, &QDialog::accepted,
        this, &Console::refresh_head);

    // NOTE: Header labels are from ADCONFIG, so have to get them
    // after going online
    object_results_id = console_widget->register_results(object_results, object_model_header_labels(), object_model_default_columns());

    const QString head_dn = ADCONFIG()->domain_head();
    const AdObject head_object = ad.search_object(head_dn);

    QStandardItem *item = console_widget->add_scope_item(object_results_id, true, QModelIndex());

    scope_head_index = QPersistentModelIndex(item->index());

    setup_scope_item(item, head_object);

    // Need to set current since by default views have none
    console_widget->set_current_scope(scope_head_index);
}

void Console::open_filter() {
    if (filter_dialog != nullptr) {
        filter_dialog->open();
    }
}

void Console::delete_objects(const QList<QModelIndex> &indexes) {
    const QString text = QString(QCoreApplication::translate("object_menu", "Are you sure you want to delete %1?")).arg("targets_display_string");
    // const QString text = QString(QCoreApplication::translate("object_menu", "Are you sure you want to delete %1?")).arg(targets_display_string(indexes));
    const bool confirmed = confirmation_dialog(text, this);

    if (confirmed) {
        AdInterface ad;
        if (ad_failed(ad)) {
            return;
        }

        for (const QModelIndex &index : indexes) {
            const QString dn = index.data(ObjectRole_DN).toString();
            const bool delete_success = ad.object_delete(dn);

            if (delete_success) {
                console_widget->delete_item(index);
            }
        }

        STATUS()->display_ad_messages(ad, this);
    }
}

void Console::on_properties_requested() {
    const QList<QModelIndex> indexes = console_widget->get_selected_items();
    if (indexes.size() != 1) {
        return;
    }
    const QModelIndex index = indexes[0];

    const QString dn = index.data(ObjectRole_DN).toString();

    PropertiesDialog *dialog = PropertiesDialog::open_for_target(dn);

    connect(
        dialog, &PropertiesDialog::applied,
        [=]() {
            AdInterface ad;
            if (ad_failed(ad)) {
                return;
            }

            const AdObject updated_object = ad.search_object(dn);

            auto update_console_item =
            [this, updated_object](const QModelIndex &i) {
                const bool is_scope = console_widget->is_scope_item(i);

                if (is_scope) {
                    QStandardItem *scope_item = console_widget->get_scope_item(i);
                    setup_scope_item(scope_item, updated_object);
                } else {
                    QList<QStandardItem *> results_row = console_widget->get_results_row(i);
                    load_object_row(results_row, updated_object);
                }
            };

            update_console_item(index);

            const QModelIndex buddy = console_widget->get_buddy(index);
            if (buddy.isValid()) {
                update_console_item(buddy);
            }

        });
}

void Console::rename() {
    const QList<QModelIndex> indexes = console_widget->get_selected_items();
    if (indexes.size() != 1) {
        return;
    }
    const QModelIndex index = indexes[0];

    const QString dn = index.data(ObjectRole_DN).toString();

    auto dialog = new RenameDialog(dn, this);

    connect(
        dialog, &RenameDialog::accepted,
        [this, dialog, index]() {
            AdInterface ad;
            if (ad_failed(ad)) {
                return;
            }

            const QString new_dn = dialog->get_new_dn();
            const AdObject updated_object = ad.search_object(new_dn);

            auto update_console_item =
            [this, updated_object](const QModelIndex &i) {
                const bool is_scope = console_widget->is_scope_item(i);

                if (is_scope) {
                    QStandardItem *scope_item = console_widget->get_scope_item(i);
                    setup_scope_item(scope_item, updated_object);
                } else {
                    QList<QStandardItem *> results_row = console_widget->get_results_row(i);
                    load_object_row(results_row, updated_object);
                }
            };

            update_console_item(index);

            const QModelIndex buddy = console_widget->get_buddy(index);
            if (buddy.isValid()) {
                update_console_item(buddy);
            }
        });

    dialog->open();
}

void Console::create(const QString &object_class) {
    const QList<QModelIndex> selected_indexes = console_widget->get_selected_items();
    if (selected_indexes.size() == 0) {
        return;
    }

    const QModelIndex selected_index = selected_indexes[0];
    const QString selected_dn = selected_index.data(ObjectRole_DN).toString();

    const auto dialog = new CreateDialog(selected_dn, object_class, this);

    connect(
        dialog, &CreateDialog::accepted,
        [=]() {
            AdInterface ad;
            if (ad_failed(ad)) {
                return;
            }

            const QString new_dn = dialog->get_created_dn();
            const AdObject new_object = ad.search_object(new_dn);

            add_object_to_console(new_object, selected_index);
        });

    dialog->open();
}

void Console::move() {
    const QList<QModelIndex> selected_indexes = console_widget->get_selected_items();
    if (selected_indexes.size() == 0) {
        return;
    }

    auto dialog = new SelectContainerDialog(this);

    // TODO:
    // const QString title = QString(tr("Move %1")).arg(targets_display_string(targets));
    // dialog->setWindowTitle(title);

    connect(
        dialog, &SelectContainerDialog::accepted,
        [=]() {
            const QString selected_dn = dialog->get_selected();

            const QModelIndex target =
            [=]() {
                const QList<QModelIndex> selected_in_scope = console_widget->search_scope_by_role(ObjectRole_DN, selected_dn);

                // NOTE: there's a crazy bug possible here
                // where a query item has some role with
                // same index as ObjectRole_DN and it's
                // value is also set to the same dn as target.
                if (selected_in_scope.size() == 1) {
                    return selected_in_scope[0];
                } else {
                    return QModelIndex();
                }
            }();

            AdInterface ad;
            if (ad_connected(ad)) {
                for (const QModelIndex &index : selected_indexes) {
                    move_object_in_console(ad, index, selected_dn, target);
                }

                STATUS()->display_ad_messages(ad, this);
            }

            console_widget->sort_scope();
        });

    dialog->open();
}

void Console::on_items_can_drop(const QList<QModelIndex> &dropped, const QModelIndex &target, bool *ok) {
    // NOTE: only check if object can be dropped if dropping a single object, because when dropping multiple objects it is ok for some objects to successfully drop and some to fail. For example, if you drop users together with OU's onto a group, users will be added to that group while OU will fail to drop.
    if (dropped.size() == 1) {
        const QModelIndex dropped_index = dropped[0];
        const AdObject dropped_object = dropped_index.data(ObjectRole_AdObject).value<AdObject>();
        const AdObject target_object = target.data(ObjectRole_AdObject).value<AdObject>();

        *ok = object_can_drop(dropped_object, target_object);
    } else {
        *ok = true;
    }
}

void Console::on_items_dropped(const QList<QModelIndex> &dropped, const QModelIndex &target) {
    // NOTE: using sibling at 0th column because we store
    // item roles in the first item (first column)
    const AdObject target_object = target.data(ObjectRole_AdObject).value<AdObject>();

    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    for (const QModelIndex &index : dropped) {
        const AdObject dropped_object = index.data(ObjectRole_AdObject).value<AdObject>();

        const DropType drop_type = get_drop_type(dropped_object, target_object);

        switch (drop_type) {
            case DropType_Move: {
                move_object_in_console(ad, index, target_object.get_dn(), target);

                break;
            }
            case DropType_AddToGroup: {
                ad.group_add_member(target_object.get_dn(), dropped_object.get_dn());

                break;
            }
            case DropType_None: {
                break;
            }
        }
    }

    STATUS()->display_ad_messages(ad, nullptr);

    console_widget->sort_scope();
}

void Console::refresh_head() {
    console_widget->refresh_scope(scope_head_index);
}

// TODO: currently calling this when current scope changes,
// but also need to call this when items are added/deleted
void Console::update_description_bar() {
    const QString text =
    [this]() {
        const QModelIndex current_scope = console_widget->get_current_scope_item();

        const QString scope_dn = current_scope.data(ObjectRole_DN).toString();
        const QString scope_name = dn_get_name(scope_dn);

        const int results_count = console_widget->get_current_results_count();
        const QString results_count_string = tr("%n object(s)", "", results_count);

        return QString("%1: %2").arg(scope_name, results_count_string);
    }();

    console_widget->set_description_bar_text(text);
}

void Console::on_action_menu_about_to_open(QMenu *menu, QAbstractItemView *view) {
    menu->addAction(rename_action);
    menu->addAction(move_action);

    QMenu *submenu_new = menu->addMenu(tr("New"));
    static const QList<QString> create_classes = {
        CLASS_USER,
        CLASS_COMPUTER,
        CLASS_OU,
        CLASS_GROUP,
    };
    for (const auto &object_class : create_classes) {
        const QString action_text = ADCONFIG()->get_class_display_name(object_class);

        submenu_new->addAction(action_text,
            [=]() {
                create(object_class);
            });
    }

    add_object_actions_to_menu(menu, view, this, true, true);
}

void Console::on_view_menu_about_to_open(QMenu *menu) {
    menu->addAction(open_filter_action);

    // NOTE: insert separator between non-checkbox actions
    // and checkbox actions
    menu->addSeparator();

    menu->addAction(advanced_view_action);
    menu->addAction(show_noncontainers_action);

    #ifdef QT_DEBUG
    menu->addAction(dev_mode_action);
    #endif
}

// Load children of this item in scope tree
// and load results linked to this scope item
void Console::fetch_scope_node(const QModelIndex &index) {
    show_busy_indicator();

    const bool dev_mode = SETTINGS()->get_bool(BoolSetting_DevMode);

    //
    // Search object's children
    //
    const QString filter =
    [=]() {
        const QString user_filter = filter_dialog->filter_widget->get_filter();

        const QString is_container = is_container_filter();

        // NOTE: OR user filter with containers filter so that container objects are always shown, even if they are filtered out by user filter
        QString out = filter_OR({user_filter, is_container});

        out = add_advanced_view_filter(out);

        // OR filter with some dev mode object classes, so that they show up no matter what when dev mode is on
        if (dev_mode) {
            const QList<QString> schema_classes = {
                "classSchema",
                "attributeSchema",
                "displaySpecifier",
            };

            QList<QString> class_filters;
            for (const QString &object_class : schema_classes) {
                const QString class_filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, object_class);
                class_filters.append(class_filter);
            }

            out = filter_OR({out, filter_OR(class_filters)});
        }

        return out;
    }();

    const QList<QString> search_attributes =
    []() {
        QList<QString> out;

        out += ADCONFIG()->get_columns();

        // NOTE: load_object_row() needs this for loading group type/scope
        out += ATTRIBUTE_GROUP_TYPE;

        // NOTE: system flags are needed for drag and drop logic
        out += ATTRIBUTE_SYSTEM_FLAGS;

        return out;
    }();

    const QString dn = index.data(ObjectRole_DN).toString();

    // TODO: handle connect/search failure
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    QHash<QString, AdObject> search_results = ad.search(filter, search_attributes, SearchScope_Children, dn);

    // Dev mode
    // NOTE: configuration and schema objects are hidden so that they don't show up in regular searches. Have to use search_object() and manually add them to search results.
    if (dev_mode) {
        const QString search_base = ADCONFIG()->domain_head();
        const QString configuration_dn = ADCONFIG()->configuration_dn();
        const QString schema_dn = ADCONFIG()->schema_dn();

        if (dn == search_base) {
            search_results[configuration_dn] = ad.search_object(configuration_dn);
        } else if (dn == configuration_dn) {
            search_results[schema_dn] = ad.search_object(schema_dn);
        }
    }

    //
    // Add items to scope and results
    //

    QList<QStandardItem *> rows;
    for (const AdObject &object : search_results.values()) {
        add_object_to_console(object, index);
    }

    // TODO: make sure to sort scope everywhere it should be
    // sorted
    console_widget->sort_scope();

    hide_busy_indicator();
}

void Console::setup_scope_item(QStandardItem *item, const AdObject &object) {
    const QString name =
    [&]() {
        const QString dn = object.get_dn();
        return dn_get_name(dn);
    }();

    item->setText(name);

    load_object_item_data(item, object);
}

// NOTE: "containers" referenced here don't mean objects
// with "container" object class. Instead it means all the
// objects that can have children(some of which are not
// "container" class).
bool object_should_be_in_scope(const AdObject &object) {
    const bool is_container =
    [=]() {
        const QList<QString> filter_containers = ADCONFIG()->get_filter_containers();
        const QString object_class = object.get_string(ATTRIBUTE_OBJECT_CLASS);

        return filter_containers.contains(object_class);
    }();

    const bool show_non_containers_ON = SETTINGS()->get_bool(BoolSetting_ShowNonContainersInConsoleTree);

    return (is_container || show_non_containers_ON);
}

void Console::add_object_to_console(const AdObject &object, const QModelIndex &parent) {
    const bool should_be_in_scope = object_should_be_in_scope(object);

    if (should_be_in_scope) {
        QStandardItem *scope_item = console_widget->add_scope_item(object_results_id, true, parent);
        setup_scope_item(scope_item, object);

        const QModelIndex scope_index = scope_item->index();

        const QList<QStandardItem *> results_row = console_widget->add_results_row(scope_index, parent);
        load_object_row(results_row, object);
    } else {
        const QList<QStandardItem *> results_row = console_widget->add_results_row(QModelIndex(), parent);
        load_object_row(results_row, object);
    }
}

// Moves object on AD server and if that succeeds, also
// moves it in console
void Console::move_object_in_console(AdInterface &ad, const QModelIndex &old_index, const QString &new_parent_dn, const QModelIndex &new_parent_index) {
    const QString old_dn = old_index.data(ObjectRole_DN).toString();
    const QString new_dn = dn_move(old_dn, new_parent_dn);

    const bool move_success = ad.object_move(old_dn, new_parent_dn);

    if (move_success) {
        console_widget->delete_item(old_index);

        // NOTE: new_parent_index may be invalid if it's not
        // loaded into console but was selected through
        // SelectContainDialog. In that case we only delete
        // object from it's old location and do nothing for
        // new location. The object will be loaded at new
        // location when it's parent is loaded.
        if (new_parent_index.isValid()) {
            const AdObject updated_object = ad.search_object(new_dn);
            add_object_to_console(updated_object, new_parent_index);
        }
    }
}
