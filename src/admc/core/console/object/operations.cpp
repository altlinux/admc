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

#include "ad_config.h"
#include "ad_display.h"
#include "ad_utils.h"
#include "core/ad.h"
#include "core/console/object/server_dn_attrs_updater.h"
#include "core/console/object/site_dn_attrs_updater.h"
#include "core/globals.h"
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
