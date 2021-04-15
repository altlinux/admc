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

#include "console_types/object.h"

#include "adldap.h"
#include "globals.h"
#include "settings.h"
#include "utils.h"
#include "status.h"
#include "central_widget.h"
#include "filter_dialog.h"
#include "filter_widget/filter_widget.h"

#include <QStandardItemModel>

int object_results_id;

void load_object_row(const QList<QStandardItem *> row, const AdObject &object) {
    // Load attribute columns
    for (int i = 0; i < g_adconfig->get_columns().count(); i++) {
        const QString attribute = g_adconfig->get_columns()[i];

        if (!object.contains(attribute)) {
            continue;
        }

        const QString display_value =
        [attribute, object]() {
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

    load_object_item_data(row[0], object);
}

void load_object_item_data(QStandardItem *item, const AdObject &object) {
    item->setData(true, ConsoleRole_HasProperties);

    item->setData(ItemType_DomainObject, ConsoleRole_Type);

    const QIcon icon = get_object_icon(object);
    item->setIcon(icon);
    
    item->setData(object.get_dn(), ObjectRole_DN);

    const QList<QString> object_classes = object.get_strings(ATTRIBUTE_OBJECT_CLASS);
    item->setData(QVariant(object_classes), ObjectRole_ObjectClasses);

    const bool cannot_move = object.get_system_flag(SystemFlagsBit_CannotMove);
    item->setData(cannot_move, ObjectRole_CannotMove);

    const bool cannot_rename = object.get_system_flag(SystemFlagsBit_CannotRename);
    item->setData(cannot_rename, ObjectRole_CannotRename);

    const bool cannot_delete = object.get_system_flag(SystemFlagsBit_CannotDelete);
    item->setData(cannot_delete, ObjectRole_CannotDelete);

    const bool account_disabled = object.get_account_option(AccountOption_Disabled);
    item->setData(account_disabled, ObjectRole_AccountDisabled);
}

QList<QString> object_model_header_labels() {
    QList<QString> out;

    for (const QString &attribute : g_adconfig->get_columns()) {
        const QString attribute_display_name = g_adconfig->get_column_display_name(attribute);

        out.append(attribute_display_name);
    }
    
    return out;
}

QList<int> object_model_default_columns() {
    // By default show first 3 columns: name, class and
    // description
    return {0, 1, 2};
}

QList<QString> object_model_search_attributes() {
    QList<QString> attributes;

    attributes += g_adconfig->get_columns();

    // NOTE: needed for loading group type/scope into "type"
    // column
    attributes += ATTRIBUTE_GROUP_TYPE;

    // NOTE: system flags are needed to disable
    // delete/move/rename for objects that can't do those
    // actions
    attributes += ATTRIBUTE_SYSTEM_FLAGS;

    return attributes;
}

void setup_object_scope_item(QStandardItem *item, const AdObject &object) {
    const QString name =
    [&]() {
        const QString dn = object.get_dn();
        return dn_get_name(dn);
    }();

    item->setText(name);

    load_object_item_data(item, object);
    disable_drag_if_object_cant_be_moved({item}, object);
}

void setup_object_results_row(const QList<QStandardItem *> row, const AdObject &object) {
    load_object_row(row, object);
    disable_drag_if_object_cant_be_moved(row, object);
}

void disable_drag_if_object_cant_be_moved(const QList<QStandardItem *> &items, const AdObject &object) {
    const bool cannot_move = object.get_system_flag(SystemFlagsBit_CannotMove);

    for (auto item : items) {
        item->setDragEnabled(!cannot_move);
    }
}

// NOTE: have to search instead of just using deleted
// index because can delete objects from query tree
void console_delete_objects(ConsoleWidget *console, const QList<QString> &dn_list, const bool ignore_query_tree) {
    for (const QString &dn : dn_list) {
        // Delete in scope
        const QList<QPersistentModelIndex> scope_indexes = get_persistent_indexes(console->search_scope_by_role(ObjectRole_DN, dn, ItemType_DomainObject));
        for (const QPersistentModelIndex &index : scope_indexes) {
            console->delete_item(index);
        }

        // Delete in results
        const QList<QPersistentModelIndex> results_indexes = get_persistent_indexes(console->search_results_by_role(ObjectRole_DN, dn, ItemType_DomainObject));
        for (const QPersistentModelIndex &index : results_indexes) {
            // NOTE: don't touch query tree indexes, they
            // stay around and just go out of date
            const bool index_is_in_query_tree =
            [=]() {
                const QModelIndex scope_parent = console->get_scope_parent(index);
                const ItemType scope_parent_type = (ItemType) scope_parent.data(ConsoleRole_Type).toInt();

                return (scope_parent_type == ItemType_QueryItem);
            }();

            if (index_is_in_query_tree && ignore_query_tree) {
                continue;
            }

            console->delete_item(index);
        }
    }
}

void console_update_object(ConsoleWidget *console, const AdObject &object) {
    auto update_helper =
    [&](const QModelIndex &the_index) {
        const bool is_scope = console->is_scope_item(the_index);

        if (is_scope) {
            QStandardItem *scope_item = console->get_scope_item(the_index);
            setup_object_scope_item(scope_item, object);
        } else {
            QList<QStandardItem *> results_row = console->get_results_row(the_index);
            load_object_row(results_row, object);
        }
    };

    // Update in scope
    const QList<QPersistentModelIndex> scope_indexes = get_persistent_indexes(console->search_scope_by_role(ObjectRole_DN, object.get_dn(), ItemType_DomainObject));
    for (const QPersistentModelIndex &index : scope_indexes) {
        update_helper(index);
    }

    // Update in results
    const QList<QPersistentModelIndex> results_indexes = get_persistent_indexes(console->search_results_by_role(ObjectRole_DN, object.get_dn(), ItemType_DomainObject));
    for (const QPersistentModelIndex &index : results_indexes) {
        update_helper(index);
    }
}

void console_add_objects(ConsoleWidget *console, AdInterface &ad, const QList<QString> &dn_list, const QModelIndex &parent) {
    if (!console_add_objects_check(console, parent)) {
        return;
    }

    const QList<AdObject> object_list =
    [&]() {
        QList<AdObject> out;

        for (const QString &dn : dn_list) {
            const AdObject object = ad.search_object(dn);
            out.append(object);
        }

        return out;
    }();

    console_add_objects(console, object_list, parent);
}

void console_move_objects(ConsoleWidget *console, AdInterface &ad, const QList<QString> &old_dn_list, const QList<QString> &new_dn_list, const QString &new_parent_dn) {
    // NOTE: delete old item AFTER adding new item because:
    // If old item is deleted first, then it's possible for
    // new parent to get selected (if they are next to each
    // other in scope tree). Then what happens is that due
    // to new parent being selected, it gets fetched and
    // loads new object. End result is that new object is
    // duplicated.
    const QModelIndex new_parent_index =
    [=]() {
        const QList<QModelIndex> search_results = console->search_scope_by_role(ObjectRole_DN, new_parent_dn, ItemType_DomainObject);

        if (search_results.size() == 1) {
            return search_results[0];
        } else {
            return QModelIndex();
        }
    }();

    console_add_objects(console, ad, new_dn_list, new_parent_index);
    console_delete_objects(console, old_dn_list, true);

    console->sort_scope();
}

void console_move_objects(ConsoleWidget *console, AdInterface &ad, const QList<QString> &old_dn_list, const QString &new_parent_dn) {
    const QList<QString> new_dn_list =
    [&]() {
        QList<QString> out;

        for (const QString &old_dn : old_dn_list) {
            const QString new_dn = dn_move(old_dn, new_parent_dn);
            out.append(new_dn);
        }

        return out;
    }();

    console_move_objects(console, ad, old_dn_list, new_dn_list, new_parent_dn);
}

// Check parent index before adding objects to console
bool console_add_objects_check(ConsoleWidget *console, const QModelIndex &parent) {
    if (!parent.isValid()) {
        return false;
    }

    // NOTE: don't add if parent wasn't fetched yet. If that
    // is the case then the object will be added naturally
    // when parent is fetched.
    const bool parent_was_fetched = console->item_was_fetched(parent);
    if (!parent_was_fetched) {
        return false;
    }

    return true;
}

void console_add_objects(ConsoleWidget *console, const QList<AdObject> &object_list, const QModelIndex &parent) {
    if (!console_add_objects_check(console, parent)) {
        return;
    }

    for (const AdObject &object : object_list) {
        const bool should_be_in_scope =
        [&]() {
            // NOTE: "containers" referenced here don't mean
            // objects with "container" object class.
            // Instead it means all the objects that can
            // have children(some of which are not
            // "container" class).
            const bool is_container =
            [=]() {
                const QList<QString> filter_containers = g_adconfig->get_filter_containers();
                const QString object_class = object.get_string(ATTRIBUTE_OBJECT_CLASS);

                return filter_containers.contains(object_class);
            }();

            const bool show_non_containers_ON = g_settings->get_bool(BoolSetting_ShowNonContainersInConsoleTree);

            return (is_container || show_non_containers_ON);
        }();

        if (should_be_in_scope) {
            QStandardItem *scope_item;
            QList<QStandardItem *> results_row;
            console->add_buddy_scope_and_results(object_results_id, ScopeNodeType_Dynamic, parent, &scope_item, &results_row);

            setup_object_scope_item(scope_item, object);
            setup_object_results_row(results_row, object);
        } else {
            const QList<QStandardItem *> results_row = console->add_results_row(parent);
            setup_object_results_row(results_row, object);
        }
    }
}

// Load children of this item in scope tree
// and load results linked to this scope item
void fetch_object(ConsoleWidget *console, FilterDialog *filter_dialog, const QModelIndex &index) {
    // TODO: handle connect/search failure
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    show_busy_indicator();

    const bool dev_mode = g_settings->get_bool(BoolSetting_DevMode);

    //
    // Search object's children
    //
    const QString filter =
    [=]() {
        const QString user_filter = filter_dialog->filter_widget->get_filter();

        const QString is_container = is_container_filter();

        // NOTE: OR user filter with containers filter so that container objects are always shown, even if they are filtered out by user filter
        QString out = filter_OR({user_filter, is_container});

        // Hide advanced view only" objects if advanced view
        // setting is off
        const bool advanced_features_OFF = !g_settings->get_bool(BoolSetting_AdvancedFeatures);
        if (advanced_features_OFF) {
            const QString advanced_features = filter_CONDITION(Condition_NotEquals, ATTRIBUTE_SHOW_IN_ADVANCED_VIEW_ONLY, "true");
            out = filter_OR({out, advanced_features});
        }

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

    const QList<QString> search_attributes = object_model_search_attributes();

    const QString dn = index.data(ObjectRole_DN).toString();

    QHash<QString, AdObject> search_results = ad.search(filter, search_attributes, SearchScope_Children, dn);

    // Dev mode
    // NOTE: configuration and schema objects are hidden so that they don't show up in regular searches. Have to use search_object() and manually add them to search results.
    if (dev_mode) {
        const QString search_base = g_adconfig->domain_head();
        const QString configuration_dn = g_adconfig->configuration_dn();
        const QString schema_dn = g_adconfig->schema_dn();

        if (dn == search_base) {
            search_results[configuration_dn] = ad.search_object(configuration_dn);
        } else if (dn == configuration_dn) {
            search_results[schema_dn] = ad.search_object(schema_dn);
        }
    }

    console_add_objects(console, search_results.values(), index);
    console->sort_scope();

    hide_busy_indicator();
}
