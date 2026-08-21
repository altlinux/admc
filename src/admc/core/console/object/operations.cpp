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
