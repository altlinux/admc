#include "subnets_list_edit.h"

#include <QListWidget>
#include "core/globals.h"
#include "core/managers/icon_manager.h"
#include "ad_filter.h"
#include "ad_object.h"
#include "ad_interface.h"
#include "ad_config.h"

SubnetsListEdit::SubnetsListEdit(QListWidget *subnets_list, QObject *parent) :
AttributeEdit(parent), subnets_list_wget(subnets_list) {

}

void SubnetsListEdit::load(AdInterface &ad, const AdObject &object) {
    if (!subnets_list_wget) {
        return;
    }

    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_SUBNET);
    auto subnet_obj_map = ad.search(g_adconfig->sites_container_dn(), SearchScope_All, filter,
                                    {ATTRIBUTE_NAME, ATTRIBUTE_SITE_OBJECT});

    for (const QString &subnet_dn : subnet_obj_map.keys()) {
        if (object.get_dn() != subnet_obj_map[subnet_dn].get_string(ATTRIBUTE_SITE_OBJECT)) {
            continue;
        }

        const QString name = subnet_obj_map[subnet_dn].get_string(ATTRIBUTE_NAME);
        QListWidgetItem *item = new QListWidgetItem(g_icon_manager->item_icon(ItemIcon_Subnet),
                                                    name);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setData(Qt::UserRole, subnet_dn);
        subnets_list_wget->addItem(item);
    }
}
