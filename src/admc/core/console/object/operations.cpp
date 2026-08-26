#include "ad_config.h"
#include "ad_display.h"
#include "ad_utils.h"
#include "core/ad.h"
#include "core/console/object/server_dn_attrs_updater.h"
#include "core/console/object/site_dn_attrs_updater.h"
#include "core/globals.h"
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
