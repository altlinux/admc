/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2026 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
 * Copyright (C) 2023 Ivan A. Melnikov
 * Copyright (C) 2023-2026 Semyon Knyazev
 * Copyright (C) 2026 Artyom V. Poptsov
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

#include <QList>
#include <QStandardItem>

#include "ad_config.h"
#include "ad_display.h"
#include "ad_utils.h"
#include "core/ad.h"
#include "core/console.h"
#include "core/console/object/server_dn_attrs_updater.h"
#include "core/console/object/site_dn_attrs_updater.h"
#include "core/console_item_type.h"
#include "core/globals.h"
#include "core/managers/icon_manager.h"
#include "core/settings.h"
#include "operations.h"

/**
 * Fix DN attributes which contain site/server that are not actual DNs after
 * move/rename.  In the vain of Unix tradition we consider "rename" operation as
 * a special case of "move" operation.
 *
 * @param object_map Mapping of objects.
 * @param old_to_new_dn_map Mapping of old DNs to new DNs.
 */
void object_update_move(AdInterface &ad,
                        const QHash<QString, AdObject> &object_map,
                        const QHash<QString, QString> &old_to_new_dn_map) {
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

void object_update_delete(AdInterface &ad,
                          const QString &object_class,
                          const QString &target_dn) {
    if (object_class == CLASS_SITE) {
        SiteDnAttrsUpdater(target_dn).update_for_delete(ad);
    } else if (object_class == CLASS_SERVER) {
        ServerDnAttrsUpdater(target_dn).update_for_delete(ad);
    }
}

/**
 * Search all objects from the provided DN list.
 *
 * @param ad An AD instance.
 * @param dn_list A list of object DNs to search.
 * @return A list of found AdObject instances.
 */
QList<AdObject> object_search_all(AdInterface &ad,
                                  const QList<QString> &dn_list) {
    QList<AdObject> object_list;
    for (const QString &dn : dn_list) {
        const AdObject object = ad.search_object(dn);
        object_list.append(object);
    }
    return object_list;
}

/**
 * Make a display value for an AD object.
 */
QString object_make_display_value(const AdObject &object,
                                  const QString &attribute) {
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
}

QList<QString> object_column_labels() {
    QList<QString> out;
    for (const QString &attribute : g_adconfig->get_columns()) {
        const QString name = g_adconfig->get_column_display_name(attribute);
        out.append(name);
    }

    return out;
}

/**
 * NOTE: "containers" referenced here don't mean objects with "container" object
 * class. Instead it means all the objects that can have children(some of which
 * are not "container" class).
 */
bool object_should_be_in_scope(const AdObject &object) {
    const QString object_class = object.get_string(ATTRIBUTE_OBJECT_CLASS);
    const QList<QString> filter_containers =
        g_adconfig->get_filter_containers();
    const bool is_container = filter_containers.contains(object_class);
    const bool show_non_containers_on =
        settings_get_bool(SETTING_show_non_containers_in_console_tree);
    const bool is_site_related =
        g_adconfig->get_site_related_classes().contains(object_class);

    return (is_container ||
            show_non_containers_on ||
            is_site_related ||
            (object_class == CLASS_PSO));
}

/**
 * Check if an object has "SITE" class.
 *
 * @param object An AD object to check.
 * @return true if the object has "SITE" class, false otherwise.
 */
bool object_is_site(const AdObject &object) {
    return object.get_string(ATTRIBUTE_OBJECT_CLASS) == CLASS_SITE;
}

void object_load_attribute_columns(const AdObject &object,
                                   const QList<QStandardItem *> row) {
    int count = g_adconfig->get_columns().count();
    for (int i = 0; i < count; i++) {
        if (g_adconfig->get_columns().count() > row.size()) {
            break;
        }

        const QString attribute = g_adconfig->get_columns()[i];

        if (! object.contains(attribute)) {
            continue;
        }

        QString display_value = object_make_display_value(object, attribute);
        row[i]->setText(display_value);
    }
}

void object_item_load_icon(QStandardItem *item, bool disabled) {
    auto set_item_icon = [item, disabled](const ItemIcon &disabled_icon,
                                          const ItemIcon &enabled_icon) {
        ItemIcon item_icon = disabled ? disabled_icon : enabled_icon;
        item->setIcon(g_icon_manager->item_icon(item_icon));
    };
    auto set_category_icon = [item](auto &icon) {
        item->setIcon(g_icon_manager->category_icon(icon));
    };
    const QString category =
        dn_get_name(item->data(ObjectRole_ObjectCategory).toString());

    if (item->data(ConsoleRole_Type).toInt() == ItemType_QueryItem) {
        set_category_icon(ADMC_CATEGORY_QUERY_ITEM);
    }
    else if (category == OBJECT_CATEGORY_PERSON) {
        set_item_icon(ItemIcon_Person_Blocked, ItemIcon_Person);
    }
    else if (category == OBJECT_CATEGORY_COMPUTER) {
        set_item_icon(ItemIcon_Computer_Blocked, ItemIcon_Computer);
    }
    else if (category == OBJECT_CATEGORY_GROUP) {
        item->setIcon(g_icon_manager->item_icon(ItemIcon_Group));
    }
    else {
        set_category_icon(category);
    }
}

void object_item_data_load(const AdObject &object, QStandardItem *item) {
    item->setData(object.get_dn(), ObjectRole_DN);

    const QList<QString> object_classes =
        object.get_strings(ATTRIBUTE_OBJECT_CLASS);
    item->setData(QVariant(object_classes), ObjectRole_ObjectClasses);

    const QString object_category =
        object.get_string(ATTRIBUTE_OBJECT_CATEGORY);
    item->setData(object_category, ObjectRole_ObjectCategory);

    const bool cannot_move =
        object.get_system_flag(SystemFlagsBit_DomainCannotMove);
    item->setData(cannot_move, ObjectRole_CannotMove);

    const bool cannot_rename =
        object.get_system_flag(SystemFlagsBit_DomainCannotRename);
    item->setData(cannot_rename, ObjectRole_CannotRename);

    const bool cannot_delete =
        object.get_system_flag(SystemFlagsBit_CannotDelete);
    item->setData(cannot_delete, ObjectRole_CannotDelete);

    const bool account_disabled =
        object.get_account_option(AccountOption_Disabled, g_adconfig);
    item->setData(account_disabled, ObjectRole_AccountDisabled);

    object_item_load_icon(item, account_disabled);
}

void object_load(const AdObject &object, const QList<QStandardItem *> row) {
    object_load_attribute_columns(object, row);
    object_item_data_load(object, row[0]);

    const bool cannot_move =
        object.get_system_flag(SystemFlagsBit_DomainCannotMove);

    for (auto item : row) {
        item->setDragEnabled(!cannot_move);
    }
}

QList<QString> object_search_attributes() {
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

QList<int> object_default_columns() {
    // By default show first 3 columns: name, class and description
    return {0, 1, 2};
}

QString object_delete_confirmation_message(
    const QList<QModelIndex> &index_deleted_list)
{
    if (index_deleted_list.size() == 1) {
        return QCoreApplication::translate(
            "ObjectImpl",
            "Are you sure you want to delete this object?");
    }
    else {
        return QCoreApplication::translate(
            "ObjectImpl",
            "Are you sure you want to delete these objects?");
    }
}

QString object_delete_confirmation_submessage(
    const QList<QModelIndex> &index_deleted_list,
    int not_empty_containers_count)
{
    if ((not_empty_containers_count == 1) && (index_deleted_list.size() == 1)) {
        return QCoreApplication::translate(
            "ObjectImpl",
            " It contains other objects.");
    }
    else if (not_empty_containers_count >= 1) {
        return QCoreApplication::translate(
            "ObjectImpl",
            " Containers to be deleted contain other objects.");
    } else {
        return QString();
    }
}
